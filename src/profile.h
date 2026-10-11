#ifndef PROFILE_H
#define PROFILE_H

#include "spall.h"

Internal void prof_init(void);
Internal void prof_begin(const char *str, L1 length);

#if (PROF_ENABLED)
#define ProfInit() prof_init()
#define ProfShutdown() spall_quit(&spall_ctx)
#define ProfBegin(x) prof_begin(x, sizeof(x) - 1)
#define ProfBeginFunc() ProfBegin(__FUNCTION__)
#define ProfEnd(x) spall_buffer_end(&spall_ctx, &spall_buffer, prof_clock())
#define ProfScope(x) DeferLoop(ProfBegin(x), ProfEnd())
#define ProfFlush() do { if (spall_buffer.data && spall_buffer.head > sizeof(SpallBufferHeader)) spall_buffer_flush(&spall_ctx, &spall_buffer); } while (0)
#else
#define ProfInit()
#define ProfShutdown()
#define ProfBegin(x)
#define ProfBeginFunc()
#define ProfEnd(x)
#define ProfScope(x)
#define ProfFlush()
#endif

#endif
