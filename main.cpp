// Primitive BMP to ASCII art generator
// Reads source.bmp and outputs art.txt
// Source must be 24-bit .bmp
// http://www.dreamincode.net/code/snippet957.htm
// Modified Sept 2010 Bill Thibault to make standalone, platform-indep.
// only works on 24-bit uncompressed BMP files. some of them anyway.

#ifdef EM_BUILD
#include "pch.h"
#endif

#include <cstring>
#include "alphabeta.h"
#include "fenparser.h"
#include <chrono>
#include <string>  
#include <sstream> 
#include <iostream>
#include "evaluator.h"
#include "opening.h"
//#include <ctime>

using namespace std::chrono;
using namespace std::chrono_literals;

std::string resultString;

extern "C" {
    //int search(char * givenFen, int length) {
    const char* search2(const char * givenFen) 
    {
        static bool firstStep = true;

        if (firstStep)
        {
#ifdef ZOBRIST
            tt_init();
#endif
            HistoryCount = 0;
            initializeHelpers();
            firstStep = false;
        }
        else
        {
            initializeTranspositionTable();
            HistoryCount++; // since fen parse comming
        }

        clearSearch();

        resultString = checkOpening(givenFen);

        if (!resultString.empty())
        {
            return resultString.c_str();
        }

        //fen::parse("r3k2r/pb1p1ppp/1pp4n/n2PP3/1q2B3/5N1P/PPQN1PP1/R4RK1 w kq - 0 1");
        //fen::parse("5k1R/5P1R/5K2/8/8/8/8/8 b - - 0 3"); // CHME -> CHMEEN ?
        //fen::parse("5k2/4P3/4K3/8/8/8/8/8 w - - 0 1"); //invalid sakkban a fekete es a vilagos jon...
        // fen::parse("5k2/4P3/4K3/8/8/8/8/8 b - - 0 1"); // check mate sok lepesben vilagos adja -> utheto a kiraly?
        //fen::parse("4k3/4P3/5K2/8/8/8/8/8 w - - 0 1"); // SMME -1 insufficient material
        //fen::parse("4k3/4P3/5K2/8/8/8/8/8 b - - 0 1"); // ugyanaz, mint 5k2/4P3/4K3/8/8/8/8/8 b - - 0 1, csak kesobbi fazisbol
        //fen::parse("4k3/4P3/4K3/8/8/8/8/8 b - - 0 1"); // SMME -2 nem tud lepni a kiraly
        //fen::parse("4k3/4N3/5K2/8/8/8/8/8 w - - 0 1"); // SMME -3 insuffcient material #2 (loval is)
        //fen::parse("4k3/4Q3/8/5K2/5N2/5N2/8/8 b - - 0 1"); // SMEN kiutheto a kiralyno
        //fen::parse("rnbqkbnr/pppppppp/8/8/3P4/8/PPP1PPPP/RNBQKBNR b KQkq - 0 1"); // d4 kezdes adatbazisbol
        //fen::parse("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1"); // e4 kezdes
        fen::parse(givenFen);

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

        // check new stalemate - no need to restore values, since this is the end
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
        ss << enemyMate << moveStr << " " << savedScore << " " << (int)(elapsedTime2.count()) << " i44 " << " " << Nodes << " " << QuiescenceNodes << " " << EvaluatedNodes << " " << nps << " " << (int)(elapsedTime.count()) << "?" << wlog.c_str();

        resultString = ss.str();
        return resultString.c_str();
    }
}

int main()
{
    //std::string result = search2("r3k2r/pb1p1ppp/1pp4n/n2PP3/1q2B3/5N1P/PPQN1PP1/R4RK1 w kq - 0 1"); // test step 1->2 same result result as step2
    //std::string result = search2("r3k2r/pb1p1ppp/1pp4n/n2PP3/1q2B3/5N1P/PPQN1PP1/R4RK1 w kq - 0 1"); // test step 1->2 same result result as step2
    //std::string result2 = search2("r3k2r/pb1p1ppp/1pP4n/n3P3/1q2B3/5N1P/PPQN1PP1/R4RK1 b kq - 0 1"); // test step 1->2 same result result as step2 -> nem ugyanaz a ket score
    //std::string result2 = search2("5Q2/8/8/8/8/7k/5K2/8 w - - 0 1"); // CHEN in 2 steps
    //std::string result2 = search2("8/8/8/8/1Q6/8/5K1k/8 w - - 0 2"); // CHEN in 1 steps
    //std::string result2 = search2("r1bq1rk1/ppppbppp/1nn1p3/1B2P3/2PP4/5N2/PP3PPP/RNBQ1RK1 w  - 0 16"); // invalid fen due to castling part
    //std::string result2 = search2("r1bq1rk1/ppppbppp/1nn1p3/1B2P3/2PP4/5N2/PP3PPP/RNBQ1RK1 w - - 0 16");
    std::string result2 = search2("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");

    std::cout << result2;
}
