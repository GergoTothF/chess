#ifndef MAGICBITBOARD_H
#define MAGICBITBOARD_H

#include "utils.h"

/*
* xeroshift64star Pseudo Random Number Generator
* This class is based on original code written and dedicated
* to the public domain by Sebastiano Vigna (2014)
* It has the following characteristics:
*  - outputs 64 bit number
*  - passes DieHarder and SmallCrush test batteries
*  - does not require warmup, no zeroland to escape
*  - internal state is a 64 bit long integer
*  - period is 2^64-1
*   speed: 1,60 ns / call (core i7 @3.40 GHz)
*
* For futher analysis ee:
* http://vigna.di.unimi.it/ftp/papers/xorshift.pdf
*/
class  PseudoRandomGenerator
{
private:
    INT64U Seed;

    INT64U random()
    {
        Seed ^= Seed >> 12, Seed ^= Seed << 25, Seed ^= Seed >> 27;
        return Seed * 2685821657736338717LL;
    }

public:
    PseudoRandomGenerator(INT64U seed) : Seed(seed) { }
    template <typename T> T rand() { return T(random()); }

    /*
     * Special Generator used to fast init magic numbers
     * Output values only have 1/8th of their bits set on average
     */
    template<typename T> T sparse_rand()
    {
        return T(random() & random() & random());
    }
};

struct Magic {
    INT64U Mask;
    INT64U MagicNumber;
    INT64U* Attacks;

    UINT Shift;

    // Computes the attack's index using the 'magic bitboards' approach
    UINT index(INT64U occupied) const
    {
        return UINT(((occupied & Mask) * MagicNumber) >> Shift);
    }
};

extern Magic RookMagics[64];
extern Magic BishopMagics[64];

/*
 * Computes all rook and bishop attacks at startup. Magic 
 * bitboards are used to look up attacks of sliding pieces. 
 * As a reference see chessprogramming wiki. In particular, 
 * here we used the so called 'fancy' approach 
 */
void init_magics(INT64U table[], Magic magics[], Direction directions[]);

template<piece::Type Pt>
inline INT64U attacks_bb(INT8U square, INT64U occupied) 
{
    const Magic& magic = Pt == piece::WhiteRook ? RookMagics[square] : BishopMagics[square];
    return magic.Attacks[magic.index(occupied)];
}

inline INT64U attacks_bb(piece::Type pieceType, INT8S square, INT64U occupied = 0)
{
    switch (pieceType)
    {
    case piece::BlackBishop:
    case piece::WhiteBishop:
        return attacks_bb<piece::WhiteBishop>(square, occupied);
    case piece::BlackRook:
    case piece::WhiteRook:
        return attacks_bb<piece::WhiteRook>(square, occupied);
    case piece::BlackQueen:
    case piece::WhiteQueen:
        return attacks_bb<piece::WhiteBishop>(square, occupied) | attacks_bb<piece::WhiteRook>(square, occupied);
    case piece::BlackKnight:
    case piece::WhiteKnight:
        return KnightAttacks[square];
    case piece::BlackKing:
    case piece::WhiteKing:
        return KingAttacks[square];
    case piece::BlackPawn:
        return PawnAttacksBlack[square];
    case piece::WhitePawn:
        return PawnAttacksWhite[square];
    default: return 0;
    }
}

inline INT64U attacksFrom(INT8U attackerColor, INT8S square, INT64U occupied)
{
    if (attackerColor == color::White)
    {
        return (attacks_bb(piece::BlackPawn, square) & Board.SquareByPiece[piece::WhitePawn]) |
               (attacks_bb(piece::WhiteKnight, square) & Board.SquareByPiece[piece::WhiteKnight]) |
               (attacks_bb(piece::WhiteKing, square) & Board.SquareByPiece[piece::WhiteKing]) |
               (attacks_bb(piece::WhiteBishop, square, occupied) & (Board.SquareByPiece[piece::WhiteBishop] | Board.SquareByPiece[piece::WhiteQueen])) |
               (attacks_bb(piece::WhiteRook, square, occupied) & (Board.SquareByPiece[piece::WhiteRook] | Board.SquareByPiece[piece::WhiteQueen]));
    }
    else
    {
        return (attacks_bb(piece::WhitePawn, square) & Board.SquareByPiece[piece::BlackPawn]) |
               (attacks_bb(piece::WhiteKnight, square) & Board.SquareByPiece[piece::BlackKnight]) |
               (attacks_bb(piece::WhiteKing, square) & Board.SquareByPiece[piece::BlackKing]) |
               (attacks_bb(piece::WhiteBishop, square, occupied) & (Board.SquareByPiece[piece::BlackBishop] | Board.SquareByPiece[piece::BlackQueen])) |
               (attacks_bb(piece::WhiteRook, square, occupied) & (Board.SquareByPiece[piece::BlackRook] | Board.SquareByPiece[piece::BlackQueen]));
    }
}

#endif // MAGICBITBOARD_H
