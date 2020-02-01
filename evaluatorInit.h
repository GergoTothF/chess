#ifndef EVALUATORINIT_H
#define EVALUATORINIT_H

#include "utils.h"

static const short int SafetyTable[100] = {
     0,  0,   1,   2,   3,   5,   7,   9,  12,  15,
    18,  22,  26,  30,  35,  39,  44,  50,  56,  62,
    68,  75,  82,  85,  89,  97, 105, 113, 122, 131,
    140, 150, 169, 180, 191, 202, 213, 225, 237, 248,
    260, 272, 283, 295, 307, 319, 330, 342, 354, 366,
    377, 389, 401, 412, 424, 436, 448, 459, 471, 483,
    494, 500, 500, 500, 500, 500, 500, 500, 500, 500,
    500, 500, 500, 500, 500, 500, 500, 500, 500, 500,
    500, 500, 500, 500, 500, 500, 500, 500, 500, 500,
    500, 500, 500, 500, 500, 500, 500, 500, 500, 500
};

/*
 * 0th element: white (index_white), 1th element: black (index_black)
 */

const short int indexByColor[2][64] = {
    { square::A8, square::B8, square::C8, square::D8, square::E8, square::F8, square::G8, square::H8,
      square::A7, square::B7, square::C7, square::D7, square::E7, square::F7, square::G7, square::H7,
      square::A6, square::B6, square::C6, square::D6, square::E6, square::F6, square::G6, square::H6,
      square::A5, square::B5, square::C5, square::D5, square::E5, square::F5, square::G5, square::H5,
      square::A4, square::B4, square::C4, square::D4, square::E4, square::F4, square::G4, square::H4,
      square::A3, square::B3, square::C3, square::D3, square::E3, square::F3, square::G3, square::H3,
      square::A2, square::B2, square::C2, square::D2, square::E2, square::F2, square::G2, square::H2,
      square::A1, square::B1, square::C1, square::D1, square::E1, square::F1, square::G1, square::H1 }, 
    { square::A1, square::B1, square::C1, square::D1, square::E1, square::F1, square::G1, square::H1,
      square::A2, square::B2, square::C2, square::D2, square::E2, square::F2, square::G2, square::H2,
      square::A3, square::B3, square::C3, square::D3, square::E3, square::F3, square::G3, square::H3,
      square::A4, square::B4, square::C4, square::D4, square::E4, square::F4, square::G4, square::H4,
      square::A5, square::B5, square::C5, square::D5, square::E5, square::F5, square::G5, square::H5,
      square::A6, square::B6, square::C6, square::D6, square::E6, square::F6, square::G6, square::H6,
      square::A7, square::B7, square::C7, square::D7, square::E7, square::F7, square::G7, square::H7,
      square::A8, square::B8, square::C8, square::D8, square::E8, square::F8, square::G8, square::H8 }
};

/*
 * PAWN PCSQ (PCSQ means something like place? / piece! control? square)
 * 
 * Unlike TSCP, CPW generally doesn't want to advance its pawns. Its piece/
 * square table for pawns takes into account the following factors:
 * - small bonus for staying on the 2nd rank
 * - file-dependent component, encouraging program to capture towards the center
 * - small bonus for standing on a3/h3 
 * - penalty for d/e pawns on their initial squares 
 * - bonus for occupying the center 
 */

const short int pawn_pcsq_mg[64] = { // mg: middle game
     0,   0,   0,   0,   0,   0,   0,   0,
    -6,  -4,   1,   1,   1,   1,  -4,  -6,
    -6,  -4,   1,   2,   2,   1,  -4,  -6,
    -6,  -4,   2,   8,   8,   2,  -4,  -6,
    -6,  -4,   5,  10,  10,   5,  -4,  -6,
    -4,  -4,   1,   5,   5,   1,  -4,  -4,
    -6,  -4,   1, -24,  -24,  1,  -4,  -6,
     0,   0,   0,   0,   0,   0,   0,   0
};

const short int pawn_pcsq_eg[64] = { // eg: end game
     0,   0,   0,   0,   0,   0,   0,   0,
    -6,  -4,   1,   1,   1,   1,  -4,  -6,
    -6,  -4,   1,   2,   2,   1,  -4,  -6,
    -6,  -4,   2,   8,   8,   2,  -4,  -6,
    -6,  -4,   5,  10,  10,   5,  -4,  -6,
    -4,  -4,   1,   5,   5,   1,  -4,  -4,
    -6,  -4,   1, -24,  -24,  1,  -4,  -6,
     0,   0,   0,   0,   0,   0,   0,   0
};

/*
 * KNIGHT PCSQ 
 * 
 * - centralization bonus
 * - rim and back rank penalty, including penalty for not being developed
 */

const short int knight_pcsq_mg[64] = {
    -8,  -8,  -8,  -8,  -8,  -8,  -8,  -8,
    -8,   0,   0,   0,   0,   0,   0,  -8,
    -8,   0,   4,   6,   6,   4,   0,  -8,
    -8,   0,   6,   8,   8,   6,   0,  -8,
    -8,   0,   6,   8,   8,   6,   0,  -8,
    -8,   0,   4,   6,   6,   4,   0,  -8,
    -8,   0,   1,   2,   2,   1,   0,  -8,
   -16, -12,  -8,  -8,  -8,  -8, -12,  -16
};

const short int knight_pcsq_eg[64] = {
    -8,  -8,  -8,  -8,  -8,  -8,  -8,  -8,
    -8,   0,   0,   0,   0,   0,   0,  -8,
    -8,   0,   4,   6,   6,   4,   0,  -8,
    -8,   0,   6,   8,   8,   6,   0,  -8,
    -8,   0,   6,   8,   8,   6,   0,  -8,
    -8,   0,   4,   6,   6,   4,   0,  -8,
    -8,   0,   1,   2,   2,   1,   0,  -8,
   -16, -12,  -8,  -8,  -8,  -8, -12,  -16
};

/*
 * BISHOP PCSQ
 *
 * - centralization bonus, smaller than for knight
 * - penalty for not being developed
 * - good squares on the own half of the board
 */

const short int bishop_pcsq_mg[64] = {
    -4,  -4,  -4,  -4,  -4,  -4,  -4,  -4,
    -4,   0,   0,   0,   0,   0,   0,  -4,
    -4,   0,   2,   4,   4,   2,   0,  -4,
    -4,   0,   4,   6,   6,   4,   0,  -4,
    -4,   0,   4,   6,   6,   4,   0,  -4,
    -4,   1,   2,   4,   4,   2,   1,  -4,
    -4,   2,   1,   1,   1,   1,   2,  -4,
    -4,  -4, -12,  -4,  -4, -12,  -4,  -4
};

const short int bishop_pcsq_eg[64] = {
    -4,  -4,  -4,  -4,  -4,  -4,  -4,  -4,
    -4,   0,   0,   0,   0,   0,   0,  -4,
    -4,   0,   2,   4,   4,   2,   0,  -4,
    -4,   0,   4,   6,   6,   4,   0,  -4,
    -4,   0,   4,   6,   6,   4,   0,  -4,
    -4,   1,   2,   4,   4,   2,   1,  -4,
    -4,   2,   1,   1,   1,   1,   2,  -4,
    -4,  -4, -12,  -4,  -4, -12,  -4,  -4
};

/*
 * ROOK PCSQ
 *
 * - bonus for 7th and 8th ranks
 * - penalty for a/h columns
 * - small centralization bonus
 */

const short int rook_pcsq_mg[64] = {
     5,   5,   5,   5,   5,   5,   5,   5,
    -5,   0,   0,   0,   0,   0,   0,  -5,
    -5,   0,   0,   0,   0,   0,   0,  -5,
    -5,   0,   0,   0,   0,   0,   0,  -5,
    -5,   0,   0,   0,   0,   0,   0,  -5,
    -5,   0,   0,   0,   0,   0,   0,  -5,
    -5,   0,   0,   0,   0,   0,   0,  -5,
     0,   0,   0,   2,   2,   0,   0,   0
};

const short int rook_pcsq_eg[64] = {
     5,   5,   5,   5,   5,   5,   5,   5,
    -5,   0,   0,   0,   0,   0,   0,  -5,
    -5,   0,   0,   0,   0,   0,   0,  -5,
    -5,   0,   0,   0,   0,   0,   0,  -5,
    -5,   0,   0,   0,   0,   0,   0,  -5,
    -5,   0,   0,   0,   0,   0,   0,  -5,
    -5,   0,   0,   0,   0,   0,   0,  -5,
     0,   0,   0,   2,   2,   0,   0,   0
};

/*
 * QUEEN PCSQ
 *
 * - small bonus for centralization in the endgame
 * - penalty for staying on the 1st rank, between rooks in the midgame
 */

const short int queen_pcsq_mg[64] = {
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   1,   1,   1,   1,   0,   0,
     0,   0,   1,   2,   2,   1,   0,   0,
     0,   0,   2,   3,   3,   2,   0,   0,
     0,   0,   2,   3,   3,   2,   0,   0,
     0,   0,   1,   2,   2,   1,   0,   0,
     0,   0,   1,   1,   1,   1,   0,   0,
    -5,  -5,  -5,  -5,  -5,  -5,  -5,  -5
};

const short int queen_pcsq_eg[64] = {
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   1,   1,   1,   1,   0,   0,
     0,   0,   1,   2,   2,   1,   0,   0,
     0,   0,   2,   3,   3,   2,   0,   0,
     0,   0,   2,   3,   3,   2,   0,   0,
     0,   0,   1,   2,   2,   1,   0,   0,
     0,   0,   1,   1,   1,   1,   0,   0,
    -5,  -5,  -5,  -5,  -5,  -5,  -5,  -5
};

/*
 * KING PAWNS PCSQ
 */

const short int king_pcsq_mg[64] = {
   -40, -30, -50, -70, -70, -50, -30, -40,
   -30, -20, -40, -60, -60, -40, -20, -30,
   -20, -10, -30, -50, -50, -30, -10, -20,
   -10,   0, -20, -40, -40, -20,   0, -10,
     0,  10, -10, -30, -30, -10,  10,   0,
    10,  20,   0, -20, -20,   0,  20,  10,
    30,  40,  20,   0,   0,  20,  40,  30,
    40,  50,  30,  10,  10,  30,  50,  40
};

const short int king_pcsq_eg[64] = {
   -72, -48, -36, -24, -24, -36, -48, -72,
   -48, -24, -12,   0,   0, -12, -24, -48,
   -36, -12,   0,  12,  12,   0, -12, -36,
   -24,   0,  12,  24,  24,  12,   0, -24,
   -24,   0,  12,  24,  24,  12,   0, -24,
   -36, -12,   0,  12,  12,   0, -12, -36,
   -48, -24, -12,   0,   0, -12, -24, -48,
   -72, -48, -36, -24, -24, -36, -48, -72
};

/*
 * WEAK PAWNS PCSQ
 *
 * Current version of CPW-engine does not differentiate between isolated and
 * backward pawns, using one  generic  cathegory of  weak pawns. The penalty
 * is bigger in the center, on the assumption that weak central pawns can be
 * attacked  from many  directions. If the penalty seems too low, please note
 * that being on a semi-open file will come into equation, too.
 */

const short int weak_pawn_pcsq[64] = {
     0,   0,   0,   0,   0,   0,   0,   0,
   -10, -12, -14, -16, -16, -14, -12, -10,
   -10, -12, -14, -16, -16, -14, -12, -10,
   -10, -12, -14, -16, -16, -14, -12, -10,
   -10, -12, -14, -16, -16, -14, -12, -10,
   -10, -12, -14, -16, -16, -14, -12, -10,
   -10, -12, -14, -16, -16, -14, -12, -10,
     0,   0,   0,   0,   0,   0,   0,   0
};

const short int passed_pawn_pcsq[64] = {
     0,   0,   0,   0,   0,   0,   0,   0,
   140, 140, 140, 140, 140, 140, 140, 140,
    92,  92,  92,  92,  92,  92,  92,  92,
    56,  56,  56,  56,  56,  56,  56,  56,
    32,  32,  32,  32,  32,  32,  32,  32,
    20,  20,  20,  20,  20,  20,  20,  20,
    20,  20,  20,  20,  20,  20,  20,  20,
     0,   0,   0,   0,   0,   0,   0,   0
};

const short int protected_passed_pawn_pcsq[64] = {
     0,   0,   0,   0,   0,   0,   0,   0,
   175, 175, 175, 175, 175, 175, 175, 175,
   115, 115, 115, 115, 115, 115, 115, 115,
    70,  70,  70,  70,  70,  70,  70,  70,
    40,  40,  40,  40,  40,  40,  40,  40,
    25,  25,  25,  25,  25,  25,  25,  25,
    25,  25,  25,  25,  25,  25,  25,  25,
     0,   0,   0,   0,   0,   0,   0,   0
};

const short int BISHOP_PAIR = 30;
const short int KNIGHT_PAIR = 8;
const short int ROOK_PAIR = 16;

/* adjustements of piece value based on the number of own pawns */
const short int n_adj[9] = { -20, -16, -12, -8, -4,  0,  4,  8, 12 };
const short int r_adj[9] = { 15,  12,   9,  6,  3,  0, -3, -6, -9 };

/* trapped and blocked pieces */
const short int KING_BLOCKS_ROOK = 24;
const short int BLOCK_CENTRAL_PAWN = 24;
const short int BISHOP_TRAPPED_A7 = 150;
const short int BISHOP_TRAPPED_A6 = 50;
const short int KNIGHT_TRAPPED_A8 = 150;
const short int KNIGHT_TRAPPED_A7 = 100;

/* minor penalties */
const short int C3_KNIGHT_BLOCKS_QUEENSIDE_PAWN = 5;

/* king's defence */
const short int SHIELD_2 = 10;
const short int SHIELD_3 = 5;
const short int NO_SHIELD = 10;

/* minor bonuses */
const short int ROOK_OPEN = 10;
const short int ROOK_HALF = 5;
const short int RETURNING_BISHOP = 20;

#endif
