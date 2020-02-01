#include "utils.h"
#include "bitlogic.h"
#include "macroutils.h"
#include "magicbitboard.h"
#include <algorithm>
#include "evaluatorInit.h"

INT8U MaxDepth = 8;

BitBoard Board;

std::string wlog;

Move* MoveList;
INT8U MoveCount;

Move BestMove;

bestmove::Type BestMoveType = bestmove::Normal;

INT64U PositionHistory[1024];
INT16U HistoryCount;
color::Type MySide;
INT64U InitialBoardHash;

int Nodes = 0;
int LogNodes = 0;
int QuiescenceNodes = 0;
int EvaluatedNodes = 0;
int EvaluatedQuiescenceNodes = 0;

EvaluationDataType EvaluationData = { { 0, 100, 100, 325, 325, 335, 335, 500, 500, 975, 975, SORT_KING_CAPTURE, SORT_KING_CAPTURE },
                                      { 0, 100, 100, 325, 325, 335, 335, 500, 500, 975, 975, 0, 0 }, 
                                      { 0, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1, 0, 0 }
                                      }; // { SortValue, PieceValue, CaptureValue, ... }

INT64U FirstRankAttacks64x8[512];
INT64U FileAttacksDown[64];
INT64U FileAttacksUp[64];
INT64U DiagonalAttacksDown[64];
INT64U DiagonalAttacksUp[64];
INT64U AntiDiagonalAttacksDown[64];
INT64U AntiDiagonalAttacksUp[64];
INT64U PawnAttacksWhite[64];
INT64U PawnAttacksBlack[64];
INT64U EnPassantPawnAttacksWhite[64];
INT64U EnPassantPawnAttacksBlack[64];
INT64U KnightAttacks[64];
INT64U KingAttacks[64];

INT64U RankMask[8];
INT64U FileMask[8];

#ifdef ZOBRIST
#ifdef ZOBRIST_CPW
int ZobristMask = 0;
TranspositionEntry* TranspositionTable;
#else
int ZobristMask = ZobristSize - 1; // must be (ZobristSize / entry_size)- 1 
TranspositionEntry TranspositionTable[ZobristSize];
//int ZobristMask = 0;
//TranspositionEntry* TranspositionTable;
#endif
szobrist Zobrist;
INT8U ZobristTo = 0;
INT8U ZobristFrom = 0;
#endif

namespace {

    INT64U RookTable[0x19000];  // To store rook attacks
    INT64U BishopTable[0x1480]; // To store bishop attacks

    void precalculateFirstRankAttacks()
    {
        for (INT8U attackerIndex = 0; attackerIndex < 8; ++attackerIndex)
        {
            INT8U normalizedAttackerIndex = 8 - attackerIndex; // since 0000001 is "h" file not "a"
            INT64U attackerFileRight = 0b11111111 >> normalizedAttackerIndex;
            INT64U attackerFileLeft = ~attackerFileRight;
            INT64U attackerFile = attackerFileLeft & ~(attackerFileLeft - 1); // least significant bit
            attackerFileLeft = attackerFileLeft & ~attackerFile & 0b11111111;

            for (INT8U occupiedIndex = 0; occupiedIndex <= 63; ++occupiedIndex)
            {
                INT64U maskLeft = occupiedIndex << 1 & attackerFileLeft;
                maskLeft = LSB(maskLeft);
                if (maskLeft == 0)
                {
                    --maskLeft; // all of bits "1"
                }
                else
                {
                    maskLeft = maskLeft | (maskLeft - 1);
                }

                INT64U maskRight = occupiedIndex & attackerFileRight;
                INT8U msbPosition = 0;
                if (maskRight != 0)
                {
                    BSR(msbPosition, maskRight);
                }

                if (maskRight != 0)
                {
                    ++msbPosition;
                }

                maskRight = 0b11111111 << msbPosition & attackerFileRight;

                FirstRankAttacks64x8[occupiedIndex * 8 + attackerIndex] = (maskLeft & attackerFileLeft) | maskRight;
            }
        }
    }

    void precalculateFileAttacks()
    {
        for (INT8U rank = 0; rank < 8; ++rank)
        {
            for (INT8U file = 0; file < 8; ++file)
            {
                INT8U squreIndex = INDEX(rank, file);
                INT64U mask = POS(squreIndex);

                INT64U maskUp = 0;
                for (INT8U upRank = 1; upRank + rank < 8; ++upRank)
                {
                    maskUp |= mask << 8 * upRank;
                }

                INT64U maskDown = 0;
                for (INT8U downRank = 1; rank - downRank >= 0; ++downRank)
                {
                    maskDown |= mask >> 8 * downRank;
                }

                FileAttacksDown[squreIndex] = maskDown;
                FileAttacksUp[squreIndex] = maskUp;
            }
        }
    }

    void precalculateDiagonalAttacks()
    {
        for (INT8U rank = 0; rank < 8; ++rank)
        {
            for (INT8U file = 0; file < 8; ++file)
            {
                INT8U squreIndex = INDEX(rank, file);
                INT64U mask = POS(squreIndex);

                INT64U maskUp = 0;
                INT8U extraShift = 1;
                for (INT8U upRank = 1; upRank + rank < 8 && file + extraShift < 8; ++upRank)
                {
                    maskUp |= mask << (8 * upRank + extraShift);
                    ++extraShift;
                }

                INT64U maskDown = 0;
                extraShift = 1;
                for (INT8U downRank = 1; rank - downRank >= 0 && file - extraShift >= 0; ++downRank)
                {
                    maskDown |= mask >> (8 * downRank + extraShift);
                    ++extraShift;
                }

                DiagonalAttacksDown[squreIndex] = maskDown;
                DiagonalAttacksUp[squreIndex] = maskUp;
            }
        }
    }

    void precalculateAntiDiagonalAttacks()
    {
        for (INT8U rank = 0; rank < 8; ++rank)
        {
            for (INT8U file = 0; file < 8; ++file)
            {
                INT8U squreIndex = INDEX(rank, file);
                INT64U mask = POS(squreIndex);

                INT64U maskUp = 0;
                INT8U extraShift = 1;
                for (INT8U upRank = 1; upRank + rank < 8 && file - extraShift >= 0; ++upRank)
                {
                    maskUp |= mask << (8 * upRank - extraShift);
                    ++extraShift;
                }

                INT64U maskDown = 0;
                extraShift = 1;
                for (INT8U downRank = 1; rank - downRank >= 0 && file + extraShift < 8; ++downRank)
                {
                    maskDown |= mask >> (8 * downRank - extraShift);
                    ++extraShift;
                }

                AntiDiagonalAttacksDown[squreIndex] = maskDown;
                AntiDiagonalAttacksUp[squreIndex] = maskUp;
            }
        }
    }

    void precalculateNormalPawnAttacks()
    {
        for (int i = 56; i < 64; ++i)
        {
            //PawnAttacksWhite[i] = 0;
            PawnAttacksBlack[i] = 0;
        }

        for (int i = 0; i < 8; ++i)
        {
            PawnAttacksWhite[i] = 0;
            //PawnAttacksBlack[i] = 0;
        }

        for (INT8U rank = 0; rank < 8; ++rank)
        {
            for (INT8U file = 0; file < 8; ++file)
            {
                INT8U squreIndex = INDEX(rank, file);
                INT64U mask = POS(squreIndex);
               
                INT64U maskWhite = 0;
                INT64U maskBlack = 0;

                if (file < 7)
                {
                    maskWhite |= mask << 9;
                    maskBlack |= mask >> 7;
                }

                if (file > 0)
                {
                    maskWhite |= mask << 7;
                    maskBlack |= mask >> 9;
                }

                if (rank != 8)
                {
                    PawnAttacksWhite[squreIndex] = maskWhite;
                }

                if (rank != 0)
                {
                    PawnAttacksBlack[squreIndex] = maskBlack;
                }
            }
        }
    }

    void precalculateEnPassantPawnAttacks()
    {
        for (int i = 0; i < 8; ++i)
        {
            EnPassantPawnAttacksWhite[i] = 0;
            EnPassantPawnAttacksBlack[i] = 0;
        }

        for (INT8U rank = 1; rank < 8; ++rank)
        {
            for (INT8U file = 0; file < 8; ++file)
            {
                INT8U squreIndex = INDEX(rank, file);

                EnPassantPawnAttacksWhite[squreIndex] = 0;
                EnPassantPawnAttacksBlack[squreIndex] = 0;

                if (rank == 4)
                {
                    INT64U mask = POS(squreIndex);

                    if (file > 0)
                    {
                        EnPassantPawnAttacksWhite[squreIndex] |= mask >> 1;
                    }

                    if (file < 7)
                    {
                        EnPassantPawnAttacksWhite[squreIndex] |= mask << 1;
                    }
                }
                else if (rank == 3)
                {
                    INT64U mask = POS(squreIndex);

                    if (file > 0)
                    {
                        EnPassantPawnAttacksBlack[squreIndex] |= mask >> 1;
                    }

                    if (file < 7)
                    {
                        EnPassantPawnAttacksBlack[squreIndex] |= mask << 1;
                    }
                }
            }
        }
    }

    void precalculateKnightAttacks()
    {
        for (INT8U rank = 0; rank < 8; ++rank)
        {
            for (INT8U file = 0; file < 8; ++file)
            {
                INT8U squreIndex = INDEX(rank, file);
                INT64U mask = POS(squreIndex);
                
                INT64U knightMask = 0;

                if (file > 0 && rank > 1)
                {
                    knightMask |= mask >> (2 * 8 + 1);
                }

                if (file > 1 && rank > 0)
                {
                    knightMask |= mask >> (1 * 8 + 2);
                }

                if (file < 7 && rank > 1)
                {
                    knightMask |= mask >> (2 * 8 - 1);
                }

                if (file < 6 && rank > 0)
                {
                    knightMask |= mask >> (1 * 8 - 2);
                }

                if (file > 0 && rank < 6)
                {
                    knightMask |= mask << (2 * 8 - 1);
                }

                if (file > 1 && rank < 7)
                {
                    knightMask |= mask << (1 * 8 - 2);
                }

                if (file < 7 && rank < 6)
                {
                    knightMask |= mask << (2 * 8 + 1);
                }

                if (file < 6 && rank < 7)
                {
                    knightMask |= mask << (1 * 8 + 2);
                }

                KnightAttacks[squreIndex] = knightMask;
            }
        }
    }

    void precalculateKingAttacks()
    {
        for (INT8U rank = 0; rank < 8; ++rank)
        {
            for (INT8U file = 0; file < 8; ++file)
            {
                INT8U squreIndex = INDEX(rank, file);
                INT64U mask = POS(squreIndex);
               
                INT64U kingMask = 0;

                if (file > 0)
                {
                    kingMask |= mask >> 1;

                    if (rank > 0)
                    {
                        kingMask |= mask >> (8 + 1);
                    }

                    if (rank < 7)
                    {
                        kingMask |= mask << (8 - 1);
                    }
                }

                if (file < 7)
                {
                    kingMask |= mask << 1;

                    if (rank > 0)
                    {
                        kingMask |= mask >> (8 - 1);
                    }

                    if (rank < 7)
                    {
                        kingMask |= mask << (8 + 1);
                    }
                }

                if (rank > 0)
                {
                    kingMask |= mask >> 8;
                }

                if (rank < 7)
                {
                    kingMask |= mask << 8;
                }

                KingAttacks[squreIndex] = kingMask;
            }
        }
    }

    void preCalculatePieceScoreTable()
    {
        for (int i = 0; i < 64; ++i) 
        {
            EvaluationData.MiddleGamePieceScoreTable[piece::BlackPawn][i] = pawn_pcsq_mg[i];
            EvaluationData.MiddleGamePieceScoreTable[piece::WhitePawn][indexByColor[color::White][i]] = pawn_pcsq_mg[i];
            EvaluationData.MiddleGamePieceScoreTable[piece::BlackKnight][i] = knight_pcsq_mg[i];
            EvaluationData.MiddleGamePieceScoreTable[piece::WhiteKnight][indexByColor[color::White][i]] = knight_pcsq_mg[i];
            EvaluationData.MiddleGamePieceScoreTable[piece::BlackBishop][i] = bishop_pcsq_mg[i];
            EvaluationData.MiddleGamePieceScoreTable[piece::WhiteBishop][indexByColor[color::White][i]] = bishop_pcsq_mg[i];
            EvaluationData.MiddleGamePieceScoreTable[piece::BlackRook][i] = rook_pcsq_mg[i];
            EvaluationData.MiddleGamePieceScoreTable[piece::WhiteRook][indexByColor[color::White][i]] = rook_pcsq_mg[i];
            EvaluationData.MiddleGamePieceScoreTable[piece::BlackQueen][i] = queen_pcsq_mg[i];
            EvaluationData.MiddleGamePieceScoreTable[piece::WhiteQueen][indexByColor[color::White][i]] = queen_pcsq_mg[i];
            EvaluationData.MiddleGamePieceScoreTable[piece::BlackKing][i] = king_pcsq_mg[i];
            EvaluationData.MiddleGamePieceScoreTable[piece::WhiteKing][indexByColor[color::White][i]] = king_pcsq_mg[i];

            EvaluationData.EndGamePieceScoreTable[piece::BlackPawn][i] = pawn_pcsq_eg[i];
            EvaluationData.EndGamePieceScoreTable[piece::WhitePawn][indexByColor[color::White][i]] = pawn_pcsq_eg[i];
            EvaluationData.EndGamePieceScoreTable[piece::BlackKnight][i] = knight_pcsq_eg[i];
            EvaluationData.EndGamePieceScoreTable[piece::WhiteKnight][indexByColor[color::White][i]] = knight_pcsq_eg[i];
            EvaluationData.EndGamePieceScoreTable[piece::BlackBishop][i] = bishop_pcsq_eg[i];
            EvaluationData.EndGamePieceScoreTable[piece::WhiteBishop][indexByColor[color::White][i]] = bishop_pcsq_eg[i];
            EvaluationData.EndGamePieceScoreTable[piece::BlackRook][i] = rook_pcsq_eg[i];
            EvaluationData.EndGamePieceScoreTable[piece::WhiteRook][indexByColor[color::White][i]] = rook_pcsq_eg[i];
            EvaluationData.EndGamePieceScoreTable[piece::BlackQueen][i] = queen_pcsq_eg[i];
            EvaluationData.EndGamePieceScoreTable[piece::WhiteQueen][indexByColor[color::White][i]] = queen_pcsq_eg[i];
            EvaluationData.EndGamePieceScoreTable[piece::BlackKing][i] = king_pcsq_eg[i];
            EvaluationData.EndGamePieceScoreTable[piece::WhiteKing][indexByColor[color::White][i]] = king_pcsq_eg[i];
        }
    }
}

int SquareDistance[64][64];

void clearSearch()
{
#ifdef HISTORY
    clearHistoryTable();
#endif

    BestMoveType = bestmove::Normal;

    BestMove.From = 0;
    BestMove.To = 0;
    BestMove.Score = -INF;

    Nodes = 0;
    EvaluatedNodes = 0;
    QuiescenceNodes = 0;
}

void initializeHelpers()
{
    precalculateFirstRankAttacks();
    precalculateFileAttacks();
    precalculateDiagonalAttacks();
    precalculateAntiDiagonalAttacks();
    precalculateNormalPawnAttacks();
    precalculateEnPassantPawnAttacks();
    precalculateKnightAttacks();
    precalculateKingAttacks();

    preCalculatePieceScoreTable();

    for (INT8U file = 0; file < 8; ++file)
    {
        FileMask[file] = file > 0 ? FileMask[file - 1] << 1 : FileA;
    }

    for (INT8U rank = 0; rank < 8; ++rank)
    {
        RankMask[rank] = rank > 0 ? RankMask[rank - 1] << 8 : Rank1;
    }

    for (INT8U square1 = 0; square1 < 64; ++square1)
    {
        for (INT8U square2 = 0; square2 < 64; ++square2)
        {
            if (square1 != square2)
            {
                SquareDistance[square1][square2] = std::max(distance_file(square1, square2), distance_rank(square1, square2));
            }
        }
    }

    Direction RookDirections[] = { NORTH_MAGIC, EAST_MAGIC, SOUTH_MAGIC, WEST_MAGIC };
    Direction BishopDirections[] = { NORTH_EAST_MAGIC, SOUTH_EAST_MAGIC, SOUTH_WEST_MAGIC, NORTH_WEST_MAGIC };

    init_magics(RookTable, RookMagics, RookDirections);
    init_magics(BishopTable, BishopMagics, BishopDirections);
}

#ifdef ZOBRIST
INT8U getCpwPieceType(piece::Type pieceType)
{
    switch (pieceType)
    {
    case piece::Empty: 
        return 6;
    case piece::BlackPawn:
    case piece::WhitePawn:
        return 5;
    case piece::BlackKnight:
    case piece::WhiteKnight:
        return 4;
    case piece::BlackBishop:
    case piece::WhiteBishop:
        return 3;
    case piece::BlackRook:
    case piece::WhiteRook:
        return 2;
    case piece::BlackQueen:
    case piece::WhiteQueen:
        return 1;
    case piece::BlackKing:
    case piece::WhiteKing:
        return 0;
    default: 
        return 9;
    }
}
#endif

#ifdef ZOBRIST
void tteval_save(INT8U depth,
                 int score,
                 int flags,
                 INT8U from,
                 INT8U to)
{
    TranspositionEntry* entry = &TranspositionTable[Board.ZobristHash & ZobristMask];

    ttsave++;

    if (entry != nullptr && entry->Flags != 255)
    {
        ttconflict++;
    }

    if (entry->Hash == Board.ZobristHash && entry->Depth > depth) return;

    //if (entry->Hash == Board.ZobristHash &&
    //    (entry->Flags != TT_EXACT && flags != TT_EXACT && entry->Depth >= (MaxDepth - depth)) ||
    //    (entry->Flags == TT_EXACT && flags != TT_EXACT) ||
    //    (entry->Flags == TT_EXACT && entry->Depth >= (MaxDepth - depth)))
    //{
    //    return;
    //}

    entry->Hash = Board.ZobristHash;
    entry->Score = score;
    entry->Depth = depth;
    entry->Flags = flags;
    entry->BestFrom = from;
    entry->BestTo = to;
}

short int tteval_probe(INT8U depth, short int alpha, short int beta)
{
    TranspositionEntry* entry = &TranspositionTable[Board.ZobristHash & ZobristMask];
 
    if (entry->Hash == Board.ZobristHash)
    {
        tthit++;
        if (entry->Depth >= depth) {

            if (entry->Flags == TT_EXACT)
                return entry->Score;

            if ((entry->Flags == TT_ALPHA) && (entry->Score <= alpha))
                return alpha;

            if ((entry->Flags == TT_BETA) && (entry->Score >= beta))
                return beta;
        }
    }

    return InvalidScore;
}

INT64U calculateZobristHash()
{
#ifdef ZOBRIST_CPW
    Board.ZobristHash = 0;

    for (int square = 0; square < 64; ++square)
    {
        piece::Type pieceType = Board.PieceBySquare[square];
        INT8U color = pieceType & 1 ? 0 : 1;
        if (pieceType != piece::Type::Empty)
        {
            Board.ZobristHash ^= Zobrist.PieceSquare[getCpwPieceType(pieceType)][color][square + RANK(square) * 8];
        }
    }

    if (Board.Enpassant != -1)
    {
        Board.ZobristHash ^= Zobrist.EnPassant[Board.Enpassant + RANK(Board.Enpassant) * 8];
    }

    if (Board.SideToMove == 1)
    {
        Board.ZobristHash ^= Zobrist.IsBlackColor;
    }

    Board.ZobristHash ^= Zobrist.Castling[Board.Castle];
    
#else
    Board.ZobristHash = 0;

    for (int square = 0; square < 64; ++square)
    {
        piece::Type pieceType = Board.PieceBySquare[square];
        if (pieceType != piece::Type::Empty)
        {
            Board.ZobristHash ^= Zobrist.PieceSquare[pieceType][square];
        }
    }

    if (Board.Enpassant != -1)
    {
        Board.ZobristHash ^= Zobrist.EnPassant[Board.Enpassant];
    }

    if (Board.SideToMove == 0)
    {
        Board.ZobristHash ^= Zobrist.IsBlackColor;
    }

    Board.ZobristHash ^= Zobrist.Castling[Board.Castle];
#endif

    return Board.ZobristHash;
}

/* function taken from Sungorus chess engine */
INT64U rand64() {
    static INT64U next = 1;

    next = next * 1103515245 + 12345;
    return next;
}

void initializeTranspositionTable()
{
    // when again calling search, maybe wasm memory is reordered -> that's why free is called at the end of searching...
    //ZobristMask = ZobristSize - 1;
    //TranspositionTable = new TranspositionEntry[ZobristSize];
}

int tt_init()
{
#ifdef ZOBRIST_CPW
    free(TranspositionTable);
    ZobristMask = ZobristSize - 1;
    //ZobristMask = (ZobristSize / sizeof(TranspositionEntry)) - 1;
    TranspositionTable = (TranspositionEntry *)malloc(ZobristSize*sizeof(TranspositionEntry));
    //TranspositionTable = (TranspositionEntry *)malloc(ZobristSize);

    /* fill the zobrist struct with random numbers */

    for (int pnr = 0; pnr <= 5; pnr++) {
        for (int cnr = 0; cnr <= 1; cnr++) {
            for (int snr = 0; snr <= 127; snr++) {
                Zobrist.PieceSquare[pnr][cnr][snr] = rand64();
            }
        }
    }

    Zobrist.IsBlackColor = rand64();

    for (int castling = 0; castling <= 15; castling++) {
        Zobrist.Castling[castling] = rand64();
    }

    for (int ep = 0; ep <= 127; ep++) {
        Zobrist.EnPassant[ep] = rand64();
    }

    return 0;
#else
    //delete[] TranspositionBuffer;
    //free(TranspositionTable);
    //ZobristMask = ZobristSize - 1;
    //ZobristMask = (ZobristSize / sizeof(TranspositionEntry)) - 1;

    //TranspositionBuffer = new unsigned char[ZobristSize * sizeof(TranspositionEntry)];

    // placement new in buf 
    //TranspositionTable = new (TranspositionBuffer) TranspositionEntry[ZobristSize];


    //TranspositionTable = (TranspositionEntry *)malloc(ZobristSize * sizeof(TranspositionEntry));

    initializeTranspositionTable();

    /* fill the zobrist struct with random numbers */

    for (int pnr = 0; pnr < piece::Count; ++pnr)
    {
        for (int snr = 0; snr < 64; snr++)
        {
            Zobrist.PieceSquare[pnr][snr] = rand64();
        }
    }

    Zobrist.IsBlackColor = rand64();

    for (int castling = 0; castling <= 15; ++castling) {
        Zobrist.Castling[castling] = rand64();
    }

    for (int ep = 0; ep <= 63; ++ep) {
        Zobrist.EnPassant[ep] = rand64();
    }

    return 0;
#endif
}

int ttsave = 0;
int ttconflict = 0;
int tthit = 0;
int tthit2 = 0;
#endif

std::string getPathAsString(INT8U PathDepth, const LogNodeArray& path)
{
    std::string result;
    for (int logIndex = 0; logIndex < PathDepth; ++logIndex)
    {
        char moveStr[5];
        convertFromSquare(path[logIndex].From, path[logIndex].To, moveStr);
        result += moveStr;
        result += " ";
    }

    return result;
}

bool slide[10] = { 0, 0, 1, 1, 1, 1, 1, 1, 0, 0 };
char vectors[10] = { 8, 8, 4, 4, 4, 4, 8, 8, 8, 8 };
char vector[10][8] = { { -17, -15, -10, -6, 6, 10, 15, 17 },
                       { -17, -15, -10, -6, 6, 10, 15, 17 },
                       { SW, SE, NW, NE },
                       { SW, SE, NW, NE },
                       { SOUTH, WEST, EAST, NORTH },
                       { SOUTH, WEST, EAST, NORTH },
                       { SW, SOUTH, SE, WEST, EAST, NW, NORTH, NE },
                       { SW, SOUTH, SE, WEST, EAST, NW, NORTH, NE },
                       { SW, SOUTH, SE, WEST, EAST, NW, NORTH, NE },
                       { SW, SOUTH, SE, WEST, EAST, NW, NORTH, NE } };

std::chrono::system_clock::time_point StartingTime;
