# Mountain

A pile of C code. Used as a base environment for multiple projects.

## Files

- `base.c` typedefs and macros.
- `arena.c` allocator.
- `lane.c` (has main function)
  1. starts up a thread (lane) for each core.
  2. allocates an arena for each lane.
  3. runs externally defined lane function on each thread.
  4. provides utility functions dividing work and synchronizing between lanes.
- `build.c`
  - unity build; I.e. includes all other .c files (modules).
  - includes itself multiple times, setting a different compile time constant each time.
  - DEF\_, TYP\_, RAM\_, ROM\_ define separate sections of included modules.
  - eliminates the need for .h files.
- `os.c` operating system abstraction.
- `strings.c` custom string type and operations.
- `rt.c` simple CPU ray tracer.
