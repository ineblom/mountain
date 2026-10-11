Global SpallProfile            spall_ctx;
Global ThreadLocal SpallBuffer spall_buffer;
Global I1                      prof_next_tid;

Internal void prof_init(void)
{
  // Spall version 3 expects nanoseconds per timestamp tick.
  D1 timestamp_unit = 1.0;

#if defined(__x86_64__) || defined(__i386__)
  L1 start_ns    = os_clock();
  L1 start_ticks = clock_L1();
  L1 end_ns     = start_ns;

  while (end_ns - start_ns < 100000000LLU)
  {
    os_sleep(100000000LLU - (end_ns - start_ns));
    end_ns = os_clock();
  }

  L1 end_ticks = clock_L1();
  timestamp_unit = (D1)(end_ns - start_ns) / (D1)(end_ticks - start_ticks);
#endif

  spall_init_file("profile.spall", timestamp_unit, &spall_ctx);
}

Internal L1 prof_clock(void)
{
#if defined(__x86_64__) || defined(__i386__)
  return clock_L1();
#else
  return os_clock();
#endif
}

Internal void prof_begin(const char *str, L1 length)
{
  if (spall_buffer.data == 0)
  {
    spall_buffer.length  =  MiB(1);
    spall_buffer.data    =  os_reserve(spall_buffer.length);

    os_commit(spall_buffer.data, spall_buffer.length);

    spall_buffer.tid  =  atomic_add_I1(&prof_next_tid, 1) + 1;
    spall_buffer.pid  =  0;

    spall_buffer_init(&spall_ctx, &spall_buffer);
  }

  spall_buffer_begin(&spall_ctx, &spall_buffer, str, length, prof_clock());
}
