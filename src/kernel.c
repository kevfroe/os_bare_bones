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

// We start displaying text in the top-left of the screen (column = 0, row = 0)
int term_col = 0;
int term_row = 0;
uint8_t term_color = TERM_COLOR_BASE; // Black background, White foreground

static inline size_t vga_buffer_index_get(int col, int row)
{
	return (VGA_COLS * row) + col;
}

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
			const size_t index = (VGA_COLS * row) + col;
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
				const size_t index = vga_buffer_index_get(col, term_row);//(VGA_COLS * term_row) + col;
				vga_buffer[index] = ((uint16_t)TERM_COLOR_BASE << 8) | ' ';
			}
			term_col = 0;
			term_row++;
			break;
		}

	default: // Normal characters just get displayed and then increment the column
		{
			const size_t index = (VGA_COLS * term_row) + term_col; // Like before, calculate the buffer index
			vga_buffer[index] = ((uint16_t)term_color << 8) | c;
			term_col ++;
			break;
		}
	}

	// What happens if we get past the last column? We need to reset the column to 0, and increment the row to get to a new line
	if (term_col >= VGA_COLS)
	{
		term_col = 0;
		term_row ++;
	}

	// What happens if we get past the last row? We need to reset both column and row to 0 in order to loop back to the top of the screen
	if (term_row >= VGA_ROWS)
	{
		term_col = 0;
		term_row = 0;
	}
}

// This function prints an entire string onto the screen
void term_print(const char* str)
{
	for (size_t i = 0; str[i] != '\0'; i ++) // Keep placing characters until we hit the null-terminating character ('\0')
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
	// We're here! Let's initiate the terminal and display a message to show we got here.

	// Initiate terminal
	term_init();

	// Display some messages
	term_print("Hello, World!\n");
	term_print("Welcome to the kernel.\n");

  term_print_color_test();

	term_error("This is an error message\n");
	term_warning("This is a warning\n");

	for (int i = 0; i < 20; i++)
	{
		term_print("Line ");
		term_putc('0' + (i % 10));
		term_print("\n");
	}
}
