#include "OLEDrgb.h"
#include "oled_rgb_char.h"
#include "oled_rgb_font.h"
#include "oled_image.h"

#include "platform_info.h"
#include "ervp_mmio_util.h"
#include "ervp_uart.h"
#include "ervp_printf.h"
#include "ervp_sprintf.h"
#include "ervp_cpg_api.h"
#include "ervp_multicore_synch.h"
#include "ervp_common_peri_group_memorymap.h"
#include "frvp_spi.h"
#include "ervp_uart.h"

unsigned char *pb_oled_rgb_font_cur;
unsigned char *pb_oled_rgb_font_user;
unsigned char oled_rgb_font_user[OLEDRGB_CHARBYTES_USER] = {0};
int dx_co_oled_rgb_font_cur = OLEDRGB_CHARBYTES;
int dy_co_oled_rgb_font_cur = 8;
unsigned short m_font_color, m_font_bk_color;

int oled_x_cursor = 0;
int oled_y_cursor = 0;

const int oled_width = 96;
const int oled_height = 64;

int on_off = 0;
char setting[10];

void oled_start(){
	begin();
	defaults();
	oled_print_page0();
}

void oled_end(){
	end();
}

void defaults(){
	set_font_color(RGB(0,0xFF,0));
	set_font_bk_color(RGB(0,0,0));
	set_current_font_table((unsigned char*) oled_rgb_font);
	set_current_user_font_table(oled_rgb_font_user);
}

void set_cursor(int xch, int ych) {
   if (xch >= oled_width) {
      xch = oled_width - 1;
   }

   if (ych >= oled_height) {
      ych = oled_height - 1;
   }

   oled_x_cursor = xch;
   oled_y_cursor = ych;
}

void advance_cursor() {
   oled_x_cursor += 1;
   if (oled_x_cursor >= oled_width) {
      oled_x_cursor = 0;
      oled_y_cursor += 1;
   }
   if (oled_y_cursor >= oled_height) {
      oled_y_cursor = 0;
   }
   set_cursor(oled_x_cursor, oled_y_cursor);
}

void put_string(char *sz) {
   while (*sz != '\0') {
      draw_glyph(*sz);
      advance_cursor();
      sz += 1;
   }
}

void draw_glyph(char ch) {
   unsigned char *pb_font;
   int ibx, iby, iw, x, y;
   unsigned short rgw_char_bmp[OLEDRGB_CHARBYTES << 4];

   if ((ch & 0x80) != 0) {
      return;
   }

   if (ch < OLEDRGB_USERCHAR_MAX) {
      pb_font = pb_oled_rgb_font_user + ch * OLEDRGB_CHARBYTES;
   } else if ((ch & 0x80) == 0) {
      pb_font = pb_oled_rgb_font_cur + (ch - OLEDRGB_USERCHAR_MAX) * OLEDRGB_CHARBYTES;
   }

   iw = 0;
   for (iby = 0; iby < dy_co_oled_rgb_font_cur; iby++) {
      for (ibx = 0; ibx < dx_co_oled_rgb_font_cur; ibx++) {
         if (pb_font[ibx] & (1 << iby)) {
            // Point in glyph
            rgw_char_bmp[iw] = m_font_color;
         } else {
            // Background
            rgw_char_bmp[iw] = m_font_bk_color;
         }
         iw++;
      }
   }
   x = oled_x_cursor * dx_co_oled_rgb_font_cur;

   y = oled_y_cursor * dy_co_oled_rgb_font_cur;

   DrawBitmap(x, y, x + OLEDRGB_CHARBYTES - 1, y + 7, (unsigned char*) rgw_char_bmp);
}

void set_font_color(unsigned short font_color) {
   m_font_color = font_color;
}

void set_font_bk_color(unsigned short font_color) {
   m_font_bk_color = font_color;
}

void set_current_font_table(unsigned char *pb_font) {
   pb_oled_rgb_font_cur = pb_font;
}

void set_current_user_font_table(unsigned char *pb_user_font) {
	pb_oled_rgb_font_user = pb_user_font;
}

void oled_print_page0(){
   printf("Draw Page0\n");
   DrawBitmap(0, 0, oled_width-1, oled_height-1, hello);
   delay_ms(3000);
}

void oled_print_page1(){
   char receive[5];
   char getc = 0;
   printf("Draw Page1\n");
	Clear();

   while(uart_check_rx_data_ready(0) == 1){
      getc = uart_getc(0);
         if(getc == 'N')
            getc = uart_getc(0);
         else if((getc == '.') || (getc == 'T'))
            break;
         else ;
      sprintf(receive, "%s%c", receive, getc);
   }

	set_font_color(RGB(229,92,209));
	set_cursor(0, 0);
	put_string("A/C : ");
   if(receive[1] == 'n') on_off = 1;
   if(receive[2] == 'f') on_off = 0;
   if(on_off == 1) put_string("ON");
   else{
      put_string("OFF");
   }
	set_font_color(RGB(127,255,0));
	set_cursor(0, 2);
	put_string("Heater : OFF"); // heater doesn't be operated
	set_font_color(RGB(0,0,255));
	set_cursor(0, 4);
	put_string("Setting : ");
	set_cursor(3, 6);
   if(on_off == 0){
      put_string("   OFF");
   }
   else if((receive[1] == 'n')){
      sprintf(setting, "%d", 25);
      put_string(setting);
      put_string(".0 C");
   }
   else if((receive[1] >= '0') && (receive[1] <= '9')){
      sprintf(setting, "%s", receive);
      put_string(setting);
      put_string(".0 C");
   }
   else if(receive[1] == 'K'){
      put_string(setting); 
      put_string(".0 C");
   }
   else{
      put_string(setting);	
      put_string(".0 C");
   } 
}

void oled_print_page2(char s2[]){
	printf("Draw Page2\n");
   Clear();
	set_font_color(RGB(0,255,187));
	set_cursor(0, 3);
	put_string("Temperature");
	set_cursor(1, 5);
   put_string(": ");
	put_string(s2);
	advance_cursor();
	put_string("C");
}
