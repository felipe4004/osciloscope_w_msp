/*******************************
 *          __                   
 *         /\ \                  
 *  __  _  \_\ \     __    ___   
 * /\ \/'\ /'_' \  /'__'\ /'___\ 
 * \/>  <//\ \ \ \/\  __//\ \__/ 
 *  /\_/\_\ \___,_\ \____\ \____\
 *  \//\/_/\/__,_ /\/____/\/____/
 * 
 *  Author: declis (xdec.de)
 ********************************/ 

#include <msp430f5529.h>
#include "typedef.h"
#include "ascii_char.h"
#include "lib_lcd.h"
#include "lib_math.h"
#include "delay.h"

uint byte,w_index,h_index;
uchar bit_num;


void draw_string_sin(sint x, sint y, const uchar *str, sint x_s, uchar div, 
						uint f_color, uint b_color, uchar sx, uchar sy)
{
	while(*str!=0&&x<lcd_width)
	{
		if(x>=-(sint)((f_width-1)*(sx+1)))	// part of char inside display
			draw_char(x,y+get_sin(x_s)/div,*str,f_color,b_color,sx,sy);
		*str++;
		x+=f_width+space_char;
		x_s+=f_width+space_char;
	}
}

void draw_string(sint x, sint y, const uchar *str, uint f_color, uint b_color, uchar sx, uchar sy)
{	
	while(*str!=0&&x<lcd_width)
	{
		if(x>=-(sint)((f_width-1)*(sx+1)))	// part of char inside display
		{
			draw_char(x,y,*str,f_color,b_color,sx,sy);
			if(b_color!=TRANSP)
			{
				bit_num=space_char;
				while(bit_num--);	// draw a horizontal area (rect) in b_color
					fill_rect(x-space_char,x-1,y,y+(f_height*(sy+1))-1,b_color); 
			}
		}
		*str++;
		x+=(f_width+space_char);
	}
}

void draw_char_text_mode(sint x, sint y, uchar character, uint f_color, uint b_color, uchar sx, uchar sy)
{
	// need correction here
	x*=(f_width*(sx+1)+space_char);
	y*=(f_height*(sy+1));
	draw_bmap_8bit_v(x,y,ascii_table[character],f_width,f_height,f_color,b_color,sx,sy);
}

void draw_char(sint x, sint y, uchar character, uint f_color, uint b_color, uchar sx, uchar sy)
{
	draw_bmap_8bit_v(x,y,ascii_table[character],f_width,f_height,f_color,b_color,sx,sy);
}

// modified bresenham algorithm with zoom factor
void draw_line(sint x0, sint y0, sint x1, sint y1, uint color, uchar sx_s, uchar sy_s)
{
	sint dx=get_abs(x1-x0),		sx=x0<x1?1:-1;
   	sint dy=-get_abs(y1-y0), 	sy=y0<y1?1:-1; 
   	sint err=dx+dy, e2; // error value e_xy
 	for(;;)
	{
		draw_pixel_xy(x0*(sx_s+1),y0*(sy_s+1),color,sx_s,sy_s);
    	if (x0==x1 && y0==y1) break;
      	e2=2*err;
      	if(e2>=dy){err+=dy; x0+=sx;} // e_xy+e_x > 0
      	if(e2<=dx){err+=dx; y0+=sy;} // e_xy+e_y < 0
	}
}

void fill_rect(sint x0, sint x1, sint y0, sint y1, uint color)
{
	// swap check maybe
	if(y0>=lcd_height||x0>=lcd_width) return;	// overflow
	if((x0+x1)<0||(y0+y1)<0) return;			// outside display
	if(x0<0) x0=0;						
	if(y0<0) y0=0;
	if(x1>=lcd_width) x1=lcd_width;	
	if(y1>=lcd_height) y1=lcd_height;
	uint x_start=x0;
	while(y0<=y1)
	{
		set_cursor(x0,y0++);
		while(x0++<=x1)
			draw_pixel(color);
		x0=x_start;
	}
} 

void draw_pixel_xy(sint x, sint y, uint color, uchar sx, uchar sy)
{
	// sx=1 and sy=1 => 1*1px = 2*2*px (4px) and so on
	if(sx||sy)
		fill_rect(x,x+sx,y,y+sy,color);
	else 
	{
		if(set_cursor(x,y))
			draw_pixel(color);
	}
}

uint set_RGB16(uchar R, uchar G, uchar B)
{
	return (uint)(R<<11)+(uint)(G<<5)+(uint)B;
}

void draw_pixel(uint color)
{
	set_instruction(1,color>>8);
	set_instruction(1,color&0xFF);	
}

uchar check_xy(sint x, sint y)
{
	if(x<0||x>=lcd_width||y<0||y>=lcd_height) return 0;
	return 1;
}

void fill_display(uint width, uint height, uint color)
{
	set_cursor(0,0);
	while(height--)
	{
		while(width--)
			draw_pixel(color);
		width=lcd_width;
	}
}

void draw_bmap_8bit_v(sint x, sint y, const uchar *bmap, uint width, uint height,
                                uint f_color, uint b_color, uchar sx, uchar sy)
{
    sint x_off=0,y_off=0,x_index=0;
    bit_num=0,byte=0,w_index=0,h_index=0;
    if(x>=lcd_width||y>=lcd_height) return;
    if(y+(sint)height<0) return;
    if(x+(sint)width<0) return;
    if(x<0)
    {
        w_index=x*-1;   // index in bitmap
        x_off=w_index;  // x offset in bitmap
        x=0;
    }
    else x*=(sx+1);     // move size of pixel-size (correction) -> CHANGE!
    byte=height;        // save real height
    if(height%8) height+=8;
    height/=8;
    if(y<0)
    {
        y*=-1;
        y_off=y/8;
        bit_num=y%8;
        height-=y_off;
        y=0;
    }
    else y*=(sy+1);     // move size of pixel-size

    while(h_index<height&&y<lcd_height)
    {
        // vertical bit tester
        while(bit_num<8)
        {
            // horizontal pixel writing
            while(w_index<width&&(x+x_index)<lcd_width)
            {
                // if bit is "1" write a pixel in foreground color
                if(bmap[w_index+((h_index+y_off)*width)]&(1<<bit_num))
                    draw_pixel_xy(x+x_index,y,f_color,sx,sy);
                // if bit is "0" write a pixel in background color
                else if(b_color!=TRANSP)
                    draw_pixel_xy(x+x_index,y,b_color,sx,sy);

                w_index++;
                x_index+=sx+1;
            }
            w_index=x_off;
            x_index=0;
            bit_num++;
            y+=sy+1;
        }
        h_index++;
        w_index=x_off;
        bit_num=0;
    }
}

uchar set_cursor(sint x, sint y)
{
	if(!set_cursor_x(x)||!set_cursor_y(y)) return 0;
	return 1;
}

uchar set_cursor_x(sint x)
{
	if(x<0||x>=lcd_width) return 0;
	set_instruction(0,0x2A);
	set_instruction(1,x>>8);
	set_instruction(1,x&0xFF);
	MEM_WRITE;
	return 1;
}

uchar set_cursor_y(sint y)
{
	if(y<0||y>=lcd_height) return 0;
	set_instruction(0,0x2B);
	set_instruction(1,y>>8);
	set_instruction(1,y&0xFF);
	MEM_WRITE;
	return 1;
}

void set_instruction(uchar register_sel, uchar number)
{
	if(register_sel) 
		P6OUT|=DC;				// data
	else P6OUT&=~DC;			// command
	
	P6OUT&=~CS;					// start condition
	
	P1OUT &= (number & 0x3C);
	P3OUT &= (number & 0xC3);	// escreve nos registradores de dados
	
	P6OUT &= ~WR;				// ativa o latch de escrita
	P6OUT |= WR;				// desativa o latch de escrita
	
	P6OUT |= CS;				// condicao de parada
	
	
}
