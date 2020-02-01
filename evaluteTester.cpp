#include "gtest/gtest.h"
#include "fenparser.h"
#include "evaluator.h"

TEST(EvaluteTester, see_simple)
{

    initializeHelpers();

    fen::parse("r3k2r/pb1p1ppp/1pp4n/n2PP3/1q2B3/5N1P/PPQN1PP1/R4RK1 w kq - 0 1");

    short int score = evaluate2();

    ASSERT_EQ(203, score);
}
