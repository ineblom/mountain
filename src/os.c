Global OS_GFX_State *os_gfx_state = 0;

Internal String8 os_read_entire_file(Arena *arena, String8 filename) {
  Temp_Arena scratch = scratch_begin(0, 0);
  String8 cstr_filename = push_str8_copy(scratch.arena, filename);
  I1 file = open((CString)cstr_filename.str, O_RDONLY);
  if (LtSI1(file, 0)) {
    scratch_end(scratch);
    return (String8){0};
  }

  L1 size = lseek(file, 0, SEEK_END);
  lseek(file, 0, SEEK_SET);
  B1 *buffer = push_array(arena, B1, size);
  L1 bytes_read = read(file, buffer, size);
  close(file);
  scratch_end(scratch);
  Assert(size == bytes_read);

  return (String8){.str = buffer, .len = size};
}

Internal L1 os_write_entire_file(String8 filename, void *data, L1 data_size) {
  Temp_Arena scratch = scratch_begin(0, 0);
  String8 cstr_filename = push_str8_copy(scratch.arena, filename);
  I1 file = open((CString)cstr_filename.str, O_CREAT | O_WRONLY | O_TRUNC, 0666);
  if (LtSI1(file, 0)) {
    scratch_end(scratch);
    return 0;
  }

  L1 bytes_written = write(file, data, data_size);
  close(file);
  scratch_end(scratch);
  return bytes_written;
}

Internal void *os_reserve(L1 size) {
  void *result = mmap(0, size, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
  return result == MAP_FAILED ? 0 : result;
}

Internal void os_commit(void *ptr, L1 size) {
  mprotect(ptr, size, PROT_READ | PROT_WRITE);
}

Internal void os_decommit(void *ptr, L1 size) {
  madvise(ptr, size, MADV_DONTNEED);
  mprotect(ptr, size, PROT_NONE);
}

Internal void os_memory_release(void *ptr, L1 size) {
  munmap(ptr, size);
}

Internal I1 os_core_count(void) {
  long count = sysconf(_SC_NPROCESSORS_ONLN);
  return count > 0 ? (I1)count : 1;
}

//~ kti: Thread

Internal OS_Thread os_thread_launch(ThreadFunc *func, void *ptr) {
  OS_Thread result = {0};
  pthread_create(&result.handle, 0, func, ptr);
  return result;
}

Internal void os_thread_detach(OS_Thread thread) {
  pthread_detach(thread.handle);
}

Internal void os_thread_join(OS_Thread thread) {
  pthread_join(thread.handle, 0);
}

//~ kti: Sync

typedef struct OS_Sync_State OS_Sync_State;
struct OS_Sync_State {
  Arena *arena;
  pthread_mutex_t entity_mutex;
  OS_Mutex first_free_mutex;
  OS_Cond_Var first_free_cond_var;
};

Global OS_Sync_State os_sync_state;
Global pthread_once_t os_sync_once = PTHREAD_ONCE_INIT;

Internal I1 os_cond_var_init_platform(OS_Cond_Var cond_var);
Internal I1 os_cond_var_wait_platform(OS_Cond_Var cond_var, OS_Mutex mutex, L1 endt);

Internal void os_sync_init(void) {
  os_sync_state.arena = arena_alloc(MiB(4));
  pthread_mutex_init(&os_sync_state.entity_mutex, 0);
}

Internal void os_mutex_entity_release(OS_Mutex mutex) {
  pthread_mutex_lock(&os_sync_state.entity_mutex);
  SLLStackPush(os_sync_state.first_free_mutex, mutex);
  pthread_mutex_unlock(&os_sync_state.entity_mutex);
}

Internal OS_Mutex os_mutex_alloc(void) {
  pthread_once(&os_sync_once, os_sync_init);
  pthread_mutex_lock(&os_sync_state.entity_mutex);
  OS_Mutex mutex = os_sync_state.first_free_mutex;
  if (mutex != 0) {
    SLLStackPop(os_sync_state.first_free_mutex);
  } else {
    mutex = push_array(os_sync_state.arena, struct OS_Mutex, 1);
  }
  pthread_mutex_unlock(&os_sync_state.entity_mutex);

  mutex->next = 0;
  I1 init_result = pthread_mutex_init(&mutex->handle, 0);
  if (init_result != 0) {
    os_mutex_entity_release(mutex);
    mutex = 0;
  }
  return mutex;
}

Internal void os_mutex_release(OS_Mutex mutex) {
  if (MemoryIsZeroStruct(&mutex)) return;
  pthread_mutex_destroy(&mutex->handle);
  os_mutex_entity_release(mutex);
}

Internal void os_mutex_take(OS_Mutex mutex) {
  if (MemoryIsZeroStruct(&mutex)) return;
  pthread_mutex_lock(&mutex->handle);
}

Internal void os_mutex_drop(OS_Mutex mutex) {
  if (MemoryIsZeroStruct(&mutex)) return;
  pthread_mutex_unlock(&mutex->handle);
}

Internal void os_cond_var_entity_release(OS_Cond_Var cond_var) {
  pthread_mutex_lock(&os_sync_state.entity_mutex);
  SLLStackPush(os_sync_state.first_free_cond_var, cond_var);
  pthread_mutex_unlock(&os_sync_state.entity_mutex);
}

Internal OS_Cond_Var os_cond_var_alloc(void) {
  pthread_once(&os_sync_once, os_sync_init);
  pthread_mutex_lock(&os_sync_state.entity_mutex);
  OS_Cond_Var cond_var = os_sync_state.first_free_cond_var;
  if (cond_var != 0) {
    SLLStackPop(os_sync_state.first_free_cond_var);
  } else {
    cond_var = push_array(os_sync_state.arena, struct OS_Cond_Var, 1);
  }
  pthread_mutex_unlock(&os_sync_state.entity_mutex);

  cond_var->next = 0;
  I1 init_result = os_cond_var_init_platform(cond_var);
  if (init_result != 0) {
    os_cond_var_entity_release(cond_var);
    cond_var = 0;
  }
  return cond_var;
}

Internal void os_cond_var_release(OS_Cond_Var cond_var) {
  if (MemoryIsZeroStruct(&cond_var)) return;
  pthread_cond_destroy(&cond_var->handle);
  os_cond_var_entity_release(cond_var);
}

Internal I1 os_cond_var_wait(OS_Cond_Var cond_var, OS_Mutex mutex, L1 endt) {
  if (MemoryIsZeroStruct(&cond_var)) return 0;
  if (MemoryIsZeroStruct(&mutex)) return 0;
  return os_cond_var_wait_platform(cond_var, mutex, endt);
}

Internal void cond_var_signal(OS_Cond_Var cond_var) {
  if (MemoryIsZeroStruct(&cond_var)) return;
  pthread_cond_signal(&cond_var->handle);
}

Internal void cond_var_broadcast(OS_Cond_Var cond_var) {
  if (MemoryIsZeroStruct(&cond_var)) return;
  pthread_cond_broadcast(&cond_var->handle);
}

Internal OS_Barrier os_barrier_alloc(I1 count) {
  OS_Barrier result = {.threshold = count, .count = count};
  pthread_mutex_init(&result.mutex, 0);
  pthread_cond_init(&result.condition, 0);
  return result;
}

Internal void os_barrier_release(OS_Barrier *barrier) {
  pthread_cond_destroy(&barrier->condition);
  pthread_mutex_destroy(&barrier->mutex);
}

Internal void os_barrier_wait(OS_Barrier *barrier) {
  pthread_mutex_lock(&barrier->mutex);
  I1 generation = barrier->generation;
  barrier->count -= 1;
  if (barrier->count == 0) {
    barrier->generation += 1;
    barrier->count = barrier->threshold;
    pthread_cond_broadcast(&barrier->condition);
  } else {
    while (generation == barrier->generation) {
      pthread_cond_wait(&barrier->condition, &barrier->mutex);
    }
  }
  pthread_mutex_unlock(&barrier->mutex);
}

Internal L1 os_clock(void) {
  struct timespec ts = {0};
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (L1)ts.tv_sec * 1000000000LLU + (L1)ts.tv_nsec;
}

Internal void os_sleep(L1 time) {
  struct timespec ts = {
    .tv_sec = time / 1000000000LLU,
    .tv_nsec = time % 1000000000LLU,
  };
  nanosleep(&ts, 0);
}

Internal void *os_library_open(String8 filename) {
  Temp_Arena scratch = scratch_begin(0, 0);
  String8 cstr_filename = push_str8_copy(scratch.arena, filename);
  void *handle = dlopen((CString)cstr_filename.str, RTLD_LAZY|RTLD_LOCAL);
  scratch_end(scratch);
  return handle;
}

Internal VoidProc *os_library_load_proc(void *lib, String8 name) {
  Temp_Arena scratch = scratch_begin(0, 0);
  String8 cstr_name = push_str8_copy(scratch.arena, name);
  VoidProc *proc = dlsym(lib, (CString)cstr_name.str);
  scratch_end(scratch);
  return proc;
}

Internal void os_library_close(void *handle) {
  if (handle != 0) {
    dlclose(handle);
  }
}
