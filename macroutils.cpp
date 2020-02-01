#include "macroutils.h"
#include "utils.h"

#ifdef PVLOG

LogNodePath NodePath[64];

LogNodePath TempPath;

#endif 

#ifdef BOARD_CHECK

BoardIntegrity Integrity[64];

void checkBoardIntegrity(INT8U depth)
{
    if (Integrity[depth].WhiteScore != Board.PieceMaterial[0] ||
        Integrity[depth].BlackScore != Board.PieceMaterial[1])
    {
        int i = 0;
        ++i;
        LOG_TEXT("ERROR: Board integrity check failed: " << Integrity[depth].WhiteScore << " " << Integrity[depth].BlackScore << " " << Board.PieceMaterial[0] << " " << Board.PieceMaterial[1] << " " << LogNodes);
    }

    testBoardAtkTo();
}

#endif

#ifdef LOG_NEXT_MOVE

NextLogNode NextNodePath[] = { {35, 42}, {49, 42}, {28, 55}, {54, 46}, {21, 38} };

bool DepthFound[CheckedDepth];

#endif

#ifdef HISTORY

int History[2][64][64]; // side / from / to -> score (move history)

void clearHistoryTable() {
    for (int side = 0; side < 2; side++)
        for (int i = 0; i < 64; i++)
            for (int j = 0; j < 64; j++) {
                History[side][i][j] = 0;
            }
}
#endif

#ifdef PVLOG2_ENABLED

LogNode Pv2NodePath[64];

#endif

