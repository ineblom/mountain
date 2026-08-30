#ifndef DEV_
# define DEV_ 0
#endif

# include "base.h"

////////////////////////////////
// kti: Modules

//- kti: Config
#define PROF_ENABLED 0

//- kti: Headers
#include "arena.h"
#include "strings.h"
#include "hash.h"
#include "os.h"
#include "lane.h"
#include "profile.h"
#include "math.h"
#include "image.h"
#include "gfx.h"
#include "font_provider.h"
#include "font_cache.h"
#include "draw.h"

#if META_APP
#include "meta.h"
#else

#include "ui.h"
#include "rt.h"
#include "editor.h"
#include "lister.h"

#endif

//- kti: Sources
#define SOURCE 1

NoInline void WrmBas(void) { Crash(0); }

#include "arena.c"
#include "strings.c"
#include "hash.c"
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
#include "rt.c"
#include "editor.c"
#include "lister.c"

#endif

NoInline void WrmEnd(void) { Crash(1); }

#undef SOURCE
