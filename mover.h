#ifndef MOVER_H
#define MOVER_H

#include "utils.h"
#include <cstdlib> // abs miatt, board-hoz 
#include "bitlogic.h"
#include "magicbitboard.h"
#include "macroutils.h"

#ifndef EM_BUILD
#include <cassert>
#endif

namespace mover {

#ifdef SWITCH_TO_NORMAL_MOVE_GENERATION
    inline bool isPiece(piece::Type piece, INT8S pieceSquare)
    {
        return Board.PieceBySquare[static_cast<INT8U>(pieceSquare)] == piece;
    }

    inline bool hasLeaperAttack(INT8S attackedSquare, piece::Type attackerPiece)
    {
        INT8S nextSquare;

        for (INT8U dir = 0; dir < 8; ++dir)
        {
            nextSquare = attackedSquare + vector[attackerPiece - 3][dir];

            bool isSquare = false;
            INT8S previousSquare = nextSquare - vector[attackerPiece - 3][dir];
            IS_SQ_AFTER_LEAP_ONE(isSquare, previousSquare, nextSquare)

            if (isSquare && isPiece(attackerPiece, nextSquare))
            {
                return true;
            }
        }

        return false;
    }

    inline bool hasStraightAttack(INT8U attackerColor, INT8S attackedSquare, int vector)
    {
        INT8S nextSquare = attackedSquare + static_cast<INT8S>(vector);

        bool isSquare = false;
        INT8S previousSquare = nextSquare - static_cast<INT8S>(vector);
        IS_SQ_AFTER_LEAP_ONE(isSquare, previousSquare, nextSquare)

        while (isSquare)
        {
            if (Board.PieceBySquare[static_cast<INT8U>(nextSquare)] != piece::Empty)
            {
                if ((attackerColor == 0 && (Board.PieceBySquare[static_cast<INT8U>(nextSquare)] == piece::WhiteRook || Board.PieceBySquare[static_cast<INT8U>(nextSquare)] == piece::WhiteQueen)) ||
                    (attackerColor == 1 && (Board.PieceBySquare[static_cast<INT8U>(nextSquare)] == piece::BlackRook || Board.PieceBySquare[static_cast<INT8U>(nextSquare)] == piece::BlackQueen)))
                {
                    return true;
                }

                return false;
            }

            previousSquare = nextSquare;
            nextSquare += vector;

            IS_SQ_AFTER_LEAP_ONE(isSquare, previousSquare, nextSquare)
        }

        return false;
    }

    inline bool hasDiagonalAttack(INT8U attackerColor, INT8S attackedSquare, int vector)
    {
        INT8S nextSquare = attackedSquare + static_cast<INT8S>(vector);

        bool isSquare = false;
        INT8S previousSquare = nextSquare - static_cast<INT8S>(vector);
        IS_SQ_AFTER_LEAP_ONE(isSquare, previousSquare, nextSquare)

        while (isSquare)
        {
            if (Board.PieceBySquare[static_cast<INT8U>(nextSquare)] != piece::Empty)
            {
                if ((attackerColor == 0 && (Board.PieceBySquare[static_cast<INT8U>(nextSquare)] == piece::WhiteBishop || Board.PieceBySquare[static_cast<INT8U>(nextSquare)] == piece::WhiteQueen)) ||
                    (attackerColor == 1 && (Board.PieceBySquare[static_cast<INT8U>(nextSquare)] == piece::BlackBishop || Board.PieceBySquare[static_cast<INT8U>(nextSquare)] == piece::BlackQueen)))
                {
                    return true;
                }

                return false;
            }

            previousSquare = nextSquare;
            nextSquare += vector;

            IS_SQ_AFTER_LEAP_ONE(isSquare, previousSquare, nextSquare)
        }

        return false;
    }

    inline bool hasBishopAttack(INT8U attackerColor, INT8S attackedSquare, int vector)
    {
        INT8S nextSquare = attackedSquare + static_cast<INT8S>(vector);

        bool isSquare = false;
        INT8S previousSquare = nextSquare - static_cast<INT8S>(vector);
        IS_SQ_AFTER_LEAP_ONE(isSquare, previousSquare, nextSquare)

            while (isSquare)
            {
                if (Board.PieceBySquare[static_cast<INT8U>(nextSquare)] != piece::Empty)
                {
                    if ((attackerColor == 0 && Board.PieceBySquare[static_cast<INT8U>(nextSquare)] == piece::WhiteBishop) ||
                        (attackerColor == 1 && Board.PieceBySquare[static_cast<INT8U>(nextSquare)] == piece::BlackBishop))
                    {
                        return true;
                    }

                    return false;
                }

                previousSquare = nextSquare;
                nextSquare += vector;

                IS_SQ_AFTER_LEAP_ONE(isSquare, previousSquare, nextSquare)
            }

        return false;
    }

    inline bool isAttackedByPawn(char attackerColor, INT8S attackedSquare)
    {
        // pawns
        if (attackerColor == color::White)
        {
            bool isSquare = false;
            IS_SQ_AFTER_LEAP_ONE(isSquare, attackedSquare, attackedSquare + SE)

            if (isSquare && isPiece(piece::WhitePawn, attackedSquare + SE))
            {
                return true;
            }

            IS_SQ_AFTER_LEAP_ONE(isSquare, attackedSquare, attackedSquare + SW)

            if (isSquare && isPiece(piece::WhitePawn, attackedSquare + SW))
            {
                return true;
            }
        }
        else
        {
            bool isSquare = false;
            IS_SQ_AFTER_LEAP_ONE(isSquare, attackedSquare, attackedSquare + NE)

            if (isSquare && isPiece(piece::BlackPawn, attackedSquare + NE))
            {
                return true;
            }

            IS_SQ_AFTER_LEAP_ONE(isSquare, attackedSquare, attackedSquare + NW)

            if (isSquare && isPiece(piece::BlackPawn, attackedSquare + NW))
            {
                return true;
            }
        }

        return false;
    }

    inline bool isAttacked(char attackerColor, INT8S attackedSquare)
    {
        //pawn
        if (isAttackedByPawn(attackerColor, attackedSquare))
        {
            return true;
        }

        //knight
        if (hasLeaperAttack(attackedSquare, attackerColor ? piece::BlackKnight : piece::WhiteKnight))
        {
            return true;
        }

        //king
        if (hasLeaperAttack(attackedSquare, attackerColor ? piece::BlackKing : piece::WhiteKing))
        {
            return true;
        }

        // rook and queen
        if (hasStraightAttack(static_cast<INT8U>(attackerColor), attackedSquare, NORTH) ||
            hasStraightAttack(static_cast<INT8U>(attackerColor), attackedSquare, SOUTH) ||
            hasStraightAttack(static_cast<INT8U>(attackerColor), attackedSquare, EAST) ||
            hasStraightAttack(static_cast<INT8U>(attackerColor), attackedSquare, WEST))
        {
            return true;
        }

        // bishop and queen
        if (hasDiagonalAttack(static_cast<INT8U>(attackerColor), attackedSquare, NE) ||
            hasDiagonalAttack(static_cast<INT8U>(attackerColor), attackedSquare, SE) ||
            hasDiagonalAttack(static_cast<INT8U>(attackerColor), attackedSquare, NW) ||
            hasDiagonalAttack(static_cast<INT8U>(attackerColor), attackedSquare, SW))
        {
            return true;
        }

        return false;
    }

#else

    inline bool isAttacked(INT8U attackerColor, INT8U attackedSquare)
    {
        return attacksFrom(attackerColor, attackedSquare, Board.AllPieces[color::Black] | Board.AllPieces[color::White]) > 0;
    }

    inline bool isAttackedByPawn(char attackerColor, INT8S attackedSquare)
    {
        if (attackedSquare == color::White)
        {
            if (attacks_bb(piece::BlackPawn, attackedSquare) & Board.SquareByPiece[piece::WhitePawn]) 
            {
                return true;
            }
        }
        else
        {
            if (attacks_bb(piece::WhitePawn, attackedSquare) & Board.SquareByPiece[piece::BlackPawn])
            {
                return true;
            }
        }

        return false;
    }

#endif

    inline void fillSquare(color::Type color, piece::Type piece, INT8U square)
    {
        Board.PieceBySquare[square] = piece;
        Board.SquareByPiece[piece] |= POS(square);

        Board.AllPieces[color] |= POS(square);

        Board.MiddleGamePositionScore[color] += EvaluationData.MiddleGamePieceScoreTable[piece][square];
        Board.EndGamePositionScore[color] += EvaluationData.EndGamePieceScoreTable[piece][square];

        if (piece == piece::BlackPawn || piece == piece::WhitePawn)
        {
            Board.PawnMaterial[color] += EvaluationData.PieceValue[piece];

            if (color == color::White)
            {
                if (FILE(square) > 0)
                {
                    Board.PawnControl[color::White] |= 1ULL << (square + NW);
                }

                if (FILE(square) < 7)
                {
                    Board.PawnControl[color::White] |= 1ULL << (square + NE);
                }
            }
            else
            {
                if (FILE(square) > 0)
                {
                    Board.PawnControl[color::Black] |= 1ULL << (square + SW);
                }

                if (FILE(square) < 7)
                {
                    Board.PawnControl[color::Black] |= 1ULL << (square + SE);
                }
            }
        }
        else
        {
            Board.PieceMaterial[color] += EvaluationData.PieceValue[piece];
        }

#ifdef ZOBRIST
#ifdef ZOBRIST_CPW
        INT8U zcolor = piece & 1 ? 0 : 1;
        Board.ZobristHash ^= Zobrist.PieceSquare[getCpwPieceType(static_cast<piece::Type>(piece))][zcolor][square + RANK(square) * 8];
#else
        Board.ZobristHash ^= Zobrist.PieceSquare[piece][square];
#endif
#endif
    }

    inline void clearSquare(INT8U square)
    {
        INT8U piece = Board.PieceBySquare[square];
        color::Type color = (piece & 1) ? color::White : color::Black;

        Board.SquareByPiece[piece] &= ~POS(square);
        Board.PieceBySquare[square] = piece::Empty;

        Board.AllPieces[color] &= ~POS(square);

        Board.MiddleGamePositionScore[color] -= EvaluationData.MiddleGamePieceScoreTable[piece][square];
        Board.EndGamePositionScore[color] -= EvaluationData.EndGamePieceScoreTable[piece][square];

        if (piece == piece::BlackPawn || piece == piece::WhitePawn)
        {
            Board.PawnMaterial[color] -= EvaluationData.PieceValue[piece];

            if (color == color::White)
            {
                if (FILE(square) > 0)
                {
                    if (FILE(square) < 2 || Board.PieceBySquare[square - 2] != piece::WhitePawn)
                    {
                        Board.PawnControl[color::White] &= ~(1ULL << (square + NW));
                    }
                }

                if (FILE(square) < 7)
                {
                    if (FILE(square) > 5 || Board.PieceBySquare[square + 2] != piece::WhitePawn)
                    {
                        Board.PawnControl[color::White] &= ~(1ULL << (square + NE));
                    }
                }
            }
            else
            {
                if (FILE(square) > 0)
                {
                    if (FILE(square) < 2 || Board.PieceBySquare[square - 2] != piece::BlackPawn)
                    {
                        Board.PawnControl[color::Black] &= ~(1ULL << (square + SW));
                    }
                }

                if (FILE(square) < 7)
                {
                    if (FILE(square) > 5 || Board.PieceBySquare[square + 2] != piece::BlackPawn)
                    {
                        Board.PawnControl[color::Black] &= ~(1ULL << (square + SE));
                    }
                }
            }
        }
        else
        {
            Board.PieceMaterial[color] -= EvaluationData.PieceValue[piece];
        }

#ifdef ZOBRIST
#ifdef ZOBRIST_CPW
        INT8U zcolor = piece & 1 ? 0 : 1;
        Board.ZobristHash ^= Zobrist.PieceSquare[getCpwPieceType(static_cast<piece::Type>(piece))][zcolor][square + RANK(square) * 8];
#else
        Board.ZobristHash ^= Zobrist.PieceSquare[piece][square];
#endif
#endif
    }

#ifdef SEE_BLIND
    /*
    * This is not yet proper static exchange evaluation, but an approximation
    * proposed by Harm Geert Mueller under acronym BLIND (better or lower
    * if not defended). As the name indicates, it detects only obviously good
    * captures, but it seems enogh to improve move ordering
    */
    inline bool see_Blind(const Move& move)
    {
        // captures by pawn do not loose matterial
        if (move.FromPiece == piece::BlackPawn || move.FromPiece == piece::WhitePawn)
        {
            return true;
        }

        // captures "lower take higher" (as well as Bishop takes Knight) are good by definition}
        if (EvaluationData.SortValue[move.CapturedPiece] >= EvaluationData.SortValue[move.FromPiece] - 50)
        {
            return true;
        }

        // make the first capture so the X-ray defender show up
        clearSquare(move.From);

        // capture the undefended pieces are good by definition
        if (!isAttacked(!Board.SideToMove, move.To))
        {
            fillSquare(static_cast<color::Type>(Board.SideToMove), static_cast<piece::Type>(move.FromPiece), move.From);
            return true;
        }

        fillSquare(static_cast<color::Type>(Board.SideToMove), static_cast<piece::Type>(move.FromPiece), move.From);

        return false; // about other captures we know nothing (John Snow) 
    }
#endif

    inline void saveMove(INT8U from, INT8U to, INT8U fromPiece, INT8U capturedPiece, INT8S flags)
    {
        MoveList[MoveCount].From = from;
        MoveList[MoveCount].To = to;
        MoveList[MoveCount].FromPiece = fromPiece;
        MoveList[MoveCount].ToPiece = fromPiece;
        MoveList[MoveCount].CapturedPiece = capturedPiece;
        MoveList[MoveCount].Castle = Board.Castle;
        MoveList[MoveCount].Enpassant = Board.Enpassant;
        MoveList[MoveCount].Ply = Board.Ply;
        MoveList[MoveCount].Flags = flags;

        MoveList[MoveCount].Score = 0;

#ifdef HISTORY
        MoveList[MoveCount].Score = History[Board.SideToMove][from][to];
#endif

        /*
         * Score for captures: add the value of the captured piece and the id of
         * the attacking piece. If two pieces attack the same target the one 
         * with higher id (CaptureValue, eg. Pawn = 5) will be investigated first. 
         * En passanrt gets the same score as pawn takes pawn. Good captures are 
         * put at the front of the list. Bad captures are put after the ordinary moves.
         */
        if (capturedPiece != piece::Empty)
        {
            SEE(MoveList[MoveCount])  
            //MoveList[MoveCount].Score = EvaluationData.SortValue[capturedPiece] + EvaluationData.CaptureValue[fromPiece];
        }

        if (fromPiece == piece::WhitePawn || fromPiece == piece::BlackPawn)
        {
            if (to == Board.Enpassant)
            {
                MoveList[MoveCount].Score = EvaluationData.SortValue[piece::WhitePawn] + EvaluationData.CaptureValue[piece::WhitePawn] + SORT_CAPTURE; // 5 -> point of pawn (enum place)
                MoveList[MoveCount].Flags = FLAG_ENPASSANT_CAPTURE;
            }

            if (RANK(to) == 0 || RANK(to) == 7)
            {
                MoveList[MoveCount].Flags |= FLAG_PROMOTION;
                MoveList[MoveCount].ToPiece = (fromPiece & 1) ? piece::WhiteQueen : piece::BlackQueen;
                MoveList[MoveCount].Score += EvaluationData.SortValue[piece::WhiteQueen] + SORT_PROMOTION;
            }
        }

        if (BestMove.From == from && BestMove.To == to)
        {
            MoveList[MoveCount].Score = SORT_HASH;
        }

//#ifdef ZOBRIST
//        // it should be last step to ovrwrite every score
//        if (to == ZobristTo && from == ZobristFrom)
//        {
//            MoveList[MoveCount].Score = SORT_ZOBRIST;
//        }
//#endif 

        ++MoveCount;
    }

#ifdef SWITCH_TO_NORMAL_MOVE_GENERATION

    inline void generatePawnMove88(INT8S square, bool isPromotionOnly)
    {
        if (Board.SideToMove == color::White)
        {
            if (isPromotionOnly && RANK(square) != 6)
            {
                return;
            }

            if (Board.PieceBySquare[square + NORTH] == piece::Empty)
            {
                saveMove(square, square + NORTH, piece::WhitePawn, piece::Empty, FLAG_NORMAL);

                if (RANK(square) == 1 && Board.PieceBySquare[square + NN] == piece::Empty)
                {
                    saveMove(square, square + NN, piece::WhitePawn, piece::Empty, FLAG_ENPASSANT);
                }
            }
        }
        else
        {
            if (isPromotionOnly && RANK(square) != 1)
            {
                return;
            }

            if (Board.PieceBySquare[square + SOUTH] == piece::Empty)
            {
                saveMove(square, square + SOUTH, piece::BlackPawn, piece::Empty, FLAG_NORMAL);

                if (RANK(square) == 6 && Board.PieceBySquare[square + SS] == piece::Empty)
                {
                    saveMove(square, square + SS, piece::BlackPawn, piece::Empty, FLAG_ENPASSANT);
                }
            }
        }
    }

    inline void generatePawnCapture88(INT8S square)
    {
        if (Board.SideToMove == color::White)
        {
            bool isSquare = false;
            IS_SQ_AFTER_LEAP_ONE(isSquare, square, square + NW)

            if (isSquare && (Board.Enpassant == square + NW || IS_BLACK(Board.PieceBySquare[square + NW])))
            {
                saveMove(square, square + NW, piece::WhitePawn, Board.PieceBySquare[square + NW], FLAG_CAPTURE);
            }

            IS_SQ_AFTER_LEAP_ONE(isSquare, square, square + NE)

            if (isSquare && (Board.Enpassant == square + NE || IS_BLACK(Board.PieceBySquare[square + NE])))
            {
                saveMove(square, square + NE, piece::WhitePawn, Board.PieceBySquare[square + NE], FLAG_CAPTURE);
            }
        }
        else
        {
            bool isSquare = false;
            IS_SQ_AFTER_LEAP_ONE(isSquare, square, square + SE)

            if (isSquare && (Board.Enpassant == square + SE || IS_WHITE(Board.PieceBySquare[square + SE])))
            {
                saveMove(square, square + SE, piece::BlackPawn, Board.PieceBySquare[square + SE], FLAG_CAPTURE);
            }

            IS_SQ_AFTER_LEAP_ONE(isSquare, square, square + SW)

            if (isSquare && (Board.Enpassant == square + SW || IS_WHITE(Board.PieceBySquare[square + SW])))
            {
                saveMove(square, square + SW, piece::BlackPawn, Board.PieceBySquare[square + SW], FLAG_CAPTURE);
            }
        }
    }

    inline INT8U generateMove(Move* moveList, bool isCaptureOnly)
    {
        MoveList = moveList; // ez azert jo, mert a felszabaditas automatikus es sporolos
        MoveCount = 0;

        if (!isCaptureOnly)
        {
            if (Board.SideToMove == color::White)
            {
                if (Board.Castle & castle::WhiteKingSide)
                {
                    if (Board.PieceBySquare[square::F1] == piece::Empty &&
                        Board.PieceBySquare[square::G1] == piece::Empty &&
                        !isAttacked(!Board.SideToMove, square::E1) &&
                        !isAttacked(!Board.SideToMove, square::F1) &&
                        !isAttacked(!Board.SideToMove, square::G1))
                    {
                        saveMove(square::E1, square::G1, piece::WhiteKing, piece::Empty, FLAG_CASTLE);
                    }
                }

                if (Board.Castle & castle::WhiteQueenSide)
                {
                    if (Board.PieceBySquare[square::B1] == piece::Empty &&
                        Board.PieceBySquare[square::C1] == piece::Empty &&
                        Board.PieceBySquare[square::D1] == piece::Empty &&
                        !isAttacked(!Board.SideToMove, square::E1) &&
                        !isAttacked(!Board.SideToMove, square::D1) &&
                        !isAttacked(!Board.SideToMove, square::C1))
                    {
                        saveMove(square::E1, square::C1, piece::WhiteKing, piece::Empty, FLAG_CASTLE);
                    }
                }
            }
            else
            {
                if (Board.Castle & castle::BlackKingSide)
                {
                    if (Board.PieceBySquare[square::F8] == piece::Empty &&
                        Board.PieceBySquare[square::G8] == piece::Empty &&
                        !isAttacked(!Board.SideToMove, square::E8) &&
                        !isAttacked(!Board.SideToMove, square::F8) &&
                        !isAttacked(!Board.SideToMove, square::G8))
                    {
                        saveMove(square::E8, square::G8, piece::BlackKing, piece::Empty, FLAG_CASTLE);
                    }
                }

                if (Board.Castle & castle::BlackQueenSide)
                {
                    if (Board.PieceBySquare[square::B8] == piece::Empty &&
                        Board.PieceBySquare[square::C8] == piece::Empty &&
                        Board.PieceBySquare[square::D8] == piece::Empty &&
                        !isAttacked(!Board.SideToMove, square::E8) &&
                        !isAttacked(!Board.SideToMove, square::D8) &&
                        !isAttacked(!Board.SideToMove, square::C8))
                    {
                        saveMove(square::E8, square::C8, piece::BlackKing, piece::Empty, FLAG_CASTLE);
                    }
                }
            }
        }

        for (INT8U square = 0; square < 64; ++square)
        {
            if (IS_SAME_COLORS(Board.PieceBySquare[square], Board.SideToMove))
            {
                // pawn
                if (Board.PieceBySquare[square] == piece::WhitePawn ||
                    Board.PieceBySquare[square] == piece::BlackPawn)
                {
                    generatePawnMove88(static_cast<INT8S>(square), false);
                    generatePawnCapture88(static_cast<INT8S>(square));
                }
                else if (Board.PieceBySquare[square] != piece::Empty)
                {
                    for (INT8S dir = 0; dir < vectors[Board.PieceBySquare[square] - 3]; ++dir)
                    {
                        for (INT8S toSquare = static_cast<INT8S>(square);;)
                        {
                            toSquare += vector[Board.PieceBySquare[square] - 3][static_cast<INT8U>(dir)];

                            bool isSquare = false;
                            INT8S previousSquare = toSquare - vector[Board.PieceBySquare[square] - 3][static_cast<INT8U>(dir)];
                            IS_SQ_AFTER_LEAP_ONE(isSquare, previousSquare, toSquare)

                            if (!isSquare)
                            {
                                break;
                            }

                            if (Board.PieceBySquare[static_cast<INT8U>(toSquare)] == piece::Empty)
                            {
                                if (!isCaptureOnly)
                                {
                                    saveMove(static_cast<INT8S>(square), toSquare, Board.PieceBySquare[static_cast<INT8U>(square)], piece::Empty, FLAG_NORMAL);
                                }
                            }
                            else if (IS_SAME_COLORS(Board.PieceBySquare[toSquare], Board.SideToMove))
                            {
                                break; // hitting own piece
                            }
                            else
                            {
                                saveMove(static_cast<INT8S>(square), toSquare, Board.PieceBySquare[static_cast<INT8U>(square)], Board.PieceBySquare[static_cast<INT8U>(toSquare)], FLAG_CAPTURE);
                                break;
                            }

                            if (!slide[Board.PieceBySquare[square] - 3])
                            {
                                break;
                            }
                        }
                    }
                }
            }

        }

        return MoveCount;
    }

    inline INT8U generateQuiscenceMove(Move* moveList)
    {
        MoveList = moveList;
        MoveCount = 0;

        for (INT8U square = 0; square < 64; ++square)
        {
            if (IS_SAME_COLORS(Board.PieceBySquare[square], Board.SideToMove))
            {
                // pawn
                if (Board.PieceBySquare[square] == piece::WhitePawn ||
                    Board.PieceBySquare[square] == piece::BlackPawn)
                {
                    generatePawnMove88(static_cast<INT8S>(square), true);
                    generatePawnCapture88(static_cast<INT8S>(square));
                }
                else if (Board.PieceBySquare[square] != piece::Empty)
                {
                    for (INT8S dir = 0; dir < vectors[Board.PieceBySquare[square] - 3]; ++dir)
                    {
                        for (INT8S toSquare = static_cast<INT8S>(square);;)
                        {
                            toSquare += vector[Board.PieceBySquare[square] - 3][static_cast<INT8U>(dir)];

                            bool isSquare = false;

                            INT8S previousSquare = toSquare - vector[Board.PieceBySquare[square] - 3][static_cast<INT8U>(dir)];
                            IS_SQ_AFTER_LEAP_ONE(isSquare, previousSquare, toSquare)

                            if (!isSquare)
                            {
                                break;
                            }

                            if (Board.PieceBySquare[static_cast<INT8U>(toSquare)] != piece::Empty)
                            {
                                if (IS_SAME_COLORS(Board.PieceBySquare[static_cast<INT8U>(toSquare)], Board.SideToMove))
                                {
                                    break; // hitting own piece
                                }
                                else
                                {
                                    saveMove(static_cast<INT8S>(square), toSquare, Board.PieceBySquare[square], Board.PieceBySquare[static_cast<INT8U>(toSquare)], FLAG_CAPTURE);
                                    break;
                                }
                            }

                            if (!slide[Board.PieceBySquare[square] - 3])
                            {
                                break;
                            }
                        }
                    }
                }
            }

        }

        return MoveCount;
    }
#else

    inline void generatePawnMove(INT8U square, bool isPromotionOnly)
    {
        if (Board.SideToMove == color::White)
        {
            if (isPromotionOnly && RANK(square) != 6) 
            {
                return;
            }

            if (Board.PieceBySquare[square + NORTH] == piece::Empty)
            {
                saveMove(square, square + NORTH, piece::WhitePawn, piece::Empty, FLAG_NORMAL);

                if ( RANK(square) == 1 && Board.PieceBySquare[square + NN] == piece::Empty )
                {
                    saveMove(square, square + NN, piece::WhitePawn, piece::Empty, FLAG_ENPASSANT);
                }
            }
        }
        else // black pawn
        {
            if (isPromotionOnly && RANK(square) != 1)
            {
                return;
            }

            if (Board.PieceBySquare[square + SOUTH] == piece::Empty)
            {
                saveMove(square, square + SOUTH, piece::BlackPawn, piece::Empty, FLAG_NORMAL);

                if (RANK(square) == 6 && Board.PieceBySquare[square + SS] == piece::Empty)
                {
                    saveMove(square, square + SS, piece::BlackPawn, piece::Empty, FLAG_ENPASSANT);
                }
            }
        }
    }

    inline void generatePawnCapture(INT8U square)
    {
        if (Board.SideToMove == color::White)
        {
            if ( IS_SQ_NW(square) && (Board.Enpassant == square + NW || IS_BLACK(Board.PieceBySquare[square + NW])))
            {
                saveMove(square, square + NW, piece::WhitePawn, Board.PieceBySquare[square + NW], FLAG_CAPTURE);
            }

            if (IS_SQ_NE(square) && (Board.Enpassant == square + NE || IS_BLACK(Board.PieceBySquare[square + NE])))
            {
                saveMove(square, square + NE, piece::WhitePawn, Board.PieceBySquare[square + NE], FLAG_CAPTURE);
            }
        }
        else
        {
            if (IS_SQ_SE(square) && (Board.Enpassant == square + SE || IS_WHITE(Board.PieceBySquare[square + SE])))
            {
                saveMove(square, square + SE, piece::BlackPawn, Board.PieceBySquare[square + SE], FLAG_CAPTURE);
            }

            if (IS_SQ_SW(square) && (Board.Enpassant == square + SW || IS_WHITE(Board.PieceBySquare[square + SW])))
            {
                saveMove(square, square + SW, piece::BlackPawn, Board.PieceBySquare[square + SW], FLAG_CAPTURE);
            }
        }
    }

    inline void checkAndSaveEnapassantCaptureMove(INT8U square)
    {
        if (Board.SideToMove == color::White)
        {
            if (IS_SQ_NW(square) && Board.Enpassant == square + NW)
            {
                saveMove(square, square + NW, piece::WhitePawn, Board.PieceBySquare[square + NW], FLAG_CAPTURE);
            }

            if (IS_SQ_NE(square) && Board.Enpassant == square + NE)
            {
                saveMove(square, square + NE, piece::WhitePawn, Board.PieceBySquare[square + NE], FLAG_CAPTURE);
            }
        }
        else
        {
            if (IS_SQ_SW(square) && Board.Enpassant == square + SW)
            {
                saveMove(square, square + SW, piece::BlackPawn, Board.PieceBySquare[square + SW], FLAG_CAPTURE);
            }

            if (IS_SQ_SE(square) && Board.Enpassant == square + SE)
            {
                saveMove(square, square + SE, piece::BlackPawn, Board.PieceBySquare[square + SE], FLAG_CAPTURE);
            }
        }
    }

    inline INT8U generateMove(Move* moveList, bool isCaptureOnly)
    {
        MoveList = moveList; // ez azert jo, mert a felszabaditas automatikus es sporolos
        MoveCount = 0;

        if (!isCaptureOnly)
        {
            if (Board.SideToMove == color::White)
            {
                if (Board.Castle & castle::WhiteKingSide)
                {
                    if (Board.PieceBySquare[square::F1] == piece::Empty &&
                        Board.PieceBySquare[square::G1] == piece::Empty &&
                        !isAttacked(!Board.SideToMove, square::E1) &&
                        !isAttacked(!Board.SideToMove, square::F1) &&
                        !isAttacked(!Board.SideToMove, square::G1))
                    {
                        saveMove(square::E1, square::G1, piece::WhiteKing, piece::Empty, FLAG_CASTLE);
#ifndef EM_BUILD
                        assert(Board.PieceBySquare[square::E1] == piece::WhiteKing);
                        assert(Board.PieceBySquare[square::H1] == piece::WhiteRook);
#endif
                    }
                }

                if (Board.Castle & castle::WhiteQueenSide)
                {
                    if (Board.PieceBySquare[square::B1] == piece::Empty &&
                        Board.PieceBySquare[square::C1] == piece::Empty &&
                        Board.PieceBySquare[square::D1] == piece::Empty &&
                        !isAttacked(!Board.SideToMove, square::E1) &&
                        !isAttacked(!Board.SideToMove, square::D1) &&
                        !isAttacked(!Board.SideToMove, square::C1))
                    {
#ifndef EM_BUILD
                        assert(Board.PieceBySquare[square::E1] == piece::WhiteKing);
                        assert(Board.PieceBySquare[square::A1] == piece::WhiteRook);
#endif
                        saveMove(square::E1, square::C1, piece::WhiteKing, piece::Empty, FLAG_CASTLE);
                    }
                }
            }
            else
            {
                if (Board.Castle & castle::BlackKingSide)
                {
                    if (Board.PieceBySquare[square::F8] == piece::Empty &&
                        Board.PieceBySquare[square::G8] == piece::Empty &&
                        !isAttacked(!Board.SideToMove, square::E8) &&
                        !isAttacked(!Board.SideToMove, square::F8) &&
                        !isAttacked(!Board.SideToMove, square::G8))
                    {
#ifndef EM_BUILD
                        assert(Board.PieceBySquare[square::E8] == piece::BlackKing);
                        assert(Board.PieceBySquare[square::H8] == piece::BlackRook);
#endif
                        saveMove(square::E8, square::G8, piece::BlackKing, piece::Empty, FLAG_CASTLE);
                    }
                }

                if (Board.Castle & castle::BlackQueenSide)
                {
                    if (Board.PieceBySquare[square::B8] == piece::Empty &&
                        Board.PieceBySquare[square::C8] == piece::Empty &&
                        Board.PieceBySquare[square::D8] == piece::Empty &&
                        !isAttacked(!Board.SideToMove, square::E8) &&
                        !isAttacked(!Board.SideToMove, square::D8) &&
                        !isAttacked(!Board.SideToMove, square::C8))
                    {
#ifndef EM_BUILD
                        assert(Board.PieceBySquare[square::E8] == piece::BlackKing);
                        assert(Board.PieceBySquare[square::A8] == piece::BlackRook);
#endif
                        saveMove(square::E8, square::C8, piece::BlackKing, piece::Empty, FLAG_CASTLE);
                    }
                }
            }
        }

        for (INT8U fromIndex = 0; fromIndex < 64; ++fromIndex)
        {
            if ((Board.PieceBySquare[fromIndex] & 1) != Board.SideToMove)
            {
                // pawns handled separatly 
                if (Board.PieceBySquare[fromIndex] == piece::WhitePawn || Board.PieceBySquare[fromIndex] == piece::BlackPawn)
                {
                    generatePawnMove(fromIndex, false);
                    generatePawnCapture(fromIndex);
                    continue;
                }

                INT64U toList = attacks_bb(Board.PieceBySquare[fromIndex], fromIndex, (Board.AllPieces[0] | Board.AllPieces[1]));

                INT8U  toIndex = 0;
                BSF(toIndex, toList);

                while (toIndex != InvalidIndex)
                {
                    INT8S flags = Board.PieceBySquare[toIndex] == piece::Empty ? FLAG_NORMAL : FLAG_CAPTURE;

                    if (Board.PieceBySquare[toIndex] != piece::Empty)
                    {
                        if (isCaptureOnly)
                        {
                            toList &= ~POS(toIndex);
                            BSF(toIndex, toList);
                            continue;
                        }

                        if (IS_SAME_COLORS(Board.PieceBySquare[toIndex], Board.SideToMove))
                        {
                            toList &= ~POS(toIndex);
                            BSF(toIndex, toList);
                            continue; // hitting own piece
                        }
                    }

                    saveMove(fromIndex, toIndex, Board.PieceBySquare[fromIndex], Board.PieceBySquare[toIndex], flags);

                    toList &= ~POS(toIndex);
                    BSF(toIndex, toList);
                }
            }
        }

        return MoveCount;
    }

    inline INT8U generateQuiscenceMove(Move* moveList)
    {
        MoveList = moveList;
        MoveCount = 0;

        for (INT8U fromIndex = 0; fromIndex < 64; ++fromIndex)
        {
            if ((Board.PieceBySquare[fromIndex] & 1) != Board.SideToMove)
            {
                // pawns handled separatly 
                if (Board.PieceBySquare[fromIndex] == piece::WhitePawn || Board.PieceBySquare[fromIndex] == piece::BlackPawn)
                {
                    generatePawnCapture(fromIndex);
                    continue;
                }

                INT64U toList = attacks_bb(Board.PieceBySquare[fromIndex], fromIndex, (Board.AllPieces[0] | Board.AllPieces[1]));
                
                INT8U  toIndex = 0;
                BSF(toIndex, toList);

                while (toIndex != InvalidIndex)
                {
                    INT8S flags = Board.PieceBySquare[toIndex] == FLAG_CAPTURE;

                    if (Board.PieceBySquare[toIndex] == piece::Empty)
                    {
                        toList &= ~POS(toIndex);
                        BSF(toIndex, toList);
                        continue;
                    }

                    if (IS_SAME_COLORS(Board.PieceBySquare[toIndex], Board.SideToMove))
                    {
                        toList &= ~POS(toIndex);
                        BSF(toIndex, toList);
                        continue; // hitting own piece
                    }

                    saveMove(fromIndex, toIndex, Board.PieceBySquare[fromIndex], Board.PieceBySquare[toIndex], flags);

                    toList &= ~POS(toIndex);
                    BSF(toIndex, toList);
                }
            }
        }

        return MoveCount;
    }
#endif

    inline void makeMove(Move& move)
    {
        // change side to move
        Board.SideToMove = !Board.SideToMove;

#ifdef ZOBRIST
        Board.ZobristHash ^= Zobrist.IsBlackColor;
#endif

        // if capture or pawn move -> reset ply 
        if (move.FromPiece == piece::WhitePawn || 
            move.FromPiece == piece::BlackPawn ||
            move.CapturedPiece != piece::Empty)
        {
            Board.Ply = 0;
        }
        else
        {
            ++Board.Ply;
        }

        clearSquare(move.From); // clear initial square

        // clear in case of capture
        if (move.CapturedPiece != piece::Empty)
        {
            clearSquare(move.To);
        }

        fillSquare(static_cast<color::Type>(!Board.SideToMove), static_cast<piece::Type>(move.ToPiece), move.To);

#ifdef ZOBRIST
        Board.ZobristHash ^= Zobrist.Castling[Board.Castle];
#endif

        // castle flags
        switch (move.From) {
        case square::H1: 
            Board.Castle &= ~castle::WhiteKingSide;
            break;
        case square::E1: 
            Board.Castle &= ~(castle::WhiteKingSide | castle::WhiteQueenSide);
            break;
        case square::A1: 
            Board.Castle &= ~castle::WhiteQueenSide;
            break;
        case square::H8:
            Board.Castle &= ~castle::BlackKingSide;
            break;
        case square::E8:
            Board.Castle &= ~(castle::BlackKingSide | castle::BlackQueenSide);
            break;
        case square::A8:
            Board.Castle &= ~castle::BlackQueenSide;
            break;
        }

        switch (move.To) {
        case square::H1:
            Board.Castle &= ~castle::WhiteKingSide;
            break;
        case square::E1:
            Board.Castle &= ~(castle::WhiteKingSide | castle::WhiteQueenSide);
            break;
        case square::A1:
            Board.Castle &= ~castle::WhiteQueenSide;
            break;
        case square::H8:
            Board.Castle &= ~castle::BlackKingSide;
            break;
        case square::E8:
            Board.Castle &= ~(castle::BlackKingSide | castle::BlackQueenSide);
            break;
        case square::A8: 
            Board.Castle &= ~castle::BlackQueenSide;
            break;
        }

#ifdef ZOBRIST
        Board.ZobristHash^= Zobrist.Castling[Board.Castle];
#endif

        // rook move in case of castle, calculateNewBlockingsAfterMove(square::F8)-ra nincs szukseg,
        // mert csak soros tamadoja lehet amit a kiraly learnyekolt
        if (move.Flags & FLAG_CASTLE)
        {
            if (move.To == square::G1)
            {
                clearSquare(square::H1);
                fillSquare(color::White, piece::WhiteRook, square::F1);
            }
            else if (move.To == square::C1)
            {
                clearSquare(square::A1);
                fillSquare(color::White, piece::WhiteRook, square::D1);
            }
            else if (move.To == square::G8)
            {
                clearSquare(square::H8);
                fillSquare(color::Black, piece::BlackRook, square::F8);
                
            }
            else if (move.To == square::C8)
            {
                clearSquare(square::A8);
                fillSquare(color::Black, piece:: BlackRook, square::D8);
            }
        }

        // Enpassant csak egy lepesig el, muszaj torolni, legfeljebb ujat allitunk (hibat okozott -> eltunt egy pawn)
        if (Board.Enpassant != -1)
        {
#ifdef ZOBRIST
#ifdef ZOBRIST_CPW
        Board.ZobristHash ^= Zobrist.EnPassant[Board.Enpassant + RANK(Board.Enpassant) * 8];
#else
        Board.ZobristHash ^= Zobrist.EnPassant[Board.Enpassant];
#endif
#endif
            Board.Enpassant = -1;
        }

        // set en passant on bitboard
        if ((move.FromPiece == piece::WhitePawn || move.FromPiece == piece::BlackPawn) &&
            abs(move.From - move.To) == 16 && // abs 16 means kettot lepett -> lehetne abs nelkul is
            (((move.FromPiece == piece::WhitePawn && (FILE(move.To) > 0) && Board.PieceBySquare[move.To-1] == piece::BlackPawn) || (FILE(move.To) < 7 && Board.PieceBySquare[move.To + 1] == piece::BlackPawn)) ||
             ((move.FromPiece == piece::BlackPawn && (FILE(move.To) > 0) && Board.PieceBySquare[move.To - 1] == piece::WhitePawn) || (FILE(move.To) < 7 && Board.PieceBySquare[move.To + 1] == piece::WhitePawn))) )
                                            // && b.pawn_ctrl[b.stm][(move.from + move.to) / 2]) ) // megnezi, hogy az ellenfel uralja-e pld E4-nel az E3-at  
                                                                   // lenyegeben, hogy van-e melette enemy pawn... osztas nelkul is menne a kiszamitasa
        {
            Board.Enpassant = (move.From + move.To) / 2;
#ifdef ZOBRIST
#ifdef ZOBRIST_CPW
            Board.ZobristHash ^= Zobrist.EnPassant[Board.Enpassant + RANK(Board.Enpassant) * 8];
#else
            Board.ZobristHash ^= Zobrist.EnPassant[Board.Enpassant];
#endif
#endif
        }

        // en-passant capture -> the captured pawn has to be removed manually
        if (move.Flags & FLAG_ENPASSANT_CAPTURE)
        {
            if (Board.SideToMove == color::White) 
            {
                clearSquare(move.To + 8);
            }
            else 
            {
                clearSquare(move.To - 8);
            }
        }

        HistoryCount++;
        PositionHistory[HistoryCount] = Board.ZobristHash;
    }

    inline void unmakeMove(Move& move)
    {
        // change side to move
        Board.SideToMove = !Board.SideToMove;

        Board.Ply = move.Ply;

#ifdef ZOBRIST
        Board.ZobristHash ^= Zobrist.IsBlackColor;

        if (move.Enpassant != -1)
        {
#ifdef ZOBRIST_CPW
            Board.ZobristHash ^= Zobrist.EnPassant[move.Enpassant + RANK(move.Enpassant) * 8];
#else
            Board.ZobristHash ^= Zobrist.EnPassant[move.Enpassant];
#endif
        }

        if (Board.Enpassant != -1)
        {
#ifdef ZOBRIST_CPW
            Board.ZobristHash ^= Zobrist.EnPassant[Board.Enpassant + RANK(Board.Enpassant) * 8];
#else
            Board.ZobristHash ^= Zobrist.EnPassant[Board.Enpassant];
#endif
        }
#endif

        Board.Enpassant = move.Enpassant;

        // unmake move
        clearSquare(move.To); 

        fillSquare(static_cast<color::Type>(Board.SideToMove), static_cast<piece::Type>(move.FromPiece), move.From);
        
        // unmake capture
        if (move.CapturedPiece != piece::Empty)
        {
            fillSquare(static_cast<color::Type>(!Board.SideToMove), static_cast<piece::Type>(move.CapturedPiece), move.To);
        }

        // uncastle -> rook move in case of castle
        if (move.Flags & FLAG_CASTLE)
        {
            if (move.To == square::G1)
            {
                clearSquare(square::F1);
                fillSquare(color::White, piece::WhiteRook, square::H1);
            }
            else if (move.To == square::C1)
            {
                clearSquare(square::D1);
                fillSquare(color::White, piece::WhiteRook, square::A1);
            }
            else if (move.To == square::G8)
            {
                clearSquare(square::F8);
                fillSquare(color::Black, piece::BlackRook, square::H8);
            }
            else if (move.To == square::C8)
            {
                clearSquare(square::D8);
                fillSquare(color::Black, piece::BlackRook, square::A8);
            }
        }

        SWITCH_ON_ZOBRIST_CASTLE(move.Castle)
        
        // adjust castling flag
        Board.Castle = move.Castle;

        // unmake en-passant capture
        if (move.Flags & FLAG_ENPASSANT_CAPTURE)
        {
            if (Board.SideToMove == color::White)
            {
                fillSquare(color::Black, piece::BlackPawn, move.To - 8);
            }
            else
            {
                fillSquare(color::White, piece::WhitePawn, move.To + 8);
            }
        }

        HistoryCount--;
    }

    inline void sortMove(INT8U moveCount, Move* moveList, INT8U current) 
    {
        //find the move with the highest score - hoping for an early cutoff
        INT8U highest = current;

        for (int i = current + 1; i < moveCount; ++i) 
        {
            if (moveList[i].Score > moveList[highest].Score)
            {
                highest = i;
            }
        }

        Move temp = moveList[highest];
        moveList[highest] = moveList[current];
        moveList[current] = temp;
    }

#ifdef SWITCH_TO_NORMAL_MOVE_GENERATION
    inline bool isBadCapture(const Move& move)
    {
        // captures by pawn won't lose material -> good capture, returning false 
        if (move.FromPiece == piece::WhitePawn || move.FromPiece == piece::BlackPawn)
        {
            return false;
        }

        // Captures "lower takes higher" (as well as BxN) are good by definition
        if (EvaluationData.PieceValue[move.CapturedPiece] >= EvaluationData.PieceValue[move.FromPiece] - 50)
        {
            return false;
        }

        /*
         * When the enemy piece is defended by a pawn, in the quiescence search
         * We  will  accept rook takes minor, but not minor takes pawn. ( More
         * exact  version  should accept B/N x P if (a) the pawn  is  the  sole
         * defender and (b) there is more than one attacker.
         * minor takes pawn (pawn takes minor) -> bad capture (fv neve bad capture -> return true)
         * rook takes pawn (pawn takes rook)-> good capture
         */
        if (isAttackedByPawn(static_cast<INT8S>(Board.PieceBySquare[static_cast<INT8U>(move.From)] & 1), move.To) &&
            EvaluationData.PieceValue[move.CapturedPiece] < EvaluationData.PieceValue[move.FromPiece] - 200)
        {
            return true;
        }

        //queen captures pawn / minor, and minor captures queen is bad deal -> bad capture 
        if (EvaluationData.PieceValue[move.CapturedPiece] < EvaluationData.PieceValue[move.FromPiece] - 500)
        {
            if (move.FromPiece & 1)
            {
                if (hasLeaperAttack(move.To, piece::BlackKnight)) return true;
                if (hasBishopAttack(color::Black, move.To, NE)) return true;
                if (hasBishopAttack(color::Black, move.To, NW)) return true;
                if (hasBishopAttack(color::Black, move.To, SE)) return true;
                if (hasBishopAttack(color::Black, move.To, SW)) return true;
            }
            else
            {
                if (hasLeaperAttack(move.To, piece::WhiteKnight)) return true;
                if (hasBishopAttack(color::White, move.To, NE)) return true;
                if (hasBishopAttack(color::White, move.To, NW)) return true;
                if (hasBishopAttack(color::White, move.To, SE)) return true;
                if (hasBishopAttack(color::White, move.To, SW)) return true;
            }
        }

        // if a capture is not processed -> it can't be considered bad 
        return false;
    }
#else
    inline bool isBadCapture(const Move& move)
    {
        // captures by pawn won't lose material -> good capture, returning false 
        if (move.FromPiece == piece::WhitePawn || move.FromPiece == piece::BlackPawn)
        {
            return false;
        }

        // Captures "lower takes higher" (as well as BxN) are good by definition
        if (EvaluationData.PieceValue[move.CapturedPiece] >= EvaluationData.PieceValue[move.FromPiece] - 50)
        {
            return false;
        }

        /*
         * When the enemy piece is defended by a pawn, in the quiescence search
         * We  will  accept rook takes minor, but not minor takes pawn. ( More
         * exact  version  should accept B/N x P if (a) the pawn  is  the  sole
         * defender and (b) there is more than one attacker.
         * minor takes pawn (pawn takes minor) -> bad capture (fv neve bad capture -> return true)
         * rook takes pawn (pawn takes rook)-> good capture
         */
        if (isAttackedByPawn(static_cast<INT8S>(Board.PieceBySquare[static_cast<INT8U>(move.From)] & 1), move.To) &&
            EvaluationData.PieceValue[move.CapturedPiece] < EvaluationData.PieceValue[move.FromPiece] - 200)
        {
            return true;
        }

        //queen captures pawn / minor, and minor captures queen is bad deal -> bad capture 
        if (EvaluationData.PieceValue[move.CapturedPiece] < EvaluationData.PieceValue[move.FromPiece] - 500)
        {
            if (move.FromPiece & 1)
            {
                if ((attacks_bb(piece::WhiteKnight, move.To) & Board.SquareByPiece[piece::BlackKnight]) |
                    (attacks_bb(piece::WhiteBishop, move.To) & Board.SquareByPiece[piece::BlackBishop]))
                {
                    return true;
                }
            }
            else
            {
                if ((attacks_bb(piece::WhiteKnight, move.To) & Board.SquareByPiece[piece::WhiteKnight]) |
                    (attacks_bb(piece::WhiteBishop, move.To) & Board.SquareByPiece[piece::WhiteBishop]))
                {
                    return true;
                }
            }
        }

        // if a capture is not processed -> it can't be considered bad 
        return false;
    }
#endif

    inline bool isPromotionMove(Move& move) 
    {
        return (move.FromPiece != move.ToPiece);
    }

    inline bool isCaptureMove(Move& move) {
        return (move.CapturedPiece != piece::Empty);
    }

    inline bool canMoveSimplified(Move& move) 
    {
        if (move.CapturedPiece == piece::WhitePawn || move.CapturedPiece == piece::BlackPawn ||
            Board.PieceMaterial[!Board.SideToMove] - EvaluationData.PieceValue[move.CapturedPiece] > EndGameMatrial)
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    inline int move_makeNull() {
        Board.SideToMove = !Board.SideToMove;
        Board.ZobristHash ^= Zobrist.IsBlackColor;
        if (Board.Enpassant != -1) {
            Board.ZobristHash ^= Zobrist.EnPassant[Board.Enpassant];
            Board.Enpassant = -1;
        }
        return 0;
    }

    inline int move_unmakeNull(INT8S ep) {
        Board.SideToMove = !Board.SideToMove;
        Board.ZobristHash ^= Zobrist.IsBlackColor;
        if (ep != -1) {
            Board.ZobristHash ^= Zobrist.EnPassant[ep];
            Board.Enpassant = ep;
        }
        return 0;
    }
}

#endif // MOVER_H
