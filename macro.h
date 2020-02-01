#ifndef MACRO_H
#define MACRO_H

#ifndef EM_BUILD
//#define EM_BUILD // Emscripten build
#endif // !EM_BUILD

#ifndef PVLOG
#define PVLOG // principal variation - legjobb lepesek
#endif // !PVLOG

#ifndef BOARD_CHECK
//#define BOARD_CHECK // check whether board before move is equal with board after unmove
#endif // !BOARD_CHECK

#ifndef ZOBRIST_CPW
//#define ZOBRIST_CPW
#endif

#ifndef ZOBRIST
#define ZOBRIST
#endif

#ifdef ZOBRIST_CPW
//#define ZOBRIST
#endif

#ifndef ZOBRIST_INTEGRITY_CHECK
//#define ZOBRIST_INTEGRITY_CHECK
#endif

#ifndef LOG_NEXT_MOVE
//#define LOG_NEXT_MOVE
#endif

#ifndef PVLOG2_ENABLED
//#define PVLOG2_ENABLED
#endif

#ifndef LOG_ENABLED
//#define LOG_ENABLED
#endif

#ifdef LOG_ENABLED
//#define PV2LOG_ENABLED
#endif

#ifdef PVLOG2_ENABLED
#define LOG_SEARCH_ENABLED()
#endif

#ifndef PV_SEARCH
#define PV_SEARCH
#endif

#ifndef HISTORY
#define HISTORY
#endif

#ifndef SEE_BLIND
#define SEE_BLIND
#endif 

#ifndef LOG_SEARCH_ENABLED
//#define LOG_SEARCH_ENABLED
#endif

#ifndef LOG_SEARCH_CPW_STYLE
//#define LOG_SEARCH_CPW_STYLE
#endif

#ifdef  LOG_SEARCH_CPW_STYLE
//#undef LOG_SEARCH_ENABLED
#endif

#ifndef SWITCH_TO_NORMAL_MOVE_GENERATION
//#define SWITCH_TO_NORMAL_MOVE_GENERATION
#endif

#ifndef PRUNING
#define PRUNING
#endif

#ifndef EVALUATE_LEVEL_2
#define EVALUATE_LEVEL_2
#endif

#endif

