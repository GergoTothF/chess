#ifndef OPENING_H
#define OPENING_H

#include <string>

inline std::string checkOpening(std::string fen)
{
    if (fen == "rnbqkbnr/pppppppp/8/8/3P4/8/PPP1PPPP/RNBQKBNR b KQkq - 0 1")
    {
        return "d7d5 0";
    }

    if (fen == "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1")
    {
        return "e7e5 0";
    }

    if (fen == "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 0")
    {
        return "e2e4 0";
    }

    return "";
}

#endif // OPENING_H
