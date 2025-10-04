#if (CPU_ && DEF_)

#define PROT_READ 0x1
#define PROT_WRITE 02
#define PROT_NONE 0x0
#define MAP_PRIVATE 0x02
#define MAP_ANON 0x20
#define MAP_FAILED L1_(-1)
#define MADV_DONTNEED 4

#define O_RDONLY 0
#define O_WRONLY 1
#define O_CREAT 100
#define SEEK_SET 0
#define SEEK_END 2 

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define CLOCK_MONOTONIC 1

typedef L1 pthread_t;
typedef struct {
  B1 __opaque[32];
} pthread_barrier_t;

#endif

#if (CPU_ && TYP_)

typedef L1 ThreadFunc(L1);

typedef struct Thread Thread;
struct Thread {
  pthread_t handle;
};

typedef struct Barrier Barrier;
struct Barrier {
  pthread_barrier_t handle;
};

typedef struct Posix_Time_Spec Posix_Time_Spec;
struct Posix_Time_Spec {
  L1 seconds;
  L1 nanoseconds;
};

extern L1 stdin;
extern L1 stdout; 
extern L1 stderr;

Internal L1 os_reserve(L1);
Internal void os_commit(L1, L1);
Internal void os_release(L1, L1);

SI1 printf(CString, ...);
SI1 fprintf(L1, CString, ...);
SI1 fflush(L1);

L1 memset(L1, SI1, L1);
L1 memmove(L1, L1, L1);

Internal L1 os_clock(void);

L1 mmap(L1, L1, I1, I1, I1, L1);
I1 munmap(L1, L1);
I1 mprotect(L1, L1, I1);
I1 madvise(L1, L1, I1);

I1 open(CString, I1, ...);
L1 lseek(I1, L1, I1);
L1 read(I1, L1, L1);
L1 write(I1, L1, L1);
I1 close(I1);

SI1 clock_gettime(I1, L1);

I1 get_nprocs(void);
I1 pthread_create(L1, L1, L1, L1);
I1 pthread_join(pthread_t, L1);
I1 pthread_barrier_init(L1, L1, I1);
I1 pthread_barrier_destroy(L1);
I1 pthread_barrier_wait(L1);

#endif

#if (CPU_ && ROM_)

Internal String8 os_read_entire_file(L1 arena, CString filename) {
  I1 file = open(filename, O_RDONLY);
  if (LtSI1(file, 0)) {
    return (String8){0};
  }

  L1 size = lseek(file, 0, SEEK_END);
  lseek(file, 0, SEEK_SET);

  L1 buffer = arena_push(arena, size);
  L1 bytes_read = read(file, buffer, size);
  Assert(size == bytes_read);

  close(file);

  String8 result = {
    .str = buffer,
    .len = size,
  };
  return result;
}

Internal L1 os_write_entire_file(CString filename, L1 data, L1 data_size) {
  I1 file = open(filename, O_CREAT | O_WRONLY, 0666);
  if (LtSI1(file, 0)) {
    return 0;
  } 

  L1 bytes_written = write(file, data, data_size);
  close(file);

  return bytes_written;
}

Internal L1 os_reserve(L1 size) {
  // TODO: Optinally do large pages: MAP_HUGETBL
  L1 result = L1_(mmap(0, size, PROT_NONE, MAP_PRIVATE|MAP_ANON, -1, 0));
  if (result == MAP_FAILED) {
    result = 0;
  }
  return result;
}

Internal void os_commit(L1 ptr, L1 size) {
  mprotect(ptr, size, PROT_READ|PROT_WRITE);
}

Internal void os_decommit(L1 ptr, L1 size) {
  madvise(ptr, size, MADV_DONTNEED);
  mprotect(ptr, size, PROT_NONE);
}

Internal void os_release(L1 ptr, L1 size) {
  munmap(ptr, size);
}

Internal I1 os_num_cores(void) {
  return get_nprocs();
}

Internal Thread os_thread_launch(ThreadFunc *func, L1 ptr) {
  Thread result = {0};

  pthread_create(L1_(&result.handle), 0, L1_(func), ptr); 

  return result;
}

Internal void os_thread_join(Thread thread) {
  pthread_join(thread.handle, 0);
}

Internal Barrier os_barrier_alloc(I1 count) {
  Barrier result = {0};
  pthread_barrier_init(L1_(&result.handle), 0, count);
  return result;
}

Internal void os_barrier_release(L1 barrier) {
  L1 handle_ptr = L1_(&TR_(Barrier, barrier)->handle);
  pthread_barrier_destroy(handle_ptr);
}

Internal void os_barrier_wait(L1 barrier) {
  L1 handle_ptr = L1_(&TR_(Barrier, barrier)->handle);
  pthread_barrier_wait(handle_ptr);
}

Internal L1 os_clock(void) {
  Posix_Time_Spec timespec = {0};
  clock_gettime(CLOCK_MONOTONIC, L1_(&timespec));
  L1 result = timespec.seconds * 1000000000 + timespec.nanoseconds;
  return result;
}

#endif