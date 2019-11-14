#ifndef LIB_LCD_H_
#define LIB_LCD_H_

extern uchar space_char;			// space between chars

#define RD				BIT0		// Read no pino 6.0
#define WR				BIT1		// Write no pino 6.1
#define DC				BIT2		// Data/Cmd no pino P6.2
#define CS				BIT3		// chip select no pino P6.3
#define RES 			BIT4		// reset no pino P6.4

#define lcd_width		240
#define lcd_height		320	
				
#define TRANSP			C_FUCHSIA	// magic pink is transparency fake

#define DISPLAY_ON		set_instruction(0,0x29)
#define DISPLAY_OFF		set_instruction(0,0x28)
#define MEM_WRITE		set_instruction(0,0x2C)

#define R_MAX			31
#define G_MAX			63
#define B_MAX			31

// 16 VGA Colors (in RGB565, 16bit)
#define C_WHITE			0xFFFF
#define C_SILVER		0xC618
#define C_BLACK			0x0000
#define C_GREY			0x8410
#define C_RED			0xF800
#define C_MAROON		0x8000
#define C_FUCHSIA		0xF81F		
#define C_PURPLE		0x8010
#define C_LIME			0x07E0
#define C_GREEN			0x0400
#define C_YELLOW		0xFFE0
#define C_OLIVE			0x8400
#define C_BLUE			0x001F
#define C_NAVY			0x0010
#define C_AQUA			0x07FF
#define C_TEAL			0x0410

#define C_DARK_BLUE		0x0002

void draw_string_sin(sint,sint,const uchar*,sint,uchar,uint,uint,uchar,uchar);
void draw_string(sint,sint,const uchar*,uint,uint,uchar,uchar);
void draw_char_text_mode(sint,sint,uchar,uint,uint,uchar,uchar);
void draw_char(sint,sint,uchar,uint,uint,uchar,uchar);
void draw_line(sint,sint,sint,sint,uint,uchar,uchar);
void fill_rect(sint,sint,sint,sint,uint);
uchar check_xy(sint,sint);
void draw_pixel_xy(sint,sint,uint,uchar,uchar);
void draw_pixel(uint);
uint set_RGB16(uchar,uchar,uchar);
void fill_display(uint,uint,uint);
uchar set_cursor(sint,sint);
uchar set_cursor_x(sint);
uchar set_cursor_y(sint);
void set_instruction(uchar,uchar);

#endif /*LIB_LCD_H_*/
