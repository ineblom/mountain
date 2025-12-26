#if  (CPU_ && DEF_)

# include "spall.h"

Internal void prof_begin(const char *str, L1 length);

# define ProfBegin(x) prof_begin(x, sizeof(x)-1)
# define ProfFuncBegin() ProfBegin(__FUNCTION__)
# define ProfEnd(x) spall_buffer_end(&spall_ctx, &spall_buffer, os_clock())
# define ProfScope(x) for (int _i_ = (ProfBegin(x), 0); !_i_; _i_ += 1, ProfEnd())

Global SpallProfile spall_ctx;
Global ThreadLocal SpallBuffer spall_buffer;

#endif

#if (CPU_ && ROM_)

Internal void prof_init(void) {
	spall_init_file("profile.spall", 1, &spall_ctx);
}

Internal void prof_begin(const char *str, L1 length) {
	if (spall_buffer.data == 0) {
		spall_buffer.length = MiB(1);
		spall_buffer.data = os_reserve(spall_buffer.length);
		os_commit(spall_buffer.data, spall_buffer.length);
		spall_buffer.tid = lane_idx();
		spall_buffer.pid = 0;
		spall_buffer_init(&spall_ctx, &spall_buffer);
	}

	spall_buffer_begin(&spall_ctx, &spall_buffer, str, length, os_clock());
}

#endif
