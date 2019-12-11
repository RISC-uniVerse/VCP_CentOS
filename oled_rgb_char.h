#ifndef OLED_RGB_CHAR_H
#define OLED_RGB_CHAR_H

#define OLEDRGB_CHARBYTES    8    // Number of bytes in a glyph
#define OLEDRGB_USERCHAR_MAX 0x20 // Number of character defs in user font table
#define OLEDRGB_CHARBYTES_USER (OLEDRGB_USERCHAR_MAX*OLEDRGB_CHARBYTES) // Number of bytes in user font table

void oled_start();
void oled_end();
void defaults();
void set_cursor(int xch, int ych);
void advance_cursor();
void put_string(char *sz);
void draw_glyph(char ch);
void set_font_color(unsigned short font_color);
void set_font_bk_color(unsigned short font_color);
void set_current_font_table(unsigned char *pb_font);
void set_current_user_font_table(unsigned char *pb_user_font);
void oled_print_page0();
void oled_print_page1();
void oled_print_page2(char s2[]);

#endif
