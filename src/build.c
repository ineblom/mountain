#if (!defined(PRE_))
#define PRE_

#ifndef CPU_
#define CPU_ 0
#endif
#ifndef DEV_
#define DEV_ 0
#endif
#ifndef GPU_
#define GPU_ 0
#endif

#if CPU_
#define DEF_ 0
#define TYP_ 0
#define RAM_ 0
#define ROM_ 0
#endif

#include "base.c"
#endif

#if (CPU_ && (!defined(BOT_)))
#define BOT_

#undef DEF_
#define DEF_ 1
#include __FILE__
#undef DEF_
#define DEF_ 0

#undef TYP_
#define TYP_ 1
#include __FILE__
#undef TYP_
#define TYP_ 0

#undef RAM_
#define RAM_ 1

typedef struct {
  #include __FILE__
  Align(64) L1 end[1024*1024/8];
} RamT;

Global Align(64) L1 ramM[sizeof(RamT)/8];
#define ramR TR_(RamT, L1_(ramM))
#define ramV TV_(RamT, L1_(ramM))

Global ThreadLocal L1 lane_ctx;

#undef RAM_
#define RAM_ 0

#undef ROM_
#define ROM_ 1
Inline void WrmBas(void) { Crash(0); }
#include __FILE__
Inline void WrmEnd(void) { Crash(1); }
#undef ROM_
#define ROM_ 0

#endif

// Modules
#include "arena.c"
#include "strings.c"
#include "os.c"
#include "lane.c"
#include "math.c"
#include "image.c"
// #include "rt.c" 
#include "testing.c"