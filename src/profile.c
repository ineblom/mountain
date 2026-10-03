Global SpallProfile            spall_ctx;
Global ThreadLocal SpallBuffer spall_buffer;

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

    spall_buffer.tid  =  lane_idx();
    spall_buffer.pid  =  0;

    spall_buffer_init(&spall_ctx, &spall_buffer);
  }

  spall_buffer_begin(&spall_ctx, &spall_buffer, str, length, prof_clock());
}
