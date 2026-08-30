#ifndef PROFILE_H
#define PROFILE_H

#include "spall.h"

Internal void prof_begin(const char *str, L1 length);

# if (PROF_ENABLED)
#  define ProfInit() spall_init_file("profile.spall", 1, &spall_ctx)
#  define ProfShutdown() spall_quit(&spall_ctx)
#  define ProfBegin(x) prof_begin(x, sizeof(x)-1)
#  define ProfFuncBegin() ProfBegin(__FUNCTION__)
#  define ProfEnd(x) spall_buffer_end(&spall_ctx, &spall_buffer, os_clock())
#  define ProfScope(x) DeferLoop(ProfBegin(x), ProfEnd())
#  define ProfFlush() spall_buffer_flush(&spall_ctx, &spall_buffer)
# else
#  define ProfInit()
#  define ProfShutdown()
#  define ProfBegin(x)
#  define ProfFuncBegin(x)
#  define ProfEnd(x)
#  define ProfScope(x)
#  define ProfFlush()
# endif

#endif
