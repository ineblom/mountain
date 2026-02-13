#if (!defined(PRE_))
#define PRE_

#ifndef DEV_
# define DEV_ 0
#endif

#define DEF_ 0
#define TYP_ 0
#define ROM_ 0

# include "base.c"
#endif

#ifndef BOT_
# define BOT_
# undef DEF_
# define DEF_ 1
# include __FILE__
# undef DEF_
# define DEF_ 0

# undef TYP_
# define TYP_ 1
# include __FILE__
# undef TYP_
# define TYP_ 0

# undef ROM_
# define ROM_ 1
NoInline void WrmBas(void) { Crash(0); }
# include __FILE__
NoInline void WrmEnd(void) { Crash(1); }
# undef ROM_
# define ROM_ 0

#endif

////////////////////////////////
// kti: Modules

//- kti: Config
#define PROF_ENABLED 0

//- kti: Core
#include "arena.c"
#include "strings.c"
#include "os.c"
#include "lane.c"
#include "profile.c"
#include "math.c"
#include "image.c"
#include "gfx.c"
#include "font_provider.c"
#include "font_cache.c"
#include "draw.c"

#if META_APP
#include "meta.c"
#else

#include "ui.c"

#include "testing.c"

// #define RT_APP
// #include "rt.c"

// #include "editor.c"
#endif
