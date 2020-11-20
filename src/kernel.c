// GCC provides these header files automatically
// They give us access to useful things like fixed-width types
#include <stddef.h>
#include <stdint.h>

// First, let's do some basic checks to make sure we are using our x86-elf cross-compiler correctly
#if defined(__linux__)
  #error "This code must be compiled with a cross-compiler"
#elif !defined(__i386__)
  #error "This code must be compiled with an x86-elf compiler"
#endif

// This is the x86's VGA textmode buffer. To display text, we write data to this memory location
volatile uint16_t* vga_buffer = (uint16_t*)0xB8000;
// By default, the VGA textmode buffer has a size of 80x25 characters
const int VGA_COLS = 80;
const int VGA_ROWS = 25;
const uint8_t TERM_COLOR_BASE = 0x0F; // Black background, white foreground

//#define VGA_IDX(row,col) ((row * VGA_COLS) + col)
static size_t vga_idx(int row, int col)
{
  return (row * VGA_COLS) + col;
}

// We start displaying text in the top-left of the screen (column = 0, row = 0)
int term_col = 0;
int term_row = 0;
uint8_t term_color = TERM_COLOR_BASE; // Black background, White foreground

// This function initiates the terminal by clearing it
void term_init()
{
  // Clear the textmode buffer
  for (int col = 0; col < VGA_COLS; col ++)
  {
    for (int row = 0; row < VGA_ROWS; row ++)
    {
      // The VGA textmode buffer has size (VGA_COLS * VGA_ROWS).
      // Given this, we find an index into the buffer for our character
      const size_t index = vga_idx(row, col);
      // Entries in the VGA buffer take the binary form BBBBFFFFCCCCCCCC, where:
      // - B is the background color
      // - F is the foreground color
      // - C is the ASCII character
      vga_buffer[index] = ((uint16_t)TERM_COLOR_BASE << 8) | ' '; // Set the character to blank (a space character)
    }
  }
}

// This function places a single character onto the screen
void term_putc(char c)
{
  // Remember - we don't want to display ALL characters!
  switch (c)
  {
  case '\n': // Newline characters should return the column to 0, and increment the row
    {
      for (int col = term_col; col < VGA_COLS; col++)
      {
        const size_t index = vga_idx(term_row, col);
        vga_buffer[index] = ((uint16_t)TERM_COLOR_BASE << 8) | ' ';
      }
      term_col = 0;
      term_row++;
      break;
    }

  case '\t': // Tab should be 4 spaces
  {
#define TAB_SPACE_CNT 4
    if (term_col + TAB_SPACE_CNT >= VGA_COLS)
    {
      for (int col = term_col; col < VGA_COLS; col++)
      {
        const size_t index = vga_idx(term_row, col);
        vga_buffer[index] = ((uint16_t)TERM_COLOR_BASE << 8) | ' ';
      }
      term_col = 0;
      term_row++;
    }
    else
    {
      for (int col = term_col; col < term_col + TAB_SPACE_CNT; col++)
      {
          const size_t index = vga_idx(term_row, col);
          vga_buffer[index] = ((uint16_t)TERM_COLOR_BASE << 8) | ' ';
      }
      term_col += TAB_SPACE_CNT;
    }
    break;
  }

  default: // Normal characters just get displayed and then increment the column
    {
      const size_t index = vga_idx(term_row, term_col);
      vga_buffer[index] = ((uint16_t)term_color << 8) | c;
      term_col++;
      break;
    }
  }

  // Went past last col, move to next row and reset col to start of row
  if (term_col >= VGA_COLS)
  {
    term_col = 0;
    term_row++;
  }

  // Went past last row, scroll up a row
  if (term_row >= VGA_ROWS)
  {
    // Copy all rows up one
    for (int row = 0; row < VGA_ROWS - 1; row++)
    {
      for (int col = 0; col < VGA_COLS; col++)
      {
        const size_t index_to   = vga_idx(row,     col);
        const size_t index_from = vga_idx(row + 1, col);
        vga_buffer[index_to] = vga_buffer[index_from];
      }
    }

    // Clear the last row
    for (int col = 0; col < VGA_COLS; col++)
    {
      const size_t index = vga_idx(VGA_ROWS - 1, col);
      vga_buffer[index] = ((uint16_t)TERM_COLOR_BASE << 8) | ' ';
    }

    term_row = VGA_ROWS - 1;
    term_col = 0;
  }
}

// This function prints an entire string onto the screen
void term_print(const char* str)
{
  for (size_t i = 0; str[i] != '\0'; i++) // Keep placing characters until we hit the null-terminating character ('\0')
    term_putc(str[i]);
}

void term_error(const char* str)
{
  uint16_t term_color_start = term_color;

  term_color = 0x4 << 4; // Red background, black foreground
  term_print("Error");
  term_color = term_color_start;
  term_print(": ");
  term_print(str);
}

void term_warning(const char* str)
{
  uint16_t term_color_start = term_color;

  term_color = 0xe << 4; // Yellow background, black foreground
  term_print("Warning");
  term_color = term_color_start;
  term_print(": ");
  term_print(str);
}

// This function prints an entire string with the given colors
void term_print_color_test()
{
  uint16_t term_color_start = term_color;

  term_print("Color Test:\n");
  for (uint16_t color = 0; color < 0xF; color++)
  {
    term_color = color;
    term_putc('A' + color);
  }
  term_print("\n");
  for (uint16_t color = 0; color < 0xF; color++)
  {
    term_color = (color << 4);
    term_putc('A' + color);
  }
  term_print("\n");
  term_color = term_color_start;
  term_print("Color Test End\n");
}

// This is our kernel's main function
void kernel_main()
{
  // Initiate terminal
  term_init();

  // Display some messages
  term_print("Hello, World 1\n");
  term_print("Hello, World 2\n");
  term_print("Hello, World 3\n");
  term_print("Hello, World 4\n");
  term_print("Hello, World 5\n");
  term_print("Hello, World 6\n");
  term_print("Hello, World 7\n");
  term_print("Hello, World 8\n");
  term_print("Hello, World 9\n");
  term_print("Hello, World 10\n");
  term_print("Hello, World 11\n");
  term_print("Hello, World 12\n");
  term_print("Hello, World 13\n");
  term_print("Hello, World 14\n");
  term_print("Hello, World 15\n");
  term_print("Hello, World 16\n");
  term_print("Hello, World 17\n");
  term_print("Hello, World 18\n");
  term_print("Hello, World 19\n");
  term_print("Hello, World 20\n");
  term_print("Welcome to the kernel.\n");

  term_print_color_test();
  
  term_print("OK now I am going to print a very long string that does not end with a line feed. Do you see the string? If it doesn't show up, burn the computer and let's never talk about this again. OK? Thanks. This message will now self destruct in 5... 4... 3... 2... 1... Just kidding. OK, here comes the line feed.\n");

  term_error("This is an error message\n");
  term_warning("This is a warning\n");

}
