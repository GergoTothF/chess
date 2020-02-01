#include "macro.h"

#ifdef EM_BUILD
#include <emscripten.h>
#endif

#include <cstring>
#include "alphabeta.h"
#include "fenparser.h"
#include <chrono>
#include <string>  
#include <sstream> 
#include <iostream>
#include <ctime>
#include "opening.h"

using namespace std::chrono;
using namespace std::chrono_literals;

std::string resultStringJs;

extern "C" {
#ifdef EM_BUILD
EMSCRIPTEN_KEEPALIVE
#endif
const char* search(char * givenFen) 
{
    static bool firstCalled = true;

    if (firstCalled)
    {
#ifdef ZOBRIST
        tt_init();
#endif

        initializeHelpers();

        firstCalled = false;
        HistoryCount = 0;
    }
    else
    {
        initializeTranspositionTable();
        HistoryCount++; // since fen parse comming
    }

    clearSearch();

    resultStringJs = checkOpening(givenFen);

    if (!resultStringJs.empty())
    {
        return resultStringJs.c_str();
    }

    //fen::parse("r3k2r/pb1p1ppp/1pp4n/n2PP3/1q2B3/5N1P/PPQN1PP1/R4RK1 w kq - 0 1");

    fen::parse(givenFen);

    //delete[] inputFen;

    auto before2 = std::chrono::system_clock::now();

    // when again calling search, maybe wasm memory is reordered -> that's why free the end
    //free(TranspositionTable);
    for (int i = 0; i < ZobristSize; ++i)
    {
        TranspositionTable[i].Hash = 0;
    }

    auto elapsedTime2 = std::chrono::duration_cast<milliseconds>(std::chrono::system_clock::now() - before2);

    auto before = std::chrono::system_clock::now();
           
    searchBase();

    auto elapsedTime = std::chrono::duration_cast<milliseconds>(std::chrono::system_clock::now() - before);

    int result = (int)(elapsedTime.count());

    char moveStr[5] = "SMME";

    if (BestMoveType == bestmove::Normal)
    {
        convertFromSquare(BestMove.From, BestMove.To, moveStr);
    }
    else if (BestMoveType == bestmove::NoMoveInCheckMate)
    {
        moveStr[0] = 'C';
        moveStr[1] = 'H';
    }

    bool isEnemyStaleMate = false;

    short int savedScore = BestMove.Score;

    if (BestMove.From != 0 && BestMove.To != 0)
    {
        mover::makeMove(BestMove);
    }

    if (BestMove.Score != INF && BestMove.Score != -INF && BestMoveType == bestmove::Normal)
    {
        isEnemyStaleMate = isInsufficientMaterial();
    }

    char enemyMate[5] = "SMEN";

    if (!isEnemyStaleMate)
    {
        enemyMate[0] = '\0';
    }

    short int score = SEARCH_CALLED(1, 0, -INF, -INF + 1, false, true);

    if (score == -INF)
    {
        enemyMate[0] = 'C';
        enemyMate[1] = 'H';
    }

    //std::time_t result = std::time(nullptr);
    //std::cout << "It took me " << time_span.count() << " millisec";

    std::stringstream ss;
    double nps = (double)(Nodes) / ((double)(elapsedTime.count()) * 1000);
    ss << moveStr << enemyMate << " " << savedScore << " " << (int)(elapsedTime2.count()) << " i44 " << HistoryCount << " " << Nodes << " " << QuiescenceNodes << " c " << EvaluatedNodes << " " << nps << " " << (int)(elapsedTime.count()) << "?" << wlog.c_str();

    resultStringJs = ss.str();
    return resultStringJs.c_str();
}

#ifdef EM_BUILD
EMSCRIPTEN_KEEPALIVE
#endif
int getLastNodeCount(char * givenFen) {
    return Nodes; 
}
}
