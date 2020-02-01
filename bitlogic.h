#ifndef BITLOGIC_H
#define BITLOGIC_H

#include "utils.h"

enum SearchDirection
{
    Unknown = 0,
    HigherBits,
    LowerBits,
    AllBits
};


inline INT64U getRankAttacks(INT64U occupiedBitBoard,
                             INT8U squreIndex,
                             SearchDirection searchDirection = AllBits)
{
    INT8U file = FILE(squreIndex); 
    INT8U rankx8 = squreIndex & 56; // rank * 8, rank = sor    

    /*
    * 63 mivel a két széle nem számít, azaz pld a 0000 0000 és 0000 0001 ugyanazt a maskot kell visszaadja
    */
    occupiedBitBoard = (occupiedBitBoard >> rankx8) & 2 * 63;

    /*
    * " * 4" is equivalent with " << 4"
    * not "8 * " since " >> 1" is needed after " & 2*63"
    * "& 2*63" is equivalent with "& 01111110"
    */
    INT64U attacks = FirstRankAttacks64x8[4 * occupiedBitBoard + file];

    if (searchDirection == LowerBits)
    {
        attacks &= static_cast<INT64U>(0b11111111) >> (8 - file);
    }
    else if (searchDirection == HigherBits)
    {
        attacks &= static_cast<INT64U>(0b11111111) << (file + 1);
    }

    return attacks << rankx8;
}

inline INT64U getFileAndDiagonalAttacks(INT64U occupiedBitBoard,
                                        INT8U squreIndex,
                                        const INT64U(&attacksDown)[64],
                                        const INT64U(&attacksUp)[64],
                                        SearchDirection searchDirection = AllBits)
{
    INT64U maskUp = 0;

    if (searchDirection != LowerBits)
    {
        const INT64U attacksUpBySqure = attacksUp[squreIndex];
        INT64U occupiedUp = occupiedBitBoard & attacksUpBySqure;

        if (occupiedUp == 0)
        {
            maskUp = attacksUpBySqure;
        }
        else
        {
            maskUp = LSB(occupiedUp); // get LSB
            maskUp = attacksUpBySqure & ((maskUp - 1) | maskUp);
  
        }
    }

    INT64U maskDown = 0;

    if (searchDirection != HigherBits)
    {
        const INT64U attacksDownBySqure = attacksDown[squreIndex];
        INT64U occupiedDown = occupiedBitBoard & attacksDownBySqure;

        if (occupiedDown == 0)
        {
            maskDown = attacksDownBySqure;
        }
        else
        {
            INT8U msbPosition = 0;
            BSR(msbPosition, occupiedDown); // getMsb
            maskDown = attacksDownBySqure & 0xFFFFFFFFFFFFFFFF << msbPosition;
        }
    }

    return maskDown | maskUp;
}

inline INT64U getAllPawnAttacks(INT64U occupiedBitBoard,
                                INT64U occupiedEnemyPawnBoard,
                                INT8U squreIndex,
                                const INT64U(&pawnAttacks)[64],
                                const INT64U(&enPassantPawnAttacks)[64])
{
    return (enPassantPawnAttacks[squreIndex] & occupiedEnemyPawnBoard) |
           (pawnAttacks[squreIndex] & occupiedBitBoard);
}

inline INT64U getAllAttacksTo(INT8U squreIndex)
{
    INT64U mask = POS(squreIndex);

    INT64U occupiedBitBoard = Board.AllPieces[color::White] | Board.AllPieces[color::Black];
    occupiedBitBoard &= ~mask;

    INT64U allRankAndFileAttacker = Board.SquareByPiece[piece::BlackQueen] |
                                    Board.SquareByPiece[piece::WhiteQueen] |
                                    Board.SquareByPiece[piece::BlackRook] |
                                    Board.SquareByPiece[piece::WhiteRook];

    INT64U allDiagonalAttacker = Board.SquareByPiece[piece::BlackQueen] |
                                 Board.SquareByPiece[piece::WhiteQueen] |
                                 Board.SquareByPiece[piece::BlackBishop] |
                                 Board.SquareByPiece[piece::WhiteBishop];

    INT64U allKnightAttacker = Board.SquareByPiece[piece::BlackKnight] |
                               Board.SquareByPiece[piece::WhiteKnight];

    INT64U allKingAttacker = Board.SquareByPiece[piece::BlackKing] |
                             Board.SquareByPiece[piece::WhiteKing];

    INT64U result = 0;

    if (allRankAndFileAttacker & mask)
    {
        result = getRankAttacks(occupiedBitBoard, squreIndex) | getFileAndDiagonalAttacks(occupiedBitBoard, squreIndex, FileAttacksDown, FileAttacksUp);
    }

    if (allDiagonalAttacker & mask)
    {
        result |= getFileAndDiagonalAttacks(occupiedBitBoard, squreIndex, DiagonalAttacksDown, DiagonalAttacksUp) |
                  getFileAndDiagonalAttacks(occupiedBitBoard, squreIndex, AntiDiagonalAttacksDown, AntiDiagonalAttacksUp);
    }
    else if (Board.SquareByPiece[piece::WhitePawn] & mask) // -> white pawn
    {
        result = PawnAttacksWhite[squreIndex];
        result |= EnPassantPawnAttacksWhite[squreIndex] & POS(Board.Enpassant) & Board.SquareByPiece[piece::BlackPawn];
    }
    else if (Board.SquareByPiece[piece::BlackPawn] & mask)
    {
        result = PawnAttacksBlack[squreIndex];
        result |= EnPassantPawnAttacksBlack[squreIndex] & POS(Board.Enpassant) & Board.SquareByPiece[piece::WhitePawn];
    }
    else if (allKnightAttacker & mask)
    {
        result = KnightAttacks[squreIndex];
    }
    else if (allKingAttacker & mask)
    {
        result = KingAttacks[squreIndex];
    }

    return result;
}

inline INT64U getAllSuperAttacksFrom(INT8U squreIndex)
{
    const INT64U C1 = 0b1;
    INT64U mask = C1 << squreIndex;

    INT64U occupiedBitBoard = Board.AllPieces[color::White] | Board.AllPieces[color::Black];

    occupiedBitBoard &= ~mask;

    INT64U allRankAndFileAttacker = Board.SquareByPiece[piece::BlackQueen] |
                                    Board.SquareByPiece[piece::WhiteQueen] |
                                    Board.SquareByPiece[piece::BlackRook] |
                                    Board.SquareByPiece[piece::WhiteRook];

    INT64U allDiagonalAttacker = Board.SquareByPiece[piece::BlackQueen] |
                                 Board.SquareByPiece[piece::WhiteQueen] |
                                 Board.SquareByPiece[piece::BlackBishop] |
                                 Board.SquareByPiece[piece::WhiteBishop];

    INT64U allKnightAttacker = Board.SquareByPiece[piece::BlackKnight] | Board.SquareByPiece[piece::WhiteKnight];

    INT64U allKingAttacker = Board.SquareByPiece[piece::BlackKing] | Board.SquareByPiece[piece::WhiteKing];

    INT64U maskRankAttack = getRankAttacks(occupiedBitBoard, squreIndex) & allRankAndFileAttacker;

    INT64U maskFileAttack = getFileAndDiagonalAttacks(occupiedBitBoard, squreIndex, FileAttacksDown, FileAttacksUp) & allRankAndFileAttacker;

    INT64U maskDiagonalAttack = getFileAndDiagonalAttacks(occupiedBitBoard, squreIndex, DiagonalAttacksDown, DiagonalAttacksUp) & allDiagonalAttacker;

    INT64U maskAntiDiagonalAttack = getFileAndDiagonalAttacks(occupiedBitBoard, squreIndex, AntiDiagonalAttacksDown, AntiDiagonalAttacksUp) & allDiagonalAttacker;

    INT64U maskPawnAttack = 0;
    maskPawnAttack |= PawnAttacksWhite[squreIndex] & Board.SquareByPiece[piece::BlackPawn];
    maskPawnAttack |= PawnAttacksBlack[squreIndex] & Board.SquareByPiece[piece::WhitePawn];

    if (mask & Board.AllPieces[color::White]) // -> white
    {
        if (mask & Board.SquareByPiece[piece::WhitePawn])
        {
            maskPawnAttack |= EnPassantPawnAttacksBlack[squreIndex] & Board.SquareByPiece[piece::BlackPawn];
        }
    }
    else
    {
        if (mask & Board.SquareByPiece[piece::BlackPawn])
        {
            maskPawnAttack |= EnPassantPawnAttacksWhite[squreIndex] & Board.SquareByPiece[piece::WhitePawn];
        }
    }

    INT64U maskKnight = KnightAttacks[squreIndex] & allKnightAttacker;
    INT64U maskKing = KingAttacks[squreIndex] & allKingAttacker;

    return maskRankAttack |
           maskFileAttack |
           maskDiagonalAttack |
           maskAntiDiagonalAttack |
           maskPawnAttack |
           maskKnight |
           maskKing;
}

inline void addExtraPawnMove(INT64U& attacksTo, piece::Type movingPiece, INT8U fromIndex)
{
    // eliminate bad capture attampts from pawn moving
    if (movingPiece < 3)
    {
        attacksTo &= Board.AllPieces[Board.SideToMove];
    }

    INT64U from = static_cast<INT64U>(0b1) << fromIndex;

    // pawn additional move
    if (movingPiece == piece::WhitePawn)
    {
        INT64U occupied = Board.AllPieces[color::Black] | Board.AllPieces[color::White];

        INT64U newTo8 = from << 8;
        bool isNewTo8Free = (newTo8 & occupied) == 0;

        if (from & RowMask[1])
        {
            INT64U newTo16 = from << 16;

            if (isNewTo8Free && (newTo16 & occupied) == 0)
            {
                attacksTo |= newTo16;
            }
        }

        if (isNewTo8Free)
        {
            attacksTo |= newTo8;
        }
    }
    else if (movingPiece == piece::BlackPawn)
    {
        INT64U occupied = Board.AllPieces[color::Black] | Board.AllPieces[color::White];

        INT64U newTo8 = from >> 8;
        bool isNewTo8Free = (newTo8 & occupied) == 0;

        if (from & RowMask[7])
        {
            INT64U newTo16 = from >> 16;

            if (isNewTo8Free && (newTo16 & occupied) == 0)
            {
                attacksTo |= newTo16;
            }
        }

        if (isNewTo8Free)
        {
            attacksTo |= newTo8;
        }
    }
}

#endif // BITLOGIC_H