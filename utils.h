#ifndef UTILS_H
#define UTILS_H

#ifndef __unix__
#include <intrin.h>
#endif

#include <iostream>
#include "macro.h"
#include <chrono>
#include <string>

using INT8U = unsigned char;
using INT8S = char;
using INT16U = unsigned short;
using INT32U = unsigned long;
using INT64U = unsigned long long;
using LineId = unsigned char;
using UINT = unsigned; // unsigned int

const INT8U InvalidIndex = 100;

#define INF 10000

#ifdef ZOBRIST
#define SORT_ZOBRIST 100000000
#endif

constexpr int SORT_KING_CAPTURE = 400000000;
#define SORT_CAPTURE 100000000
#define SORT_HASH 200000000
#define SORT_PROMOTION 90000000
#define SORT_KILL 80000000

#define FLAG_NORMAL 0
#define FLAG_CAPTURE 1
#define FLAG_ENPASSANT_CAPTURE 2
#define FLAG_CASTLE 4
#define FLAG_ENPASSANT 8
#define FLAG_PROMOTION 16

// file is column, rank is row
#define FILE(square) ((square) & 7) 
#define RANK(square) ((square) >> 3)

// tell if x is a square on board

#define LSB(bits) (bits & ~(bits - 1)) 

#define POS(index) ((index == InvalidIndex) ? 0 : (1ULL << index))
#define POS_RF(rank, file) (8 * rank + file)
#define INDEX(rank, file) ((rank << 3) + file)

#define IS_SQ_NW(x) ( (RANK(x) < 7 && FILE(x) > 0) ? (1) : (0) )
#define IS_SQ_NE(x) ( (RANK(x) < 7 && FILE(x) < 7) ? (1) : (0) )
#define IS_SQ_SW(x) ( (RANK(x) > 0 && FILE(x) > 0) ? (1) : (0) )
#define IS_SQ_SE(x) ( (RANK(x) > 0 && FILE(x) < 7) ? (1) : (0) )

#define IS_SQ_AFTER_LEAP_ONE(result, fromSquare, toSquare) \
if (toSquare > 63 || toSquare < 0) result = false; /* kivedi a sor szeleket, es ha tulcsordul minuszba, akkor is jo a " > 7" */ \
else if ( FILE(fromSquare) == 0 && (FILE(toSquare) != 1 && FILE(toSquare) != 0 && FILE(toSquare) != 2)) result = false; \
else if ( FILE(fromSquare) == 7 && (FILE(toSquare) != 6 && FILE(toSquare) != 7 && FILE(toSquare) != 5)) result = false; \
else if ( FILE(fromSquare) == 6 && FILE(toSquare) == 0) result = false; \
else if ( FILE(fromSquare) == 1 && FILE(toSquare) == 7) result = false; \
else result = true;

#define IS_WHITE(checkedPiece) ((checkedPiece & 1) ? (1) : (0))
#define IS_BLACK(checkedPiece) (((checkedPiece != piece::Empty) && (checkedPiece & 1) == 0) ? (1) : (0))
#define IS_SAME_COLORS(checkedPiece, color) ((checkedPiece & 1) != (color & 1) ? (1) : (0))

#ifdef __unix__
#define BSR(result, number) \
    if (number) \
    { \
        result = static_cast<INT8U>(63-__builtin_clzll(number)); \
    } \
    else \
    { \
        result = InvalidIndex; \
    } \

#define BSF(result, number) \
    if (number) \
    { \
        result = static_cast<INT8U>(__builtin_ctzll(number)); \
    }  \
    else \
    { \
        result = InvalidIndex; \
    } \

#define BSR64(result, number) \
    if (number) \
    { \
        result = POS(63-__builtin_clzll(number)); \
    }  \
    else \
    { \
        result = InvalidIndex; \
    } \

#define BSF64(result, number) \
    if (number) \
    { \
        result = POS(__builtin_ctzll(number)); \
    }  \
    else \
    { \
        result = InvalidIndex; \
    } \

#define POP_COUNT(number) static_cast<INT8U>(__builtin_popcountll(number))

#else

#define BSR(result, number) \
    if (number) \
    { \
        INT32U position = 0; \
        _BitScanReverse64(&position, number); \
        result = static_cast<INT8U>(position); \
    }  \
    else \
    { \
        result = InvalidIndex; \
    } \

#define BSF(result, number) \
    if (number) \
    { \
        INT32U position = 0; \
        _BitScanForward64(&position, number); \
        result = static_cast<INT8U>(position); \
    }  \
    else \
    { \
        result = InvalidIndex; \
    } \

#define BSR64(result, number) \
    if (number) \
    { \
        INT32U msbPosition = 0; \
        _BitScanReverse64(&msbPosition, number); \
        result = POS(msbPosition); \
    }  \
    else \
    { \
        result = 0; \
    } \

#define BSF64(result, number) \
    if (number) \
    { \
        INT32U msbPosition = 0; \
        _BitScanForward64(&msbPosition, number); \
        result = POS(msbPosition); \
    }  \
    else \
    { \
        result = 0; \
    } \

#define POP_COUNT(number) static_cast<INT8U>(__popcnt64(number))

#endif

// TODO: depricated -> use RankMask instead of it
const INT64U RowMask[8] = { 0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'11111111,
                            0b00000000'00000000'00000000'00000000'00000000'00000000'11111111'00000000,
                            0b00000000'00000000'00000000'00000000'00000000'11111111'00000000'00000000,
                            0b00000000'00000000'00000000'00000000'11111111'00000000'00000000'00000000,
                            0b00000000'00000000'00000000'11111111'00000000'00000000'00000000'00000000,
                            0b00000000'00000000'11111111'00000000'00000000'00000000'00000000'00000000,
                            0b00000000'11111111'00000000'00000000'00000000'00000000'00000000'00000000,
                            0b11111111'00000000'00000000'00000000'00000000'00000000'00000000'00000000
                          };

//const INT64U FileMaskByIndex[8] = { 0b00000001'00000001'00000001'00000001'00000001'00000001'00000001'00000001,
//                                    0b00000010'00000010'00000010'00000010'00000010'00000010'00000010'00000010,
//                                    0b00000100'00000100'00000100'00000100'00000100'00000100'00000100'00000100,
//                                    0b00001000'00001000'00001000'00001000'00001000'00001000'00001000'00001000,
//                                    0b00010000'00010000'00010000'00010000'00010000'00010000'00010000'00010000,
//                                    0b00100000'00100000'00100000'00100000'00100000'00100000'00100000'00100000,
//                                    0b01000000'01000000'01000000'01000000'01000000'01000000'01000000'01000000,
//                                    0b10000000'10000000'10000000'10000000'10000000'10000000'10000000'10000000
//};

const INT64U RankCollectionMask[8] = { 0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'11111111,
                                       0b00000000'00000000'00000000'00000000'00000000'00000000'11111111'11111111,
                                       0b00000000'00000000'00000000'00000000'00000000'11111111'11111111'11111111,
                                       0b00000000'00000000'00000000'00000000'11111111'11111111'11111111'11111111,
                                       0b00000000'00000000'00000000'11111111'11111111'11111111'11111111'11111111,
                                       0b00000000'00000000'11111111'11111111'11111111'11111111'11111111'11111111,
                                       0b00000000'11111111'11111111'11111111'11111111'11111111'11111111'11111111,
                                       0b11111111'11111111'11111111'11111111'11111111'11111111'11111111'11111111
                                     }; // x-edik sorig 

const INT64U RankCollectionMaskInverse[8] = { 0b11111111'11111111'11111111'11111111'11111111'11111111'11111111'11111111,
                                              0b11111111'11111111'11111111'11111111'11111111'11111111'11111111'00000000,
                                              0b11111111'11111111'11111111'11111111'11111111'11111111'00000000'00000000,
                                              0b11111111'11111111'11111111'11111111'11111111'00000000'00000000'00000000,
                                              0b11111111'11111111'11111111'11111111'00000000'00000000'00000000'00000000,
                                              0b11111111'11111111'11111111'00000000'00000000'00000000'00000000'00000000,
                                              0b11111111'11111111'00000000'00000000'00000000'00000000'00000000'00000000,
                                              0b11111111'00000000'00000000'00000000'00000000'00000000'00000000'00000000
                                            }; // x-edik sor vagy a folott


namespace square {
    enum Type : INT8S
    {
        A1 = 0, B1, C1, D1, E1, F1, G1, H1,
        A2 = 8, B2, C2, D2, E2, F2, G2, H2,
        A3 = 16, B3, C3, D3, E3, F3, G3, H3,
        A4 = 24, B4, C4, D4, E4, F4, G4, H4,
        A5 = 32, B5, C5, D5, E5, F5, G5, H5,
        A6 = 40, B6, C6, D6, E6, F6, G6, H6,
        A7 = 48, B7, C7, D7, E7, F7, G7, H7,
        A8 = 56, B8, C8, D8, E8, F8, G8, H8,
    };
};

struct Move
{
    int Score;
    INT8U From;
    INT8U To;
    INT8U FromPiece;
    INT8U ToPiece;
    INT8U CapturedPiece;
    char Castle;
    INT8S Enpassant;
    INT8U Ply; // save the actual board Ply here (role is when unmaking move)
    char Flags;
};

namespace piece {
    /*
    * in evaluation PieceDistanceBonus is depends on the order of the pieces
    */
    enum Type : INT8U
    {
        Empty = 0,
        WhitePawn,
        BlackPawn,
        WhiteKnight,
        BlackKnight,
        WhiteBishop,
        BlackBishop,
        WhiteRook,
        BlackRook,
        WhiteQueen,
        BlackQueen,
        WhiteKing, // important order of king for SEE (for get least valuable attacker function)
        BlackKing, // fehér - fekete - fehér ... enum tagok váltakozása kell a SEE-nél (least valuable attacker-nél)
        Count
    };
};

namespace color {
    enum Type : INT8U
    {
        White,
        Black,
        Empty
    };
}

namespace castle {
    enum Type : INT8U
    {
        WhiteKingSide = 1,
        WhiteQueenSide = 2,
        BlackKingSide = 4,
        BlackQueenSide = 8
    };
}

struct BitBoard
{
public:
    piece::Type PieceBySquare[64];
    INT64U SquareByPiece[piece::Type::Count];
    INT64U PawnControl[2];
    /* *********************************** */
    INT64U AllPieces[2];
    INT64U ZobristHash;
    short int PieceMaterial[2];
    short int PawnMaterial[2];
    short int MiddleGamePositionScore[2];
    short int EndGamePositionScore[2];
    INT8S Enpassant; // az aki legutojara 2 lepett pawn-nal, az enpassant egy lepesig el...
                     // eggyel ala mutat a lepett pawn-nak. Pld. E4-nel E3
    INT8U Ply; // how many steps without capture
    char Castle;     // 1 = shortW, 2 = longW, 4 = shortB, 8 = longB
    INT8U SideToMove; // 0 : white or 1 black -> who will step next (has the right to step)
};

#ifdef ZOBRIST
INT8U getCpwPieceType(piece::Type pieceType);

constexpr int ZobristSize = 0x800000; 
extern int ZobristMask;

#ifdef ZOBRIST_CPW
struct szobrist {
    INT64U PieceSquare[6][2][128];
    INT64U IsBlackColor;
    INT64U Castling[16];
    INT64U EnPassant[128];
};
#else
struct szobrist {
    INT64U PieceSquare[piece::Count][64];
    INT64U IsBlackColor;
    INT64U Castling[16];
    INT64U EnPassant[64];
};
#endif

extern szobrist Zobrist;
extern INT8U ZobristTo;
extern INT8U ZobristFrom;

extern int ttsave;
extern int ttconflict;
extern int tthit;
extern int tthit2;

enum ettflag {
    TT_EXACT,
    TT_ALPHA,
    TT_BETA
};

struct TranspositionEntry
{
    INT64U Hash;
    short int Score;
    INT8U BestTo;
    INT8U BestFrom;
    INT8U Depth;
    INT8U Flags = 255; // tteval_save needs to set default
};
#endif

struct EvaluationDataType {
    int SortValue[13];
    short int PieceValue[13];
    short int CaptureValue[13];
    int MiddleGamePieceScoreTable[piece::Count][64];
    int EndGamePieceScoreTable[piece::Count][64];
};

#define NORTH  8
#define NN (NORTH + NORTH)
#define SOUTH -8
#define SS (SOUTH + SOUTH)
#define EAST 1
#define WEST -1
#define NE 9
#define SW -9
#define NW 7
#define SE -7

extern std::string wlog;

const short int EndGameMatrial = 1300;
const short int InvalidScore = 9000; // make distance from INF

extern INT8U MaxDepth;

extern BitBoard Board;

extern Move* MoveList;
extern INT8U MoveCount;

extern Move BestMove;

namespace bestmove {
    enum Type : INT8U
    {
        Normal = 0,
        StaleMate = 1,
        NoMoveInCheckMate = 2
    };
}

extern bestmove::Type BestMoveType;

extern INT64U PositionHistory[1024];
extern INT16U HistoryCount;
extern color::Type MySide;
extern INT64U InitialBoardHash;

extern int Nodes;
extern int QuiescenceNodes;
extern int EvaluatedNodes;
extern int LogNodes;

extern EvaluationDataType EvaluationData;

extern INT64U FirstRankAttacks64x8[512];
extern INT64U FileAttacksDown[64];
extern INT64U FileAttacksUp[64];
extern INT64U DiagonalAttacksDown[64];
extern INT64U DiagonalAttacksUp[64];
extern INT64U AntiDiagonalAttacksDown[64];
extern INT64U AntiDiagonalAttacksUp[64];
extern INT64U PawnAttacksWhite[64];
extern INT64U PawnAttacksBlack[64];
extern INT64U EnPassantPawnAttacksWhite[64];
extern INT64U EnPassantPawnAttacksBlack[64];
extern INT64U KnightAttacks[64];
extern INT64U KingAttacks[64];

#ifdef ZOBRIST
#ifndef ZOBRIST_CPW
extern TranspositionEntry TranspositionTable[ZobristSize]; // kb 250 - 300 MB, kb 40 byte egy entry
//extern TranspositionEntry* TranspositionTable;
#endif
#endif

void clearSearch();
void initializeHelpers();

inline  void convertFromSquare(INT8S from, INT8S to, char * a) 
{
    a[0] = FILE(from) + 'a';
    a[1] = RANK(from) + '1';
    a[2] = FILE(to) + 'a';
    a[3] = RANK(to) + '1';
    a[4] = 0;
}

constexpr INT64U FileA = 0x0101010101010101ULL;
constexpr INT64U FileB = FileA << 1;
constexpr INT64U FileC = FileA << 2;
constexpr INT64U FileD = FileA << 3;
constexpr INT64U FileE = FileA << 4;
constexpr INT64U FileF = FileA << 5;
constexpr INT64U FileG = FileA << 6;
constexpr INT64U FileH = FileA << 7;

constexpr INT64U Rank1 = 0xFF;
constexpr INT64U Rank2 = Rank1 << (8 * 1);
constexpr INT64U Rank3 = Rank1 << (8 * 2);
constexpr INT64U Rank4 = Rank1 << (8 * 3);
constexpr INT64U Rank5 = Rank1 << (8 * 4);
constexpr INT64U Rank6 = Rank1 << (8 * 5);
constexpr INT64U Rank7 = Rank1 << (8 * 6);
constexpr INT64U Rank8 = Rank1 << (8 * 7);

enum Direction : int {
    NORTH_MAGIC = 8,
    EAST_MAGIC = 1,
    SOUTH_MAGIC = -NORTH_MAGIC,
    WEST_MAGIC = -EAST_MAGIC,

    NORTH_EAST_MAGIC = NORTH_MAGIC + EAST_MAGIC,
    NORTH_WEST_MAGIC = NORTH_MAGIC + WEST_MAGIC,
    SOUTH_EAST_MAGIC = SOUTH_MAGIC + EAST_MAGIC,
    SOUTH_WEST_MAGIC = SOUTH_MAGIC + WEST_MAGIC
};

extern INT64U RankMask[8];
extern INT64U FileMask[8];

extern int SquareDistance[64][64];

/*
* distance between x and y, defined as the number of steps for the king in x to reach y
*/
inline int distance(INT8U x, INT8U y)
{
    return SquareDistance[x][y];
}

inline int distance_file(INT8U x, INT8U y)
{
    return std::abs(FILE(x) - FILE(y));
}

inline int distance_rank(INT8U x, INT8U y)
{
    return std::abs(RANK(x) - RANK(y));
}

constexpr bool is_ok(INT8U square)
{
    return square >= 0 && square <= 63;
}

inline INT64U sliding_attack(Direction directions[], INT8U originalSquare, INT64U occupied)
{
    INT64U attack = 0;

    for (int i = 0; i < 4; ++i)
    {
        for (INT8U square = originalSquare + directions[i];
            is_ok(square) && distance(square, square - directions[i]) == 1;
            square += directions[i])
        {
            attack |= POS(square);

            if (occupied & POS(square))
            {
                break;
            }
        }
    }

    return attack;
}

#ifdef ZOBRIST
void tteval_save(INT8U depth,
    int score,
    int flags,
    INT8U from,
    INT8U to);

INT64U calculateZobristHash();

short int tteval_probe(INT8U depth, short int alpha, short int beta);

INT64U rand64();
void initializeTranspositionTable();
int tt_init();
#endif

#ifdef ZOBRIST_INTEGRITY_CHECK
inline bool checkZobristIntegrity()
{
    INT64U originalZobrist = Board.ZobristHash;
    Board.ZobristHash = 0;
    calculateZobristHash();
    if (originalZobrist != Board.ZobristHash)
    {
        wlog += " integrity error start\n";
        wlog += std::to_string(originalZobrist) + std::string("  ccc ") + std::to_string(Board.ZobristHash);
        wlog += "...\n";
        int y = 0;
        y++;
        return false;
    }

    return true;
}
#endif

struct LogNode
{
    INT8U From = 255;
    INT8U To = 255;
};

using LogNodeArray = LogNode[64];

std::string getPathAsString(INT8U PathDepth, const LogNodeArray& path);

extern bool slide[10];
extern char vectors[10];
extern char vector[10][8];

extern std::chrono::system_clock::time_point StartingTime;
const int TimeOutInMs = 110 * 1000; // after 110 sec the best move is retuned

inline bool checkTimeOut()
{
    auto elipsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - StartingTime);
    return (int)elipsedTime.count() > TimeOutInMs;
}

#endif // UTILS_H
