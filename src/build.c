#if (!defined(PRE_))
#define PRE_

#ifndef DEV_
# define DEV_ 0
#endif

#define DEF_ 0
#define HEADER 0
#define SOURCE 0

# include "base.c"
#endif

#ifndef BOT_
# define BOT_
# undef DEF_
# define DEF_ 1
# include __FILE__
# undef DEF_
# define DEF_ 0

# undef HEADER
# define HEADER 1
# include __FILE__
# undef HEADER
# define HEADER 0

# undef SOURCE
# define SOURCE 1
NoInline void WrmBas(void) { Crash(0); }
# include __FILE__
NoInline void WrmEnd(void) { Crash(1); }
# undef SOURCE
# define SOURCE 0

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

// #define RT_APP
#include "rt.c"

#include "editor_new.c"

// #include "editor.c"
#endif
