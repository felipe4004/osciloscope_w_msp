/*******************************
 *          __                   
 *         /\ \                  
 *  __  _  \_\ \     __    ___   
 * /\ \/'\ /'_' \  /'__'\ /'___\ 
 * \/>  <//\ \ \ \/\  __//\ \__/ 
 *  /\_/\_\ \___,_\ \____\ \____\
 *  \//\/_/\/__,_ /\/____/\/____/
 * 
 *  Author: declis (xdec.de
 ********************************/ 

#include <msp430f5529.h>
#include "typedef.h"
#include "init_display.h"
#include "lib_math.h"
#include "lib_lcd.h"
#include "delay.h"
#include "const_data.h"

#define C_BORDER    0x0005
#define s_num       110

uchar space_char=1;		// space between chars

// help vars
uint x3=0;
sint x=0,y=0,x1=0,y1=0,x2=0,y2=0,r=0,i=0,z=0;

void main(void)
{
	WDTCTL=WDTPW+WDTHOLD;		
  	
  	init_LCD(C_BLACK);			// init. Display
  	
  	// FADE IN HELLO WORLD -------------------------------------------
  	while(x<64)
  	{
  		// fade in via set_RGB function
  		draw_string(90,160,string_00,set_RGB16(0,x++,0),TRANSP,0,0);
  		wait_ms(60);
  	}
  	x=0;
	// CREATE BORDERS ------------------------------------------------
	fill_display(lcd_width,lcd_height,C_BORDER);
	x=lcd_height/2;y=lcd_height/2;
	while(x!=45)
	{
		fill_rect(0,239,x,x,C_BLACK);
		fill_rect(0,239,x-1,x-1,C_BORDER+10);
		x--;
		fill_rect(0,239,y,y,C_BLACK);
		fill_rect(0,239,y+1,y+1,C_BORDER+10);
		y++;
		wait_ms(10);
	}
	x=0;y=0;
	fill_rect(0,239,40,40,C_BORDER+10);		// line
	fill_rect(0,239,280,280,C_BORDER+10);	// line
	// ---------------------------------------------------------------	
	
	wait_ms(1000);
	
}
