#ifndef MACROUTILITY_H
#define MACROUTILITY_H

#include "macro.h"
#include "utils.h"
#include "macroutils.h"

#ifdef PVLOG

struct LogNodePath
{
    LogNode Nodes[64];
    int Score = -INF;
    INT8U Depth = 0;
};

extern LogNodePath NodePath[64];

extern LogNodePath TempPath;

#define ADD_HISTORY(depth, from, to) \
    TempPath.Nodes[depth].From = from; \
    TempPath.Nodes[depth].To = to; \

#define ADD_PV_HISTORY(depth, from, to, score) \
    TempPath.Nodes[depth].From = from; \
    TempPath.Nodes[depth].To = to; \
    TempPath.Score = (depth & 1) == 0 ? score : -score; \
    TempPath.Depth = depth; \
    \
    NodePath[depth].Score = TempPath.Score; \
    NodePath[depth].Depth = TempPath.Depth; \
    for (int d = 0; d < depth ; ++d) \
    { \
        NodePath[depth].Nodes[d].From = TempPath.Nodes[d].From; \
        NodePath[depth].Nodes[d].To = TempPath.Nodes[d].To; \
    } \

#define COPY_PV_HISTORY(depth) \
    NodePath[depth].Score = NodePath[depth+1].Score; \
    NodePath[depth].Depth = NodePath[depth+1].Depth; \
    for (int d = 0; d < NodePath[depth+1].Depth ; ++d) \
    { \
        NodePath[depth].Nodes[d].From = NodePath[depth+1].Nodes[d].From; \
        NodePath[depth].Nodes[d].To = NodePath[depth+1].Nodes[d].To; \
    } \

#define LOG_BEST_MOVE() \
    for (int logIndex = 0; logIndex < NodePath[0].Depth; ++logIndex) \
    { \
        char moveStr[5]; \
        convertFromSquare(NodePath[0].Nodes[logIndex].From, NodePath[0].Nodes[logIndex].To, moveStr); \
        std::cout << moveStr << " " << NodePath[0].Score << std::endl; \
    } \

#else // PVNODE

#define ADD_HISTORY(depth, from, to) //empty
#define ADD_PV_HISTORY(depth, from, to, score)
#define COPY_PV_HISTORY(depth)

#define LOG_BEST_MOVE() \
    char moveStr[5]; \
    convertFromSquare(BestMove.From, BestMove.To, moveStr); \
    std::cout << moveStr << " " << BestMove.Score << std::endl; \

#endif

#ifdef BOARD_CHECK

struct BoardIntegrity
{
    short WhiteScore;
    short BlackScore;
};

extern BoardIntegrity Integrity[64];

#define SAVE_BOARD_SCORES(depth) \
Integrity[depth].WhiteScore = Board.PieceMaterial[0]; \
Integrity[depth].BlackScore = Board.PieceMaterial[1]; \

void checkBoardIntegrity(INT8U depth);

#define CHECK_BOARD_INTEGRITY_MOVE() testBoardAtkTo(); 
#define CHECK_BOARD_INTEGRITY_UNMOVE(depth) checkBoardIntegrity(depth); 

#else

#define SAVE_BOARD_SCORES(depth)

void checkBoardIntegrity(INT8U depth);

#define CHECK_BOARD_INTEGRITY_MOVE()
#define CHECK_BOARD_INTEGRITY_UNMOVE(depth)

#endif

#ifdef LOG_ENABLED

#define LOG_TEXT(text) \
    std::cout << text << std::endl;

#define LOG_MOVE(text, depth, move, score, alpha, beta, isPV) \
    char moveStr[5]; \
    LogNodes++; \
    convertFromSquare(move.From, move.To, moveStr); \
    char text2[3] = {text[0], isPV ? 'p' : text[1], '\0'}; \
    if (LogNodes % 10 == 0) \
    { \
        std::string result = getPathAsString(incrementedMaxDepth, Pv2NodePath); \
        std::cout << LogNodes << " " << text2 << static_cast<int>(depth) << " " << result.c_str() << \
            moveStr << " " << score << " " << alpha << " " << beta << " " << static_cast<int>(MaxDepth) << std::endl; \
    } \
    LOG_TEXT(text2 << static_cast<int>(depth) << " " << moveStr << " " << score) \

#define LOG_MOVE2(text, depth, reverseDepth, score, alpha, beta) \
    Move move; \
    move.From = 0; \
    move.To = 0; \
    LOG_MOVE(text, depth, move, score, alpha, beta, false) \

#elif defined LOG_SEARCH_CPW_STYLE

#define LOG_TEXT(text) \
    std::cout << text << std::endl;

#define LOG_MOVE(text, depth, move, score, alpha, beta, isPV) \
    char moveStr[5]; \
    LogNodes++; \
    convertFromSquare(move.From, move.To, moveStr); \
    char text2[3] = {text[0], isPV ? 'p' : text[1], '\0'}; \
    std::cout << " " << text2 << static_cast<int>(depth) << " " << \
        moveStr << " " << score << " " << alpha << " " << beta << " " << std::endl; \

#define LOG_MOVE2(text, depth, score, alpha, beta) \
    Move move; \
    move.From = 0; \
    move.To = 0; \
    LOG_MOVE(text, depth, move, score, alpha, beta, false) \

#else

#define LOG_MOVE(text, depth, move, score, alpha, beta, isPV)
#define LOG_MOVE2(text, depth, score, alpha, beta)
#define LOG_TEXT(text)

#endif

#ifdef ZOBRIST
#define SWITCH_ON_ZOBRIST_CASTLE(castle) \
Board.ZobristHash ^= Zobrist.Castling[castle]; \
Board.ZobristHash ^= Zobrist.Castling[Board.Castle]; 
#else
#define SWITCH_ON_ZOBRIST_CASTLE(castle)
#endif

#ifdef LOG_NEXT_MOVE

struct NextLogNode
{
    INT8U From = 255;
    INT8U To = 255;
};

constexpr int CheckedDepth = 5;

extern NextLogNode NextNodePath[CheckedDepth];

extern bool DepthFound[CheckedDepth];

#define PREPARE_LOG_NEXT_MOVE(move, MaxDepth, depth) \
    int realDepth = (MaxDepth == -1) ? depth : (MaxDepth - depth); \
    if (realDepth < CheckedDepth ) \
    { \
        if (NextNodePath[realDepth].To == move.To && NextNodePath[realDepth].From == move.From) \
        { \
            DepthFound[realDepth] = true; \
        } \
        else \
        { \
            DepthFound[realDepth] = false; \
        } \
    }

#define CHECK_LOG_NEXT_MOVE(move, MaxDepth, depth, score, alpha, beta, code) \
    int realDepth = (MaxDepth == -1) ? depth : (MaxDepth - depth); \
    if (realDepth == CheckedDepth) \
    { \
        bool isLogNeeded = true; \
        for (int depthIndex = 0; depthIndex < CheckedDepth; ++depthIndex) \
        { \
            if (!DepthFound[depthIndex]) \
            { \
                isLogNeeded = false; \
                break; \
            } \
        } \
        \
        if (isLogNeeded) \
        { \
            char moveStr[5]; \
            convertFromSquare(move.From, move.To, moveStr); \
            std::cout << "Next move log: " << moveStr << " " << score << " " << alpha << " " << beta << " " << code << std::endl; \
        } \
    }

#else

#define PREPARE_LOG_NEXT_MOVE(move, MaxDepth, depth)
#define CHECK_LOG_NEXT_MOVE(move, MaxDepth, depth, score, alpha, beta, code)

#endif

#ifdef SEE_BLIND

#define SEE(move) \
if (see_Blind(move)) \
{  \
    move.Score = EvaluationData.SortValue[capturedPiece] + EvaluationData.CaptureValue[fromPiece] + SORT_CAPTURE; \
} \
else \
{ \
     move.Score = EvaluationData.SortValue[capturedPiece] + EvaluationData.CaptureValue[fromPiece]; \
} 

#else

#define SEE(move) \
move.Score = EvaluationData.SortValue[capturedPiece] + EvaluationData.CaptureValue[fromPiece] + SORT_CAPTURE; 

#endif

#ifdef HISTORY
extern int History[2][64][64];

void clearHistoryTable();

#define SAVE_HISTORY_MOVE() \
            /* \
             * On a quiet move (when beta cut-off?) update killer moves and history table \
             * in order to enhance move ordering \
             */ \
            if (!mover::isCaptureMove(move) && !mover::isPromotionMove(move)) \
            { \
                History[Board.SideToMove][move.From][move.To] += depth * depth; \
            } \
            \
            /* \
             * With a super deep history table would overflow \
             * - let's prevent it.\
             */ \
            \
            if (History[Board.SideToMove][move.From][move.To] > SORT_KILL) { \
                for (int cl = 0; cl < 2; cl++) \
                for (int a = 0; a < 128; a++) \
                for (int b = 0; b < 128; b++) { \
                    History[cl][a][b] = History[cl][a][b] / 2; \
                } \
            }

#else 

#define SAVE_HISTORY_MOVE()

#endif

#ifdef LOG_SEARCH_ENABLED
#define SEARCH(depth, ply, alpha, beta, isNullAllowed, isPV) search(depth, ply, alpha, beta, isNullAllowed, isPV, INT8U& incrementedMaxDepth, INT8U reversedDepth)
#define SEARCH_CALLED(depth, ply, alpha, beta, isNullAllowed, isPV) search(depth, ply, alpha, beta, isNullAllowed, isPV, incrementedMaxDepth, reversedDepth)
#define SEARCH_CALLED_FIRST(result, depth, ply, alpha, beta, isNullAllowed, isPV) INT8U incrementedMaxDepth = MaxDepth; INT8U reversedDepth = 0; result = SEARCH_CALLED(depth, ply, alpha, beta, isNullAllowed, isPV)
#define INCREMENT_REVERSED_DEPTH() reversedDepth++;
#define INCREMENT_MAX_DEPTH() incrementedMaxDepth++; 
#define SAVE_MAX_INCEMENTED_DEPTH() INT8U savedIncrementedDepth = incrementedMaxDepth;
#define RESTORE_MAX_INCEMENTED_DEPTH() incrementedMaxDepth = savedIncrementedDepth;
#else
#define SEARCH(depth, ply, alpha, beta, isNullAllowed, isPV) search(depth, ply, alpha, beta, isNullAllowed, isPV)
#define SEARCH_CALLED(depth, ply, alpha, beta, isNullAllowed, isPV) search(depth, ply, alpha, beta, isNullAllowed, isPV)
#define SEARCH_CALLED_FIRST(result, depth, ply, alpha, beta, isNullAllowed, isPV) result = SEARCH_CALLED(depth, ply, alpha, beta, isNullAllowed, isPV)
#define INCREMENT_REVERSED_DEPTH()
#define SAVE_MAX_INCEMENTED_DEPTH()
#define RESTORE_MAX_INCEMENTED_DEPTH()
#endif

#ifdef PV_SEARCH

#define RESEARCH() \
        /* \
         * The code below introduces principal variation search. It means \
         * that once we are in a PV node (indicated by IS_PV flag) and we \
         * have found a move that raises alpha, we assume that the rest \
         * moves ought to be refuted. This is done relativly cheaply by using \
         * a null window search centered around alpa. Only if this search fails high, \
         * we are forced repeat it with full window. \
         * \
         * Understanding the shorthend in the first two lines is a bit tricky \
         * If alpha has not been raised, we might be either in a zero window (scout) node \
         * or in an open window (pv) node, entered after a scout search failed high. \
         * In both cases, we need to search with the same alpha, the same beta AND the same node type \
         * \
         * A kovetkezo if-else resz az ugynevezett PVS searching \
         * -search(depth - 1, -alpha - 1, -alpha, false) > alpha \
         * azt jelenti, hogyha csak beta cut off van (csak rosszabbakat talal, mint a PV node), \
         * akkor a search eredmenye az elozo alpha (az uj beta). \
         * Tehat bizonyitottan a kis "ablakban" (alpha korul keresve).nincs jobb eredmeny. \
         * Ha megis lenne jobb eredmeny, akkor az alpha cut off-ot okoz (fail soft?). \
         * Hiszen maximum 1-gyel jobbat talalok a kovetkezo korben a jelenlegi beta-tol (regi alpha).\
         * Mert ha pld alpha = 100 -> beta kovetkezo = -100, \
         * akkor legjobb esetben is csak -101 lehet a kovetkezo alphaja, hiszen a masik \
         * bebiztositotta maganak a -100-at, attol csak rosszabb lehet.\
         * Tehat az -(-alpha-1) = alpha + 1 -gyel terunk vissza, ami nagyobb lesz, mint az alpha. \
         * A pontos erteket a teljes ablakos keresessel kapjuk meg. \
         * PV node az a node, ami eljut a kereses gyokereig, lehet kesobb kiesik. \
         * Altalaban PV node-ok szama peldaul 55 volt 5 millio node eseten \
         */ \
        if (!raised_alpha) \
        { \
            score = -SEARCH_CALLED(depth - 1, ply + 1, -beta, -alpha, true, isPV); \
        } \
        else \
        { \
            /* first try to refute a move - if this fails, do a real search */ \
            if (-SEARCH_CALLED(depth - 1, ply + 1, -alpha - 1, -alpha, true, false) > alpha) \
            { \
                score = -SEARCH_CALLED(depth - 1, ply + 1, -beta, -alpha, true, true); \
            } \
        } 

#else 

#define RESEARCH() score = -search(depth - 1, ply+1, -beta, -alpha, true, true);

#endif

#ifdef PVLOG2_ENABLED

extern LogNode Pv2NodePath[64];

#define LOGPV2(depth, reversedDepth) \
if (depth == 1) \
{ \
    std::string result = getPathAsString(incrementedMaxDepth, Pv2NodePath); \
    std::cout << "Pv path found: " << result.c_str() << " score: " << score << " " << alpha << " " << beta << std::endl; \
}\

#define SAVE_PV2LOG(reversedDepth, move) \
Pv2NodePath[reversedDepth].From = move.From; \
Pv2NodePath[reversedDepth].To = move.To; \

#else

#define LOGPV2(depth, reversedDepth)
#define SAVE_PV2LOG(reversedDepth, move)

#endif

#endif
