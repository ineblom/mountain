
#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>

typedef struct N_Block N_Block;
struct N_Block
{
  N_Block *next;
  N_Block *prev;

  String8 contents;
};

typedef enum Style
{
  STYLE__RESET,
  STYLE__NOTE,
  STYLE__INPUT,
} Style;

Global I1 needs_redraw = 1;

static void handle_resize(int)
{
  atomic_swap_I1(&needs_redraw, 1);
}

void style(Style style)
{
  CString str = "\033[0m";

  switch (style)
  {
    case STYLE__RESET: break;
    case STYLE__NOTE: str = "\033[37m"; break;
    case STYLE__INPUT: str = "\033[48;2;41;41;41m\033[37m"; break;
  }

  printf(str);
}

int main(void)
{
  Arena *arena = arena_alloc(GiB(1));

  //- kti: Sigaction

  struct sigaction action = {0};
  action.sa_handler = handle_resize;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;

  sigaction(SIGWINCH, &action, 0);

  //- kti: Store original terminal state.
  struct termios original = {0};
  tcgetattr(STDIN_FILENO, &original);

  //- kti: Custom terminal state.
  struct termios custom = original;
  custom.c_lflag &= ~(ECHO | ICANON); // Disable eacho and canonical mode.
  custom.c_cc[VMIN] = 0;
  custom.c_cc[VTIME] = 1;
  
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &custom);

  //- kti: Blocks

  N_Block *first_block = 0;
  N_Block *last_block  = 0;

  N_Block *test_block = push_array(arena, N_Block, 1);
  test_block->contents = str8("This is a test block.");
  DLLPushBack(first_block, last_block, test_block);

  //- kti: Input.

  B1 input_buffer[1024] = {0};
  L1 input_len = 0;

  I1 running = 1;
  while (running)
  {
    B1 bytes = 0;
    ssize_t byte_count = read(STDIN_FILENO, &bytes, 1);

    if (byte_count != 0)
    {
      I1 is_input = char_is_alpha(bytes) || bytes == ' ';
      if (is_input && input_len < ArrayCount(input_buffer))
      {
        input_buffer[input_len] = bytes;
        input_len += 1;
        atomic_swap_I1(&needs_redraw, 1);
      }
    }

    if (needs_redraw)
    {
      needs_redraw = 0;

      struct winsize size = {0};
      ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
      L1 rows = size.ws_row;
      L1 cols = size.ws_col;

      printf("\033[2J\033[H");

      printf("\n");

      for (N_Block *node = first_block; node != 0; node = node->next)
      {
        printf(" ");
        style(STYLE__NOTE);
        printf("%.*s\n", str8_varg(node->contents));
        style(STYLE__RESET);
      }

      printf("\n");

      style(STYLE__INPUT);

      for (L1 col = 0; col < cols; col += 1) printf(" ");

      printf(">");
      printf(" ");
      printf("%.*s", (int)input_len, input_buffer);
      for (L1 col = 2+input_len; col < cols; col += 1) printf(" ");

      for (L1 col = 0; col < cols; col += 1) printf(" ");

      printf("\033[1A\r\033[2C");

      style(STYLE__RESET);

      fflush(stdout);
    }
  } 

  //- kti: Restore original terminal state.
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);

  return 0;
}
