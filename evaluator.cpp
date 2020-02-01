#include "evaluator.h"
#include "magicbitboard.h"
#include "bitconst.h"
#include "evaluatorInit.h"

EvaluatedDataType EvaluatedData;

INT8U kingIndex[2];
INT64U negatedAllPieces[2];
INT64U occupied;

#ifdef EVALUATE_LEVEL_2
short int evaluate()
{
    ++EvaluatedNodes;

    short int result = 0;

    short int middleGameScore = Board.PieceMaterial[color::White] + Board.PawnMaterial[color::White] + Board.MiddleGamePositionScore[color::White] - 
                                Board.PieceMaterial[color::Black] - Board.PawnMaterial[color::Black] - Board.MiddleGamePositionScore[color::Black];

    short int endGameScore = Board.PieceMaterial[color::White] + Board.PawnMaterial[color::White] + Board.EndGamePositionScore[color::White] - 
                             Board.PieceMaterial[color::Black] - Board.PawnMaterial[color::Black] - Board.EndGamePositionScore[color::Black];

    EvaluatedData.GamePhase = POP_COUNT(Board.SquareByPiece[piece::WhiteKnight]) + 
                             POP_COUNT(Board.SquareByPiece[piece::WhiteBishop]) + 
                             2 * POP_COUNT(Board.SquareByPiece[piece::WhiteRook]) + 
                             4 * POP_COUNT(Board.SquareByPiece[piece::WhiteQueen]) + 
                             POP_COUNT(Board.SquareByPiece[piece::BlackKnight]) + 
                             POP_COUNT(Board.SquareByPiece[piece::BlackBishop]) + 
                             2 * POP_COUNT(Board.SquareByPiece[piece::BlackRook]) + 
                             4 * POP_COUNT(Board.SquareByPiece[piece::BlackQueen]);

    /*
     * Clear all eval data
     */

    for (int side = 0; side <= 1; side++) 
    {
        EvaluatedData.MiddleGameMobility[side] = 0;
        EvaluatedData.EndGameMobility[side] = 0;
        EvaluatedData.KingAttackttCount[side] = 0;
        EvaluatedData.KingAttackWeight[side] = 0;
        EvaluatedData.MiddleGameTropism[side] = 0;
        EvaluatedData.EndGameTropism[side] = 0;
        EvaluatedData.Position[side] = 0;
        EvaluatedData.KingShield[side] = 0;
        EvaluatedData.AdjustMaterial[side] = 0;
        EvaluatedData.Blockages[side] = 0;
        kingIndex[side] = InvalidIndex;

        negatedAllPieces[side] = ~Board.AllPieces[side];
    }

    occupied = Board.AllPieces[color::White] | Board.AllPieces[color::Black];

    BSF(kingIndex[0], Board.SquareByPiece[piece::WhiteKing]);
    BSF(kingIndex[1], Board.SquareByPiece[piece::BlackKing]);

    /*
     * add king's pawn shield score and evaluate part of piece blockage score 
     * (the rest of the latter will be done via piece eval) 
     */

    EvaluatedData.KingShield[color::White] = whiteKingShield();
    EvaluatedData.KingShield[color::Black] = blackKingShield();
    blockedPieces();
    middleGameScore += (EvaluatedData.KingShield[color::White] - EvaluatedData.KingShield[color::Black]);

    /*
     *  Adjusting material value for the various combinations of pieces.
     *  Currently it scores bishop, knight and rook pairs. The first one 
     *  gets a bonus, the latter two - a penalty. Beside that knights lose 
     *  value as pawns disappear, whereas rooks gain. 
     */

    if (POP_COUNT(Board.SquareByPiece[piece::WhiteBishop]) >1) EvaluatedData.AdjustMaterial[color::White] += BISHOP_PAIR;
    if (POP_COUNT(Board.SquareByPiece[piece::BlackBishop]) > 1) EvaluatedData.AdjustMaterial[color::Black] += BISHOP_PAIR;
    if (POP_COUNT(Board.SquareByPiece[piece::WhiteKnight]) > 1) EvaluatedData.AdjustMaterial[color::White] -= KNIGHT_PAIR;
    if (POP_COUNT(Board.SquareByPiece[piece::BlackKnight]) > 1) EvaluatedData.AdjustMaterial[color::Black] -= KNIGHT_PAIR;
    if (POP_COUNT(Board.SquareByPiece[piece::WhiteRook]) > 1) EvaluatedData.AdjustMaterial[color::White] -= ROOK_PAIR;
    if (POP_COUNT(Board.SquareByPiece[piece::BlackRook]) > 1) EvaluatedData.AdjustMaterial[color::Black] -= ROOK_PAIR;


    EvaluatedData.AdjustMaterial[color::White] += n_adj[POP_COUNT(Board.SquareByPiece[piece::WhitePawn])] * POP_COUNT(Board.SquareByPiece[piece::WhiteKnight]);
    EvaluatedData.AdjustMaterial[color::Black] += n_adj[POP_COUNT(Board.SquareByPiece[piece::BlackPawn])] * POP_COUNT(Board.SquareByPiece[piece::BlackKnight]);
    EvaluatedData.AdjustMaterial[color::White] += r_adj[POP_COUNT(Board.SquareByPiece[piece::WhitePawn])] * POP_COUNT(Board.SquareByPiece[piece::WhiteRook]);
    EvaluatedData.AdjustMaterial[color::Black] += r_adj[POP_COUNT(Board.SquareByPiece[piece::BlackPawn])] * POP_COUNT(Board.SquareByPiece[piece::BlackRook]);

    for (INT8U index = 0; index < 64; index++)
    {
        switch (Board.PieceBySquare[index])
        {
        case piece::BlackBishop:
            evaluateBishop<piece::BlackBishop>(index, color::Black);
            break;
        case piece::WhiteBishop:
            evaluateBishop<piece::WhiteBishop>(index, color::White);
            break;
        case piece::BlackRook:
            evaluateRook<piece::BlackRook>(index, color::Black);
            break;
        case piece::WhiteRook:
            evaluateRook<piece::WhiteRook>(index, color::White);
            break;
        case piece::BlackQueen:
            evaluateQueen<piece::BlackQueen>(index, color::Black);
            break;
        case piece::WhiteQueen:
            evaluateQueen<piece::WhiteQueen>(index, color::White);
            break;
        case piece::BlackKnight:
            evaluateKnight<piece::BlackKnight>(index, color::Black);
            break;
        case piece::WhiteKnight:
            evaluateKnight<piece::WhiteKnight>(index, color::White);
            break;
        case piece::BlackKing:
            break;
        case piece::WhiteKing:
            break;
        case piece::BlackPawn:
            result -= evaluatePawn(index, color::Black);
            break;
        case piece::WhitePawn:
            result += evaluatePawn(index, color::White); // directly added to the score
            break;
        default:
            break;
        }
    }

    /*
     * Merge middle game and end game score. We interpolate between these two
     * values, using a gamePhase value, based on remaing piece material on
     * both sides. With less pieces, end game score becomes more influential
     * -> sulyozzuk a middle game and end game score-okat a gamephase tukreben
     */

    middleGameScore += (EvaluatedData.MiddleGameMobility[color::White] - EvaluatedData.MiddleGameMobility[color::Black]);
    endGameScore += (EvaluatedData.EndGameMobility[color::White] - EvaluatedData.EndGameMobility[color::Black]);
    middleGameScore += (EvaluatedData.MiddleGameTropism[color::White] - EvaluatedData.MiddleGameTropism[color::Black]);
    endGameScore += (EvaluatedData.EndGameTropism[color::White] - EvaluatedData.EndGameTropism[color::Black]);
    
    if (EvaluatedData.GamePhase > 24) EvaluatedData.GamePhase = 24;
    int middleGameWeight = EvaluatedData.GamePhase;
    int endGameWeight = 24 - middleGameWeight;
    result += ( (middleGameScore * middleGameWeight) + (endGameScore * endGameWeight) ) / 24;

    /*
     * Add phase-independent score components.
     */

    result += (EvaluatedData.Position[color::White] - EvaluatedData.Position[color::Black]);
    result += (EvaluatedData.Blockages[color::White] - EvaluatedData.Blockages[color::Black]);
    result += (EvaluatedData.AdjustMaterial[color::White] - EvaluatedData.AdjustMaterial[color::Black]);

    /*
     * Merge king attack score. We don't apply this value if there are less 
     *  than two attackers or if the attacker has no queen
     */

    if (EvaluatedData.KingAttackttCount[color::White] < 2 || POP_COUNT(Board.SquareByPiece[piece::WhiteQueen]) == 0) EvaluatedData.KingAttackWeight[color::White] = 0;
    if (EvaluatedData.KingAttackttCount[color::Black] < 2 || POP_COUNT(Board.SquareByPiece[piece::BlackQueen]) == 0) EvaluatedData.KingAttackWeight[color::Black] = 0;
    result += SafetyTable[EvaluatedData.KingAttackWeight[color::White]];
    result -= SafetyTable[EvaluatedData.KingAttackWeight[color::Black]];

    color::Type stronger = color::Black;
    color::Type weaker = color::Black;

    /*
     * corrigate stalemate and drawish situations
     */
    if (result > 0) {
        stronger = color::White;
        weaker = color::Black;
    }
    else {
        stronger = color::Black;
        weaker = color::White;
    }

    if (Board.PawnMaterial[stronger] == 0) {

        if (Board.PieceMaterial[stronger] < 400)
        {
            return 0;
        }

        if (Board.PawnMaterial[weaker] == 0 && Board.PieceMaterial[stronger] == (2 * EvaluationData.PieceValue[piece::WhiteKnight]))
        {
            return 0;
        }

        if (Board.PawnMaterial[stronger] == EvaluationData.PieceValue[piece::WhiteRook] &&
            Board.PieceMaterial[weaker] == EvaluationData.PieceValue[piece::WhiteBishop])
        {
            result /= 2;
        }

        if (Board.PawnMaterial[stronger] == (EvaluationData.PieceValue[piece::WhiteRook] + EvaluationData.PieceValue[piece::WhiteBishop]) &&
            Board.PieceMaterial[weaker] == EvaluationData.PieceValue[piece::WhiteRook])
        {
            result /= 2;
        }

        if (Board.PawnMaterial[stronger] == (EvaluationData.PieceValue[piece::WhiteRook] + EvaluationData.PieceValue[piece::WhiteKnight]) &&
            Board.PieceMaterial[weaker] == EvaluationData.PieceValue[piece::WhiteRook])
        {
            result /= 2;
        }
    }

    /*
     * igazitsuk a tamado felhez a score-unkat
     *
     * white lepese utan azert negativ, mert
     * pld vegyuk a depth = 4-et white-black-white-black lep, tehat a side-to-move white lesz
     * tehat nem kell negalni. Tegyuk fel, hogy nincs quiescence (ami ebbol a szempontbol nem oszt szoroz).
     * A black kiertekelesekor a score megis "-1-szerezodik" oka, hogy depth = 0 -nal visszeterunk,
     * amit a depth = 1 fog "-1"-szerezni
     *
     * Rovidebben: ket "-1"-szerezes van:
     * egyik score = -search()
     * a masik itt az evaluate-be
     */

    if (Board.SideToMove == color::Black)
    {
        result = -result;
    }

    return result;
}
#else // EVALUATE LEVEL 1
short int evaluate()
{
    ++EvaluatedNodes;

    short int result = Board.PieceMaterial[color::White] + Board.PawnMaterial[color::White] - Board.PieceMaterial[color::Black] - Board.PawnMaterial[color::Black];

    /*
     * igazitsuk a tamado felhez a score-unkat
     *
     * white lepese utan azert negativ, mert
     * pld vegyuk a depth = 4-et white-black-white-black lep, tehat a side-to-move white lesz
     * tehat nem kell negalni. Tegyuk fel, hogy nincs quiescence (ami ebbol a szempontbol nem oszt szoroz).
     * A black kiertekelesekor a score megis "-1-szerezodik" oka, hogy depth = 0 -nal visszeterunk,
     * amit a depth = 1 fog "-1"-szerezni
     *
     * Rovidebben: ket "-1"-szerezes van:
     * egyik score = -search()
     * a masik itt az evaluate-be
     */

    if (Board.SideToMove == color::Black)
    {
        result = -result;
    }

    return result;
}
#endif

bool isInsufficientMaterial()
{
    if (Board.PawnMaterial[color::White] == 0 && Board.PawnMaterial[color::Black] == 0 &&
        Board.PieceMaterial[color::White] < 400 && Board.PieceMaterial[color::Black] < 400)
    {
        return true;
    }

    if ((Board.PawnMaterial[color::White] == 0 && Board.PieceMaterial[color::Black] == (2 * EvaluationData.PieceValue[piece::WhiteKnight])) ||
        (Board.PawnMaterial[color::Black] == 0 && Board.PieceMaterial[color::White] == (2 * EvaluationData.PieceValue[piece::WhiteKnight])))
    {
        return true;
    }

    return false;
}

short int whiteKingShield()
{
    short int result = 0;

    /* king on the kingside */
    if (FILE(kingIndex[color::White]) > FILE_E) 
    {
        if (Board.PieceBySquare[square::F2] == piece::WhitePawn)
        {
            result += SHIELD_2;
        }
        else if (Board.PieceBySquare[square::F3] == piece::WhitePawn)
        {
            result += SHIELD_3;
        }

        if (Board.PieceBySquare[square::G2] == piece::WhitePawn)
        {
            result += SHIELD_2;
        }
        else if (Board.PieceBySquare[square::G3] == piece::WhitePawn)
        {
            result += SHIELD_3;
        }

        if (Board.PieceBySquare[square::H2] == piece::WhitePawn)
        {
            result += SHIELD_2;
        }
        else if (Board.PieceBySquare[square::H3] == piece::WhitePawn)
        {
            result += SHIELD_3;
        }
    }

    /* king on the queenside */
    else if (FILE(kingIndex[color::White]) < FILE_D) 
    {
        if (Board.PieceBySquare[square::A2] == piece::WhitePawn)
        {
            result += SHIELD_2;
        }
        else if (Board.PieceBySquare[square::A3] == piece::WhitePawn)
        {
            result += SHIELD_3;
        }

        if (Board.PieceBySquare[square::B2] == piece::WhitePawn)
        {
            result += SHIELD_2;
        }
        else if (Board.PieceBySquare[square::B3] == piece::WhitePawn)
        {
            result += SHIELD_3;
        }

        if (Board.PieceBySquare[square::C2] == piece::WhitePawn)
        {
            result += SHIELD_2;
        }
        else if (Board.PieceBySquare[square::C3] == piece::WhitePawn)
        {
            result += SHIELD_3;
        }
    }

    return result;
}

short int blackKingShield()
{
    short int result = 0;

    /* king on the kingside */
    if (FILE(kingIndex[color::Black]) > FILE_E)
    {
        if (Board.PieceBySquare[square::F7] == piece::BlackPawn)
        {
            result += SHIELD_2;
        }
        else if (Board.PieceBySquare[square::F6] == piece::BlackPawn)
        {
            result += SHIELD_3;
        }

        if (Board.PieceBySquare[square::G7] == piece::BlackPawn)
        {
            result += SHIELD_2;
        }
        else if (Board.PieceBySquare[square::G6] == piece::BlackPawn)
        {
            result += SHIELD_3;
        }

        if (Board.PieceBySquare[square::H7] == piece::BlackPawn)
        {
            result += SHIELD_2;
        }
        else if (Board.PieceBySquare[square::H6] == piece::BlackPawn)
        {
            result += SHIELD_3;
        }
    }

    /* king on the queenside */
    else if (FILE(kingIndex[color::Black]) < FILE_D)
    {
        if (Board.PieceBySquare[square::A7] == piece::BlackPawn)
        {
            result += SHIELD_2;
        }
        else if (Board.PieceBySquare[square::A6] == piece::BlackPawn)
        {
            result += SHIELD_3;
        }

        if (Board.PieceBySquare[square::B7] == piece::BlackPawn)
        {
            result += SHIELD_2;
        }
        else if (Board.PieceBySquare[square::B6] == piece::BlackPawn)
        {
            result += SHIELD_3;
        }

        if (Board.PieceBySquare[square::C7] == piece::BlackPawn)
        {
            result += SHIELD_2;
        }
        else if (Board.PieceBySquare[square::C6] == piece::BlackPawn)
        {
            result += SHIELD_3;
        }
    }

    return result;
}

void blockedPieces() {

    // central pawn blocked, bishop hard to develop
    if (Board.PieceBySquare[square::C1] == piece::WhiteBishop &&
        Board.PieceBySquare[square::D2] == piece::WhitePawn &&
        Board.PieceBySquare[square::D3] != piece::Empty)
    {
        EvaluatedData.Blockages[color::White] -= BLOCK_CENTRAL_PAWN;
    }

    if (Board.PieceBySquare[square::F1] == piece::WhiteBishop &&
        Board.PieceBySquare[square::E2] == piece::WhitePawn &&
        Board.PieceBySquare[square::E3] != piece::Empty)
    {
        EvaluatedData.Blockages[color::White] -= BLOCK_CENTRAL_PAWN;
    }

    if (Board.PieceBySquare[square::C8] == piece::BlackBishop &&
        Board.PieceBySquare[square::D7] == piece::BlackPawn &&
        Board.PieceBySquare[square::D6] != piece::Empty)
    {
        EvaluatedData.Blockages[color::Black] -= BLOCK_CENTRAL_PAWN;
    }

    if (Board.PieceBySquare[square::F8] == piece::BlackBishop &&
        Board.PieceBySquare[square::E7] == piece::BlackPawn &&
        Board.PieceBySquare[square::E6] != piece::Empty)
    {
        EvaluatedData.Blockages[color::Black] -= BLOCK_CENTRAL_PAWN;
    }

    // trapped knight
    if (Board.PieceBySquare[square::A8] == piece::WhiteKnight &&
        (Board.PieceBySquare[square::A7] == piece::BlackPawn || Board.PieceBySquare[square::C7] == piece::BlackPawn))
    {
        EvaluatedData.Blockages[color::White] -= KNIGHT_TRAPPED_A8;
    }

    if (Board.PieceBySquare[square::H8] == piece::WhiteKnight &&
        (Board.PieceBySquare[square::H7] == piece::BlackPawn || Board.PieceBySquare[square::F7] == piece::BlackPawn))
    {
        EvaluatedData.Blockages[color::White] -= KNIGHT_TRAPPED_A8;
    }

    if (Board.PieceBySquare[square::A7] == piece::WhiteKnight &&
        Board.PieceBySquare[square::A6] == piece::BlackPawn &&
        Board.PieceBySquare[square::B7] == piece::BlackPawn)
    {
        EvaluatedData.Blockages[color::White] -= KNIGHT_TRAPPED_A7;
    }

    if (Board.PieceBySquare[square::H7] == piece::WhiteKnight &&
        Board.PieceBySquare[square::H6] == piece::BlackPawn &&
        Board.PieceBySquare[square::G7] == piece::BlackPawn)
    {
        EvaluatedData.Blockages[color::White] -= KNIGHT_TRAPPED_A7;
    }

    if (Board.PieceBySquare[square::A1] == piece::BlackKnight &&
        (Board.PieceBySquare[square::A2] == piece::WhitePawn || Board.PieceBySquare[square::C2] == piece::WhitePawn))
    {
        EvaluatedData.Blockages[color::Black] -= KNIGHT_TRAPPED_A8;
    }

    if (Board.PieceBySquare[square::H1] == piece::BlackKnight &&
        (Board.PieceBySquare[square::H2] == piece::WhitePawn || Board.PieceBySquare[square::F2] == piece::WhitePawn))
    {
        EvaluatedData.Blockages[color::Black] -= KNIGHT_TRAPPED_A8;
    }

    if (Board.PieceBySquare[square::A2] == piece::BlackKnight &&
        Board.PieceBySquare[square::A3] == piece::WhitePawn &&
        Board.PieceBySquare[square::B2] == piece::WhitePawn)
    {
        EvaluatedData.Blockages[color::Black] -= KNIGHT_TRAPPED_A7;
    }

    if (Board.PieceBySquare[square::H2] == piece::BlackKnight &&
        Board.PieceBySquare[square::H3] == piece::WhitePawn &&
        Board.PieceBySquare[square::G2] == piece::WhitePawn)
    {
        EvaluatedData.Blockages[color::Black] -= KNIGHT_TRAPPED_A7;
    }

    // knight blocking queenside pawns
    if (Board.PieceBySquare[square::C3] == piece::WhiteKnight &&
        Board.PieceBySquare[square::C2] == piece::WhitePawn &&
        Board.PieceBySquare[square::D4] == piece::WhitePawn &&
        Board.PieceBySquare[square::E4] != piece::WhitePawn)
    {
        EvaluatedData.Blockages[color::White] -= C3_KNIGHT_BLOCKS_QUEENSIDE_PAWN;
    }

    if (Board.PieceBySquare[square::C6] == piece::BlackKnight &&
        Board.PieceBySquare[square::C7] == piece::BlackPawn &&
        Board.PieceBySquare[square::D5] == piece::BlackPawn &&
        Board.PieceBySquare[square::E5] != piece::BlackPawn)
    {
        EvaluatedData.Blockages[color::Black] -= C3_KNIGHT_BLOCKS_QUEENSIDE_PAWN;
    }

    // trapped bishop
    if (Board.PieceBySquare[square::A7] == piece::WhiteBishop &&
        Board.PieceBySquare[square::B6] == piece::BlackPawn)
    {
        EvaluatedData.Blockages[color::White] -= BISHOP_TRAPPED_A7;
    }

    if (Board.PieceBySquare[square::H7] == piece::WhiteBishop &&
        Board.PieceBySquare[square::G6] == piece::BlackPawn)
    {
        EvaluatedData.Blockages[color::White] -= BISHOP_TRAPPED_A7;
    }

    if (Board.PieceBySquare[square::B8] == piece::WhiteBishop &&
        Board.PieceBySquare[square::C7] == piece::BlackPawn)
    {
        EvaluatedData.Blockages[color::White] -= BISHOP_TRAPPED_A7;
    }

    if (Board.PieceBySquare[square::G8] == piece::WhiteBishop &&
        Board.PieceBySquare[square::F7] == piece::BlackPawn)
    {
        EvaluatedData.Blockages[color::White] -= BISHOP_TRAPPED_A7;
    }

    if (Board.PieceBySquare[square::A6] == piece::WhiteBishop &&
        Board.PieceBySquare[square::B5] == piece::BlackPawn)
    {
        EvaluatedData.Blockages[color::White] -= BISHOP_TRAPPED_A6;
    }

    if (Board.PieceBySquare[square::H6] == piece::WhiteBishop &&
        Board.PieceBySquare[square::G5] == piece::BlackPawn)
    {
        EvaluatedData.Blockages[color::White] -= BISHOP_TRAPPED_A6;
    }

    if (Board.PieceBySquare[square::A2] == piece::BlackBishop &&
        Board.PieceBySquare[square::B3] == piece::WhitePawn)
    {
        EvaluatedData.Blockages[color::Black] -= BISHOP_TRAPPED_A7;
    }

    if (Board.PieceBySquare[square::H2] == piece::BlackBishop &&
        Board.PieceBySquare[square::G3] == piece::WhitePawn)
    {
        EvaluatedData.Blockages[color::Black] -= BISHOP_TRAPPED_A7;
    }

    if (Board.PieceBySquare[square::B1] == piece::BlackBishop &&
        Board.PieceBySquare[square::C2] == piece::WhitePawn)
    {
        EvaluatedData.Blockages[color::Black] -= BISHOP_TRAPPED_A7;
    }

    if (Board.PieceBySquare[square::G1] == piece::BlackBishop &&
        Board.PieceBySquare[square::F2] == piece::WhitePawn)
    {
        EvaluatedData.Blockages[color::Black] -= BISHOP_TRAPPED_A7;
    }

    if (Board.PieceBySquare[square::A3] == piece::BlackBishop &&
        Board.PieceBySquare[square::B4] == piece::WhitePawn)
    {
        EvaluatedData.Blockages[color::Black] -= BISHOP_TRAPPED_A6;
    }

    if (Board.PieceBySquare[square::H3] == piece::BlackBishop &&
        Board.PieceBySquare[square::G4] == piece::WhitePawn)
    {
        EvaluatedData.Blockages[color::Black] -= BISHOP_TRAPPED_A6;
    }

    // bishop on initial sqare supporting castled king
    if (Board.PieceBySquare[square::F1] == piece::WhiteBishop &&
        Board.PieceBySquare[square::G1] == piece::WhiteKing)
    {
        EvaluatedData.Position[color::White] += RETURNING_BISHOP;
    }

    if (Board.PieceBySquare[square::C1] == piece::WhiteBishop &&
        Board.PieceBySquare[square::B1] == piece::WhiteKing)
    {
        EvaluatedData.Position[color::White] += RETURNING_BISHOP;
    }

    if (Board.PieceBySquare[square::F8] == piece::BlackBishop &&
        Board.PieceBySquare[square::G8] == piece::BlackKing)
    {
        EvaluatedData.Position[color::Black] += RETURNING_BISHOP;
    }

    if (Board.PieceBySquare[square::C8] == piece::BlackBishop &&
        Board.PieceBySquare[square::B8] == piece::BlackKing)
    {
        EvaluatedData.Position[color::Black] += RETURNING_BISHOP;
    }

    // uncastled king blocking own rook
    if ((Board.PieceBySquare[square::F1] == piece::WhiteKing || Board.PieceBySquare[square::G1] == piece::WhiteKing) &&
        (Board.PieceBySquare[square::H1] == piece::WhiteRook || Board.PieceBySquare[square::G1] == piece::WhiteRook))
    {
        EvaluatedData.Blockages[color::White] -= KING_BLOCKS_ROOK;
    }

    if ((Board.PieceBySquare[square::C1] == piece::WhiteKing || Board.PieceBySquare[square::B1] == piece::WhiteKing) &&
        (Board.PieceBySquare[square::A1] == piece::WhiteRook || Board.PieceBySquare[square::B1] == piece::WhiteRook))
    {
        EvaluatedData.Blockages[color::White] -= KING_BLOCKS_ROOK;
    }

    if ((Board.PieceBySquare[square::F8] == piece::BlackKing || Board.PieceBySquare[square::G8] == piece::BlackKing) &&
        (Board.PieceBySquare[square::H8] == piece::BlackRook || Board.PieceBySquare[square::G8] == piece::BlackRook))
    {
        EvaluatedData.Blockages[color::Black] -= KING_BLOCKS_ROOK;
    }

    if ((Board.PieceBySquare[square::C8] == piece::BlackKing || Board.PieceBySquare[square::B8] == piece::BlackKing) &&
        (Board.PieceBySquare[square::A8] == piece::BlackRook || Board.PieceBySquare[square::B8] == piece::BlackRook))
    {
        EvaluatedData.Blockages[color::Black] -= KING_BLOCKS_ROOK;
    }
}

template <piece::Type PieceType>
void evaluateQueen(INT8U index, INT8U color)
{
    /*
     * Queen should not be developed too early
     */
    if (color == color::White)
    {
        if (RANK(index) > 0)
        {
            if (Board.SquareByPiece[piece::WhiteKnight] & POS(square::B1)) EvaluatedData.Position[color] -= 10;
            if (Board.SquareByPiece[piece::WhiteBishop] & POS(square::C1)) EvaluatedData.Position[color] -= 10;
            if (Board.SquareByPiece[piece::WhiteBishop] & POS(square::F1)) EvaluatedData.Position[color] -= 10;
            if (Board.SquareByPiece[piece::WhiteKnight] & POS(square::G1)) EvaluatedData.Position[color] -= 10;
        }
    }
    else
    {
        if (RANK(index) < 7)
        {
            if (Board.SquareByPiece[piece::BlackKnight] & POS(square::B8)) EvaluatedData.Position[color] -= 10;
            if (Board.SquareByPiece[piece::BlackBishop] & POS(square::C8)) EvaluatedData.Position[color] -= 10;
            if (Board.SquareByPiece[piece::BlackBishop] & POS(square::F8)) EvaluatedData.Position[color] -= 10;
            if (Board.SquareByPiece[piece::BlackKnight] & POS(square::G8)) EvaluatedData.Position[color] -= 10;
        }
    }

    INT64U attacksTo = attacks_bb(PieceType, index, occupied) & negatedAllPieces[color];
   
    INT8U attackCount = POP_COUNT(attacksTo & KingNeighbours[kingIndex[!color]]);

    if (attackCount != 0)
    {
        EvaluatedData.KingAttackttCount[color]++;
        EvaluatedData.KingAttackWeight[color] += 4 * attackCount;
    }

    short int mobility = POP_COUNT(attacksTo);
    EvaluatedData.MiddleGameMobility[color] += 1 * (mobility - 14);
    EvaluatedData.EndGameMobility[color] += 2 * (mobility - 14);

    short int tropism = getTropism(index, kingIndex[!color]);
    EvaluatedData.MiddleGameTropism[color] += 2 * tropism;
    EvaluatedData.EndGameTropism[color] += 4 * tropism;
}

template <piece::Type PieceType>
void evaluateBishop(INT8U index, INT8U color)
{
    INT64U attacksTo = attacks_bb(PieceType, index, occupied) & negatedAllPieces[color];

    INT8U attackCount =  POP_COUNT(attacksTo & KingNeighbours[kingIndex[!color]]);

    if (attackCount != 0)
    {
        EvaluatedData.KingAttackttCount[color]++;
        EvaluatedData.KingAttackWeight[color] += 2 * attackCount;
    }

    short int mobility = POP_COUNT(attacksTo & ~Board.PawnControl[!color]);
    EvaluatedData.MiddleGameMobility[color] += 3 * (mobility - 7);
    EvaluatedData.EndGameMobility[color] += 3 * (mobility - 7);

    short int tropism = getTropism(index, kingIndex[!color]);
    EvaluatedData.MiddleGameTropism[color] += 2 * tropism;
    EvaluatedData.EndGameTropism[color] += 1 * tropism;
}

template <piece::Type PieceType>
void evaluateRook(INT8U index, INT8U color)
{
    /*
     * Bonus for rook on the seventh rank. It is applied when there are pawns *
     * to attack along that rank or if enemy king is cut off on 8th rank      *
     */

    if ((!color && RANK(index) == 6 && ((Board.SquareByPiece[piece::BlackPawn] & Rank7) || RANK(kingIndex[color]) == 7)) ||
        (color && RANK(index) == 1 && ((Board.SquareByPiece[piece::WhitePawn] & Rank2) || RANK(kingIndex[!color]) == 0)))
    {
        EvaluatedData.MiddleGameMobility[color] += 20;
        EvaluatedData.EndGameMobility[color] += 30;
    }

    /*
     * Bonus for open and half-open files is merged with mobility score.
     * Bonus for open files targetting enemy king (ugyanazon az oszlopon vagy kozvetlen melette)
     * is added to attWeight[] 
     */

    if ((Board.SquareByPiece[color ? piece::BlackPawn : piece::WhitePawn] & FileMask[FILE(index)]) == 0) 
    {
        if ((Board.SquareByPiece[color ? piece::WhitePawn : piece::BlackPawn] & FileMask[FILE(index)]) == 0) // fully open file
        {
            EvaluatedData.MiddleGameMobility[color] += ROOK_OPEN;
            EvaluatedData.EndGameMobility[color] += ROOK_OPEN;
            if (abs(FILE(index) - FILE(kingIndex[!color])) < 2)
            {
                EvaluatedData.KingAttackWeight[color] += 1;
            }
        }
        else // half open file
        {
            EvaluatedData.MiddleGameMobility[color] += ROOK_HALF;
            EvaluatedData.EndGameMobility[color] += ROOK_HALF;
            if (abs(FILE(index) - FILE(kingIndex[!color])) < 2)
            {
                EvaluatedData.KingAttackWeight[color] += 2;
            }
        }
    }

    INT64U attacksTo = attacks_bb(PieceType, index, occupied) & negatedAllPieces[color];

    INT8U attackCount = POP_COUNT(attacksTo & KingNeighbours[kingIndex[!color]]);

    if (attackCount != 0)
    {
        EvaluatedData.KingAttackttCount[color]++;
        EvaluatedData.KingAttackWeight[color] += 3 * attackCount;
    }

    short int mobility = POP_COUNT(attacksTo & ~Board.PawnControl[!color]);
    EvaluatedData.MiddleGameMobility[color] += 2 * (mobility - 7);
    EvaluatedData.EndGameMobility[color] += 4 * (mobility - 7);

    short int tropism = getTropism(index, kingIndex[!color]);
    EvaluatedData.MiddleGameTropism[color] += 2 * tropism;
    EvaluatedData.EndGameTropism[color] += 1 * tropism;
}

template <piece::Type PieceType>
void evaluateKnight(INT8U index, INT8U color)
{
    INT64U attacksTo = attacks_bb(PieceType, index, occupied) & negatedAllPieces[color];

    INT8U attackCount = POP_COUNT(attacksTo & KingNeighbours[kingIndex[!color]]);

    if (attackCount != 0)
    {
        EvaluatedData.KingAttackttCount[color]++;
        EvaluatedData.KingAttackWeight[color] += 2 * attackCount;
    }

    short int mobility = POP_COUNT(attacksTo & ~Board.PawnControl[!color]);
    EvaluatedData.MiddleGameMobility[color] += 4 * (mobility - 4);
    EvaluatedData.EndGameMobility[color] += 4 * (mobility - 4);

    short int tropism = getTropism(index, kingIndex[!color]);
    EvaluatedData.MiddleGameTropism[color] += 3 * tropism;
    EvaluatedData.EndGameTropism[color] += 3 * tropism;
}

short int evaluatePawn(INT8U index, INT8U color)
{
    short int result = 0;
    bool flagIsPassed = true; // we will be trying to disprove that
    bool flagIsWeak = true;   // we will be trying to disprove that
    bool flagIsOpposed = false;

    /*
     * check opposite pawn and doubled pawn, and ...
     */
    if (color == color::White)
    {
        /*
         * If a pawn is attacked by an enemy pawn it isn't passed 
         * (not sure this is the best decision)
         * ez arra vonatkozott, ha ramutat tamadas,
         * mig az ele mutato tamadasnal sem lesz passed a pawn (ott nem volt ez a bizonytalansag)
         */
        if (RankCollectionMaskInverse[RANK(index)] & Board.PawnControl[!color] & FileMask[FILE(index)])
        {
            flagIsPassed = false;
        }

        if (RankCollectionMaskInverse[RANK(index)] & FileMask[FILE(index)] & Board.SquareByPiece[piece::BlackPawn])
        {
            flagIsPassed = false;
            flagIsOpposed = true;
        }

        if (RankCollectionMaskInverse[RANK(index + NORTH)] & FileMask[FILE(index)] & Board.SquareByPiece[piece::WhitePawn])
        {
            flagIsPassed = false;
            result -= 20;
        }

        /*
         * checking whether pawn has support
         * lenyegeben azt nezi hogy ele, vagy moge nem-e mutat vedelem sajat pawn-jatol
         * so that a pawn in a duo (tehat ha egymas mellett allnak?) will not be considered weak
         */
        if (RankCollectionMask[RANK(index + NORTH)] & Board.PawnControl[color] & FileMask[FILE(index)])
        {
            flagIsWeak = 0;
        }
    }
    else
    {
        /*
         * If a pawn is attacked by an enemy pawn it isn't passed
         * (not sure this is the best decision)
         * ez arra vonatkozott, ha ramutat tamadas,
         * mig az ele mutato tamadasnal sem lesz passed a pawn (ott nem volt ez a bizonytalansag)
         */
        if (RankCollectionMask[RANK(index)] & Board.PawnControl[!color] & FileMask[FILE(index)])
        {
            flagIsPassed = false;
        }

        if (RankCollectionMask[RANK(index)] & Board.SquareByPiece[piece::WhitePawn] & FileMask[FILE(index)])
        {
            flagIsPassed = false;
            flagIsOpposed = true;
        }

        if (RankCollectionMask[RANK(index + SOUTH)] & FileMask[FILE(index)] & Board.SquareByPiece[piece::BlackPawn])
        {
            flagIsPassed = false;
            result -= 20;
        }

        /*
         * checking whether pawn has support
         * lenyegeben azt nezi hogy ele, vagy moge nem-e mutat vedelem sajat pawn-jatol
         * so that a pawn in a duo (tehat ha egymas mellett allnak?) will not be considered weak
         */
        if (RankCollectionMaskInverse[RANK(index + SOUTH)] & Board.PawnControl[color] & FileMask[FILE(index)])
        {
            flagIsWeak = false;
        }
    }

    /*
    * Evaluate passed pawns, scoring them higher if they are protected
    * or if their advance is supported by friendly pawns
    */
    if ( flagIsPassed ) 
    {
        // is pawn supported by another pawn
        if ((color && FILE(index) > 0 && (Board.PieceBySquare[index + WEST] == piece::BlackPawn || Board.PieceBySquare[index + NW] == piece::BlackPawn)) ||
            (color && FILE(index) < 7 && (Board.PieceBySquare[index + EAST] == piece::BlackPawn || Board.PieceBySquare[index + NE] == piece::BlackPawn)) ||
            (!color && FILE(index) > 0 && (Board.PieceBySquare[index + WEST] == piece::WhitePawn || Board.PieceBySquare[index + SW] == piece::WhitePawn)) ||
            (!color && FILE(index) < 7 && (Board.PieceBySquare[index + EAST] == piece::WhitePawn || Board.PieceBySquare[index + SE] == piece::WhitePawn)) )
        {
            result += protected_passed_pawn_pcsq[indexByColor[color][index]];
        }
        else
        {
            result += passed_pawn_pcsq[indexByColor[color][index]];
        }
    }

    /*
    *  Evaluate weak pawns, increasing the penalty if they are situated
    *  on a half-open file
    */
    if ( flagIsWeak )
    {
        result += weak_pawn_pcsq[indexByColor[color][index]];
        if (!flagIsOpposed)
        {
            result -= 4;
        }
    }
    
    return result;
}

/*
 * Kiralytol valo tavolsaga a tamadonak (ezt nevezi tropism-nak)
 */
short int getTropism(INT8U square1, INT8U square2) 
{
    return 7 - (abs(RANK(square1) - RANK(square2)) + abs(FILE(square1) - FILE(square2)));
}
