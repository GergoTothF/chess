#include "utils.h"
#include <stdlib.h>
#include "mover.h"

namespace fen {

    inline void clearBoard()
    {
        for (int i = 0; i < 64; ++i)
        {
            Board.PieceBySquare[i] = piece::Empty;
        }

        for (INT8U i = piece::Empty; i < piece::Count; ++i)
        {
            Board.SquareByPiece[i] = 0;
        }

        Board.Castle = 0;
        Board.Enpassant = -1;
        Board.Ply = 0;
        Board.ZobristHash = 0;
        Board.SideToMove = color::White;

        Board.AllPieces[color::White] = 0;
        Board.AllPieces[color::Black] = 0;
        Board.PieceMaterial[color::White] = 0;
        Board.PieceMaterial[color::Black] = 0;
        Board.PawnMaterial[color::White] = 0;
        Board.PawnMaterial[color::Black] = 0;

        Board.PawnControl[color::White] = 0;
        Board.PawnControl[color::Black] = 0;

        Board.MiddleGamePositionScore[color::White] = 0;
        Board.MiddleGamePositionScore[color::Black] = 0;
        Board.EndGamePositionScore[color::White] = 0;
        Board.EndGamePositionScore[color::Black] = 0;
    }

    inline INT8U convertToSqure(const char * a) {
        INT8U sq;
        sq = a[0] - 'a';
        sq += (a[1] - '1') * 8;

        return sq;
    }

    inline void parse(const char * fen)
    {
        clearBoard();

        const char * f = fen;

        char col = 0;
        char row = 7;

        do {
            switch (f[0]) {
            case 'K': mover::fillSquare(color::White, piece::WhiteKing, POS_RF(row, col));   ++col; break;
            case 'Q': mover::fillSquare(color::White, piece::WhiteQueen, POS_RF(row, col));  ++col; break;
            case 'R': mover::fillSquare(color::White, piece::WhiteRook, POS_RF(row, col));   ++col; break;
            case 'B': mover::fillSquare(color::White, piece::WhiteBishop, POS_RF(row, col)); ++col; break;
            case 'N': mover::fillSquare(color::White, piece::WhiteKnight, POS_RF(row, col)); ++col; break;
            case 'P': mover::fillSquare(color::White, piece::WhitePawn, POS_RF(row, col));   ++col; break;
            case 'k': mover::fillSquare(color::Black, piece::BlackKing, POS_RF(row, col));   ++col; break;
            case 'q': mover::fillSquare(color::Black, piece::BlackQueen, POS_RF(row, col));  ++col; break;
            case 'r': mover::fillSquare(color::Black, piece::BlackRook, POS_RF(row, col));   ++col; break;
            case 'b': mover::fillSquare(color::Black, piece::BlackBishop, POS_RF(row, col)); ++col; break;
            case 'n': mover::fillSquare(color::Black, piece::BlackKnight, POS_RF(row, col)); ++col; break;
            case 'p': mover::fillSquare(color::Black, piece::BlackPawn, POS_RF(row, col));   ++col; break;
            case '/': row--; col = 0; break;
            case '1': col += 1; break;
            case '2': col += 2; break;
            case '3': col += 3; break;
            case '4': col += 4; break;
            case '5': col += 5; break;
            case '6': col += 6; break;
            case '7': col += 7; break;
            case '8': col += 8; break;
            };

            f++;
        } while (f[0] != ' ');

        f++;

        if (f[0] == 'w')
        {
            Board.SideToMove = color::White;
            MySide = color::White;
        }
        else
        {
            Board.SideToMove = color::Black;
            MySide = color::Black;
        }

        f += 2;

        do
        {
            switch (f[0])
            {
            case 'K': Board.Castle |= castle::WhiteKingSide; break;
            case 'Q': Board.Castle |= castle::WhiteQueenSide; break;
            case 'k': Board.Castle |= castle::BlackKingSide; break;
            case 'q': Board.Castle |= castle::BlackQueenSide; break;
            }

            ++f;
        } while (f[0] != ' ');


        f++;

        if (f[0] != '-')
        {
            Board.Enpassant = convertToSqure(f);
        }

        do {
            f++;
        } while (f[0] != ' ');

        f++;

        int ply = 0;
        sscanf(f, "%d", &ply);
        Board.Ply = static_cast<INT8U>(ply);

#ifdef ZOBRIST
        calculateZobristHash();
#endif
        if (HistoryCount != 0)
        {
            if (PositionHistory[HistoryCount - 1] == Board.ZobristHash)
            {
                HistoryCount--;
            }
            else
            {
                PositionHistory[HistoryCount] = Board.ZobristHash;
            }
        }
    }

}
