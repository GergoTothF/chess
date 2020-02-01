#ifndef ALPHABETA_H
#define ALPHABETA_H

#include "mover.h"
#include "macroutils.h"
#include "evaluator.h"

const int draw_middlegame = -10; // middlegame draw value
const int draw_endgame = 0;   // endgame draw value

/*
 * Only allowed unique positions -> else expecting draw
 * nem varjuk meg a harmadik ismetlest, eleg ketto
 */
inline bool isRepetition()
{
    for (int i = 0; i < HistoryCount; i++)
    {
        if (PositionHistory[i] == Board.ZobristHash)
        {
            return true;
        }
    }

    return false;
}

inline short int contempt()
{
    int result = draw_middlegame;

    if (Board.PieceMaterial[!Board.SideToMove] < EndGameMatrial)
    {
        result = draw_endgame;
    }

    if (Board.SideToMove == MySide)
    {
        return result;
    }
    else
    {
        return -result;
    }
}

inline short int quiescence(short int alpha, short int beta, INT8U quiescenceDepth)
{
    ++Nodes;
    ++QuiescenceNodes;

    // get a "stand pat" score, masodik korben is jo, mert ott visszautok, ha az elfogadhato -> standpat ok 
    short int score = evaluate();

    /*
     * Paros depth-nel a quiescence nulladik szintje negalodni fog a depth = 1 -en,
     * tehat a minnel kisebb standpat a jo (mert "-1" szerezodni fog), es azt fogja elfogadni
     * sotet (paros depth-nel, depth = 1 a sotet), ami a "-1"-szerezodes utan a legnagyobb
     *
     * A beta az, amit az ellenfel minimum biztositotta maganak,
     * az alfa, amit maximum biztosit maganak az aktualis fel.
     * A kovetkezo korben az uj alfa a regi beta "-1-szerese",
     * az uj beta = -alfa (regi alfa). Kereses kozben az [alfa,beta] tartomany szukul.
     *
     * beta cut-off az ellenfel ennel mar jobbat (jobb agat) talalt maganak
     */
    int stand_pat = score;

    // check if stand-pat score causes a beta cutoff 
    if (score >= beta)
    {
        LOG_MOVE2("0q", quiescenceDepth, score, alpha, beta);
        return beta;
    }

    // check if stand-pat score may become a new alpha 
    if (alpha < score)
    {
        alpha = score;
    }

    /*
     *  We have taken into account the stand pat score, and it didn't let us
     *  come to a definite conclusion about the position. So we have to search
     */

    Move moveList[256];
    INT8U moveCount = mover::generateQuiscenceMove(moveList);

    for (INT8U i = 0; i < moveCount; ++i)
    {
        mover::sortMove(moveCount, moveList, i);

        if (moveList[i].CapturedPiece == piece::WhiteKing || moveList[i].CapturedPiece == piece::BlackKing)
        {
            return INF;
        }

        /*
         *  Delta cutoff - a move guarentees the score well below alpha, so
         *  there's no point in searching it. We don't use his heuristic (!) in
         *  the endgame, because of the insufficient material issues.
         */

        if (moveList[i].CapturedPiece != piece::Empty && // enpassant is good
            stand_pat + EvaluationData.PieceValue[moveList[i].CapturedPiece] < (alpha - 200) &&
            Board.PieceMaterial[!Board.SideToMove] - EvaluationData.PieceValue[moveList[i].CapturedPiece] > EndGameMatrial &&
            !mover::isPromotionMove(moveList[i]))
        {
            continue;
        }

        /*
         * badCapture() replaces a cutoff based on the Static Exchange Evalu-
         * ation, marking the place where it ought to be coded. Despite being
         * just a hack, it saves quite a few nodes.
         * Igazabol ez a feltetel rendszer nagyjabol az isBadCapture()-rel azonos
         */

        if (mover::isBadCapture(moveList[i]) &&
            !mover::canMoveSimplified(moveList[i]) &&
            !mover::isPromotionMove(moveList[i]))
        {
            continue;
        }

        /*
         *  Cutoffs misfired, we have to search the current move
         */

        SAVE_BOARD_SCORES(MaxDepth + quiescenceDepth)

        mover::makeMove(moveList[i]);

        PREPARE_LOG_NEXT_MOVE(moveList[i], -1, MaxDepth + quiescenceDepth)

        CHECK_BOARD_INTEGRITY_MOVE();

        ADD_HISTORY(MaxDepth + quiescenceDepth, moveList[i].From, moveList[i].To)

        score = -quiescence(-beta, -alpha, quiescenceDepth + 1);

        mover::unmakeMove(moveList[i]);

        CHECK_BOARD_INTEGRITY_UNMOVE(MaxDepth + quiescenceDepth);

        if (score > alpha)
        {
            if (score >= beta)
            {
                LOG_MOVE("bq", quiescenceDepth, moveList[i], score, alpha, beta, false);
                CHECK_LOG_NEXT_MOVE(moveList[i], -1, MaxDepth + quiescenceDepth, score, alpha, beta, "qb")
                    return beta;
            }

            LOG_MOVE("aq", quiescenceDepth, moveList[i], score, alpha, beta, false);
            CHECK_LOG_NEXT_MOVE(moveList[i], -1, MaxDepth + quiescenceDepth, score, alpha, beta, "cq")

                COPY_PV_HISTORY(MaxDepth + quiescenceDepth)

                alpha = score;
        }
        else
        {
            LOG_MOVE("nq", quiescenceDepth, moveList[i], score, alpha, beta, false);
            CHECK_LOG_NEXT_MOVE(moveList[i], -1, MaxDepth + quiescenceDepth, score, alpha, beta, "nq")
        }
    }

    if (alpha == stand_pat)
    {
        ADD_PV_HISTORY(MaxDepth + quiescenceDepth, 1, 1, (quiescenceDepth & 1) == 0 ? stand_pat : -stand_pat)
    }

    return alpha;
}

inline short int SEARCH(INT8U depth, INT8U ply, short int alpha, short int beta, bool isNullAllowed, bool isPV)
{
    short int score = -INF;
#ifdef ZOBRIST
    ettflag tt_flag = TT_ALPHA;
#endif
    Move moveList[256]; // move list, felszabaditashoz jo
    INT8U moveCount;
    Move move;
    bool raised_alpha = false;
    INT8U moves_tried = 0;

    SAVE_MAX_INCEMENTED_DEPTH()

    INT8U ownKingIndex = 0;
    BSF(ownKingIndex, Board.SquareByPiece[Board.SideToMove ? piece::BlackKing : piece::WhiteKing]);
    bool isOwnChecked = mover::isAttacked(!Board.SideToMove, ownKingIndex);

    /*
     * Are we in check? If so, extend. It also means that program will
     * never enter in quiescence search while in check
     */
    if (isOwnChecked)
    {
        if (depth == MaxDepth)
        {
            MaxDepth++;
        }

        depth++;

        //INCREMENT_MAX_DEPTH()
    }

    if (depth < 1)
    {
        return quiescence(alpha, beta, 0);
    }

#ifdef ZOBRIST
    INT8U zoBestFrom = 0;
    INT8U zoBestTo = 0;

    ZobristFrom = 255;
    ZobristTo = 255;

    short int ttScore = tteval_probe(depth, alpha, beta);

    // in case of PV node (isPV==false), then we can not return scores out of bounds (only alpha < score < beta)
    if (ttScore != InvalidScore && (!isPV || (ttScore > alpha && ttScore < beta)))
    {
        return ttScore;
    }
#endif

    /*
     * Az olyan move-okat nem szamolom, amit nemi szamitas utan
     * return vagy continue-val abbahagyunk szamolni, azokat nem szamolom.
     * Nagyjabol azokra koncentralok, amelyik tovabbi lepeseket indukal.
     * Kulonosen azokat, amelyeket kiertekelek.
     * A LogNode-oknal azokat ketszer szamolom,
     * amihez nem tartozik move se (LOG_MOVE2)
     */
    ++Nodes;

    if (isRepetition())
    {
        return contempt();
    }

#ifdef PRUNING
    /*
     * Eval pruning, static null move
     * Reverse Futility Pruning: https://www.chessprogramming.org/Reverse_Futility_Pruning
     * https://open-chess.org/viewtopic.php?t=3056
     * http://www.talkchess.com/forum3/viewtopic.php?t=41302&start=1
     *  van annyira jo pozicioban (eval margin-nal fellette van a beta-nak: amit a masik biztositott maganak),
     * hogyha nem lepne, akkor is ekkora elonye lenne -> meg allhatunk, de mivel terjunk vissza?
     */

    if (depth < 3 &&
        !isPV &&
        !isOwnChecked &&
        std::abs(beta - 1) > -INF + 100)
    {
        int static_eval = evaluate();

        int eval_margin = 120 * depth;

        if (static_eval - eval_margin >= beta)
        {
            return static_eval - eval_margin;
        }
    }

    /*
     * Here is introduced: NULL MOVE PRUNING. It means allowing opponent
     * to execute two moves in a row, i.e. capturing something and escaping
     * a recapture. If this cannot wreck our position, then it is so good
     * that there's no point in searching futher. The flag "isNullAllowed"
     * ensures we don't do two null moves in a row. Null move is not used
     * in the endgame because of the risk of zugzwang
     */
    if (depth > 2 &&
        isNullAllowed &&
        !isPV &&
        evaluate() > beta &&
        Board.PieceMaterial[Board.SideToMove] > EndGameMatrial &&
        !isOwnChecked)
    {
        INT8S ep_old = Board.Enpassant;
        mover::move_makeNull();

        /*
         * We use a so called adaptive null move pruning. Size of reduction
         * depends on remaining depth
         */
        char R = 2;
        if (depth > 6) R = 3;

        score = -SEARCH_CALLED(depth - R - 1, ply + 1, -beta, -beta + 1, false, false);

        mover::move_unmakeNull(ep_old);

        /*
         * megnezzuk, hogy lesz-e fail hard, azaz beta cut off
         * ha beta cut off van, akkor -(-beta+1)-gyel ter vissza, ami beta-1 < beta lesz
         */
        if (score >= beta)
        {
            return beta;
        }
    }   // end of null move code
#endif

    moveCount = mover::generateMove(moveList, false);

    for (INT8U i = 0; i < moveCount; ++i)
    {
        if (depth == MaxDepth)
        {
            if (checkTimeOut())
            {
                break;
            }
        }

        SAVE_BOARD_SCORES(reversedDepth)

        mover::sortMove(moveCount, moveList, i); // pick the best of untried moves

        move = moveList[i];

        mover::makeMove(move);

        SAVE_PV2LOG(reversedDepth, move)

        PREPARE_LOG_NEXT_MOVE(move, MaxDepth, depth)

#ifdef ZOBRIST_INTEGRITY_CHECK
        checkZobristIntegrity();
#endif

        CHECK_BOARD_INTEGRITY_MOVE();

        ADD_HISTORY(MaxDepth - depth, moveList[i].From, moveList[i].To)

        // filter out illegal moves
        BSF(ownKingIndex, Board.SquareByPiece[Board.SideToMove ? piece::WhiteKing : piece::BlackKing]);

        if (mover::isAttacked(Board.SideToMove, ownKingIndex))
        {
            mover::unmakeMove(move);

#ifdef ZOBRIST_INTEGRITY_CHECK
            checkZobristIntegrity();
#endif

            CHECK_BOARD_INTEGRITY_UNMOVE(reversedDepth);

            LOG_MOVE("in", depth, move, 255, alpha, beta, isPV)

            continue;
        }

        moves_tried++;

        INCREMENT_REVERSED_DEPTH()

        RESEARCH()

        mover::unmakeMove(move);

#ifdef ZOBRIST_INTEGRITY_CHECK
        checkZobristIntegrity();
#endif

        CHECK_BOARD_INTEGRITY_UNMOVE(reversedDepth);


        if (score > alpha)
        {
            if (InitialBoardHash == Board.ZobristHash)
            {
                BestMove = move;
                BestMove.Score = score;
            }

            if (score >= beta)
            {
                LOG_MOVE("bn", depth, move, score, alpha, beta, isPV);

                CHECK_LOG_NEXT_MOVE(move, MaxDepth, depth, score, alpha, beta, "bn")

#ifdef ZOBRIST
                tteval_save(depth, beta, TT_BETA, move.From, move.To);
#endif       

                SAVE_HISTORY_MOVE()

                RESTORE_MAX_INCEMENTED_DEPTH()

                return beta;   //  fail hard beta-cutoff
            }

#ifdef ZOBRIST
            tt_flag = TT_EXACT;
#endif

            LOG_MOVE("an", depth, move, score, alpha, beta, isPV);

            CHECK_LOG_NEXT_MOVE(move, MaxDepth, depth, score, alpha, beta, "cn")

            COPY_PV_HISTORY(MaxDepth - depth)

#ifdef ZOBRIST
            zoBestFrom = move.From;
            zoBestTo = move.To;
#endif

            raised_alpha = true;

            LOGPV2(depth, reversedDepth)

            alpha = score; // alpha acts like max in MiniMax
        }
        else
        {
            LOG_MOVE("nn", depth, move, score, alpha, beta, isPV);
            CHECK_LOG_NEXT_MOVE(move, MaxDepth, depth, score, alpha, beta, "nn")
        }
    }

    BSF(ownKingIndex, Board.SquareByPiece[Board.SideToMove ? piece::BlackKing : piece::WhiteKing]);

    if (isInsufficientMaterial() ||
        (moves_tried == 0 && !mover::isAttacked(!Board.SideToMove, ownKingIndex))) // handle stalemated
    {
        alpha = contempt();

        if (depth == MaxDepth)
        {
            BestMove.Score = alpha;
            BestMoveType = bestmove::StaleMate;
        }
    }

    INT8U enemyKingIndex = 0;
    BSF(enemyKingIndex, Board.SquareByPiece[Board.SideToMove ? piece::WhiteKing : piece::BlackKing]);

    if (moves_tried == 0)
    {
        if (mover::isAttacked(!Board.SideToMove, ownKingIndex))
        {
            if (InitialBoardHash == Board.ZobristHash)
            {
                BestMoveType = bestmove::NoMoveInCheckMate;
            }

            if (score == -INF)
            {
                return score + ply;
            }
            else if (score == INF)
            {
                return score - ply;
            }
        }
    }

#ifdef ZOBRIST
    tteval_save(depth, alpha, tt_flag, zoBestFrom, zoBestTo);
#endif

    RESTORE_MAX_INCEMENTED_DEPTH()

    return alpha;
}

inline short int searchBase()
{
    StartingTime = std::chrono::system_clock::now();

    InitialBoardHash = Board.ZobristHash;

    short int result = 0;

    //#ifdef ZOBRIST
    //    MaxDepth = 2;
    //    SEARCH_CALLED_FIRST(result, MaxDepth, 0, -INF, INF, false);
    //    MaxDepth = 4;
    //    SEARCH_CALLED_FIRST(result, MaxDepth, 0, -INF, INF, false);
    //    MaxDepth = 6;
    //    SEARCH_CALLED_FIRST(result, MaxDepth, 0, -INF, INF, false);
    //#endif

    for (int i = 8; i <= 8; i += 2)
    {
        MaxDepth = i;

        SEARCH_CALLED_FIRST(result, MaxDepth, 0, -INF, INF, false, true);
    }

    return result;
}

#endif // ALPHABETA_H
