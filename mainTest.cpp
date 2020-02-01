#include "gtest/gtest.h"
#include <cstring>
#include "alphabeta.h"
#include "fenparser.h"
#include <chrono>
#include <string>  
#include <sstream> 
#include <iostream>
//#include <ctime>

using namespace std::chrono;
using namespace std::chrono_literals;

std::stringstream ss;

extern "C" {
    //int search(char * givenFen, int length) {
    const char* search2(const char * givenFen, int receivedSearhMode) {
        if (receivedSearhMode == 1)
        {
            SearchMode = searching::Odd;
        }
        else if (receivedSearhMode == 2)
        {
            SearchMode = searching::Even;
        }

#ifdef ZOBRIST
        tt_init();
#endif

        initializeHelpers();

        //fen::parse("r3k2r/pb1p1ppp/1pp4n/n2PP3/1q2B3/5N1P/PPQN1PP1/R4RK1 w kq - 0 1");

        fen::parse(givenFen);

        //delete[] inputFen;

        auto before = std::chrono::system_clock::now();
        auto before1 = std::chrono::steady_clock::now();
        //auto before3 = std::time(0);

        searchBase();

        auto elapsedTime = std::chrono::duration_cast<milliseconds>(std::chrono::system_clock::now() - before);
        auto elapsedTime2 = std::chrono::duration_cast<milliseconds>(std::chrono::steady_clock::now() - before1);
        //auto after3 = std::time(0);

        int result = (int)(elapsedTime.count());
            
        char moveStr[5];
        convertFromSquare(BestMove.From, BestMove.To, moveStr);

        //std::time_t result = std::time(nullptr);

        //std::cout << "It took me " << time_span.count() << " millisec";

        double nps = (double)(Nodes) / ((double)(elapsedTime.count()) * 1000);
        ss << moveStr << " " << BestMove.Score << " " << Nodes << " " << QuiescenceNodes << " " << EvaluatedNodes << " " << nps << " " << (int)(elapsedTime.count());

        // int resultLength = (int)(ss.str().length());
        // const char* resultStr = ss.str().c_str();

        // int i = 0;
        // while (i < resultLength) {
            // givenFen[i+1024] = resultStr[i]; // legyen givenFen + 1024 a visszateresi ertek memoriaja 
            // i++;
        // }

        //return result;
        //return (int)(elapsedTime.count());
        //return std::chrono::system_clock::now().time_since_epoch().count();
        //int result = (int)(elapsedTime.count());
        //return result;
        return ss.str().c_str();
        //return "Hello World!";
        //return BestMove.From;
        //return (int)ss.str().length();
    }
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
