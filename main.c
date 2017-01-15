/*
 *
 * raspberrycompote.blogspot.com/2014/04/low-level-graphics-on-raspberry-pi.html
 *
 * Original work by J-P Rosti (a.k.a -rst- and 'Raspberry Compote')
 * Modified with Text Support by Tommy Agustianto (satria.nt@gmail.com)
 *
 * Licensed under the Creative Commons Attribution 3.0 Unported License
 * (http://creativecommons.org/licenses/by/3.0/deed.en_US)
 *
 * Distributed in the hope that this will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include "AsciiLib.h"

#define  MAX_X  320
#define  MAX_Y  240 

char *fbp = 0;
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;

char outstr[200];
time_t t;
struct tm *tmp;
const char* fmt = "%a, %d %b %y %T";

// helper function to 'plot' a pixel in given color
void put_pixel_RGB565(int x, int y, int r, int g, int b)
{
    // remember to change main(): vinfo.bits_per_pixel = 16;
    // or on RPi just comment out the whole 'Change vinfo'
    // and: screensize = vinfo.xres * vinfo.yres * 
    //                   vinfo.bits_per_pixel / 8;

    // calculate the pixel's byte offset inside the buffer
    // note: x * 2 as every pixel is 2 consecutive bytes
    unsigned int pix_offset = x * 2 + y * finfo.line_length;

    // now this is about the same as 'fbp[pix_offset] = value'
    // but a bit more complicated for RGB565
    unsigned short c = ((r / 8) << 11) + ((g / 4) << 5) + (b / 8);
    // or: c = ((r / 8) * 2048) + ((g / 4) * 32) + (b / 8);
    // write 'two bytes at once'
    *((unsigned short*)(fbp + pix_offset)) = c;

}

void blackScreen(void)
{
    int x,y;
    
     for (y = 0; y < vinfo.yres; y++) {
	for(x=0; x<vinfo.xres*2; x++) {
		put_pixel_RGB565(x, y, 0,0,0);
	}
     }
}

void draw_line(int x0, int y0, int x1, int y1, int r, int g, int b) {
    int dx = x1 - x0;
    dx = (dx >= 0) ? dx : -dx; // abs()
    int dy = y1 - y0;
    dy = (dy >= 0) ? dy : -dy; // abs()
    int sx;
    int sy;
    if (x0 < x1)
        sx = 1;
    else
        sx = -1;
    if (y0 < y1)
        sy = 1;
    else
        sy = -1;
    int err = dx - dy;
    int e2;
    int done = 0;
    while (!done) {
        put_pixel_RGB565(x0, y0, r, g, b);
        if ((x0 == x1) && (y0 == y1))
            done = 1;
        else {
            e2 = 2 * err;
            if (e2 > -dy) {
                err = err - dy;
                x0 = x0 + sx;
            }
            if (e2 < dx) {
                err = err + dx;
                y0 = y0 + sy;
            }
        }
    }
}

void draw_rect(int x0, int y0, int w, int h, int r, int g, int b) {
    draw_line(x0, y0, x0 + w, y0, r, g, b); // top
    draw_line(x0, y0, x0, y0 + h, r, g, b); // left
    draw_line(x0, y0 + h, x0 + w, y0 + h, r, g, b); // bottom
    draw_line(x0 + w, y0, x0 + w, y0 + h, r, g, b); // right
}

void PutChar(int Xpos, int Ypos, char ASCI)
{
    int i, j;
    unsigned char buffer[16], tmp_char;
    GetASCIICode(buffer,ASCI);  /* È¡×ÖÄ£Êý¾Ý */
    for( i=0; i<16; i++ )
    {
        tmp_char = buffer[i];
        for( j=0; j<8; j++ )
        {
            if( (tmp_char >> 7 - j) & 0x01 == 0x01 )
            {
                put_pixel_RGB565( Xpos + j, Ypos + i, 255,255,255);  /* ×Ö·ûÑÕÉ« */
            }
            else
            {
                put_pixel_RGB565( Xpos + j, Ypos + i, 0,0,0);  /* ±³¾°ÑÕÉ« */
            }
        }
    }
}



void draw_text(int Xpos, int Ypos, char *str)
{
    int TempChar;
    do
    {
        TempChar = *str++;  
        PutChar( Xpos, Ypos, TempChar);    
        if( Xpos < MAX_X - 8 )
        {
            Xpos += 8;
        } 
        else if ( Ypos < MAX_Y - 16 )
        {
            Xpos = 0;
            Ypos += 16;
        }   
        else
        {
            Xpos = 0;
            Ypos = 0;
        }    
    }
    while ( *str != 0 );
}



// helpe// helper function for drawing - no more need to go mess with
// the main function when just want to change what to draw...
void draw() {

    int x,y;
    unsigned char str[16];

    x = 0;
   
    blackScreen();
    draw_rect(1, 1, 318, 238, 200,200,200);    
    draw_rect(2, 2, 100, 30, 200,0,0);    
    draw_text(10,10,"STAT : ");    
    for (y=0; y<100; y++) {

    	draw_text(50, 10, "OK");
	printf("CNT--\n");
    }
//     some pixels

    // some lines (note the quite likely 'Moire pattern')
   // for (x = 0; x < vinfo.xres; x+=20) {
   //     draw_line(0, 0, x, vinfo.yres - 1, GREEN);
  //  }
    
    // some rectangles
//    draw_rect(vinfo.xres, vinfo.yres - 10, vinfo.xres, vinfo.yres / 2, PURPLE);    
 //   draw_rect(vinfo.xres / 4 + 10, vinfo.yres / 2 + 20, vinfo.xres / 4 - 20, vinfo.yres / 4 - 20, PURPLE);    
 //   fill_rect(vinfo.xres / 4 + 20, vinfo.yres / 2 + 30, vinfo.xres / 4 - 40, vinfo.yres / 4 - 40, YELLOW);    

    // some circles
  //  int d;
  //  for(d = 10; d < vinfo.yres / 6; d+=10) {
  //      draw_circle(3 * vinfo.xres / 4, vinfo.yres / 4, d, RED);
  //  }
    
  //  fill_circle(3 * vinfo.xres / 4, 3 * vinfo.yres / 4, vinfo.yres / 6, ORANGE);
  //  fill_circle(3 * vinfo.xres / 4, 3 * vinfo.yres / 4, vinfo.yres / 8, RED);

}


int main(int argc, char* argv[])
{
    int fbfd = 0;
    struct fb_var_screeninfo orig_vinfo;
    long int screensize = 0;




    // Open the file for reading and writing
    fbfd = open("/dev/fb1", O_RDWR);
    if (fbfd == -1) {
        printf("Error: cannot open framebuffer device.\n");
        return(1);
    }
    printf("The framebuffer device was opened successfully.\n");

    // Get variable screen information
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        printf("Error reading variable information.\n");
    }
    printf("Original %dx%d, %dbpp\n", vinfo.xres, vinfo.yres,
         vinfo.bits_per_pixel );

    // Store for reset (copy vinfo to vinfo_orig)
    memcpy(&orig_vinfo, &vinfo, sizeof(struct fb_var_screeninfo));

    // Change variable info
    vinfo.bits_per_pixel = 8;
    if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &vinfo) == -1) {
        printf("Error setting variable information.\n");
    }

    // Get fixed screen information
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1) {
        printf("Error reading fixed information.\n");
    }

    // map fb to user mem
    screensize = finfo.smem_len;
    fbp = (char*)mmap(0,
                        screensize,
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED,
                        fbfd,
                        0);

    if ((int)fbp == -1) {
        printf("Failed to mmap.\n");
    }
    else {
        // draw...
        draw();
        sleep(5);
    }

    // cleanup
    // unmap fb file from memory
    munmap(fbp, screensize);
    // reset the display mode
    if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &orig_vinfo)) {
        printf("Error re-setting variable information.\n");
    }
    // close fb file    
    close(fbfd);

    return 0;
    
}
