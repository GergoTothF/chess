#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "utils.h"

struct EvaluatedDataType {
    short int KingAttackttCount[2];    // number of pieces attacking zone around enemy king
    short int KingAttackWeight[2]; // weight of ling attacking pieces - index to SafetyTable
    short int KingShield[2];
    short int MiddleGameMobility[2]; // mobility of all pieces at middle game
    short int EndGameMobility[2]; // mobility at end game
    short int MiddleGameTropism[2]; // tropism = all pieces distance score from their enemy score (at middle game)
    short int EndGameTropism[2]; // tropism at end game
    short int Position[2]; // poziciohoz tartozok "dijazasok". pld ket futo, early queen movin, stb...
    short int Blockages[2];
    short int AdjustMaterial[2];  // Adjusting material value for the various combinations of pieces: for exampe bishop pair bonus
    short int GamePhase; // piece material: 25 at opening, 0 at the end 
};

extern EvaluatedDataType EvaluatedData;

short int evaluate();

template <piece::Type PieceType>
void evaluateQueen(INT8U index, INT8U color);

template <piece::Type PieceType>
void evaluateBishop(INT8U index, INT8U color);

template <piece::Type PieceType>
void evaluateRook(INT8U index, INT8U color);

template <piece::Type PieceType>
void evaluateKnight(INT8U index, INT8U color);

short int evaluatePawn(INT8U index, INT8U color);

short int getTropism(INT8U square1, INT8U square2);

short int whiteKingShield();
short int blackKingShield();
void blockedPieces();

bool isInsufficientMaterial();

#endif