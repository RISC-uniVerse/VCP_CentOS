#include "OLEDrgb.h"

#include "platform_info.h"
#include "ervp_mmio_util.h"
#include "ervp_uart.h"
#include "ervp_printf.h"
#include "ervp_cpg_api.h"
#include "ervp_multicore_synch.h"
#include "ervp_common_peri_group_memorymap.h"
#include "frvp_spi.h"

#ifndef SPI_BASEADDR
#define SPI_BASEADDR        (COMMON_PERI_GROUP_BASEADDR+BASEADDR_COMMON_PERI_GROUP_SPI_GROUP0)
#endif

#define SPI_FREQ_OF_OLED        5000000
//#define SPI_FREQ_OF_OLED        8000000
#define SPI_MODE_OF_OLED        SPI_SCKMODE_3

#define OLED_CTRL_DC_SEL_ADDR           (COMMON_PERI_GROUP_BASEADDR+MMAP_OLED_CTRL_DC_SEL)
#define OLED_CTRL_RESET_ADDR            (COMMON_PERI_GROUP_BASEADDR+MMAP_OLED_CTRL_RESET)
#define OLED_CTRL_VCCEN_ADDR             (COMMON_PERI_GROUP_BASEADDR+MMAP_OLED_CTRL_VBAT)
#define OLED_CTRL_PMODEN_ADDR              (COMMON_PERI_GROUP_BASEADDR+MMAP_OLED_CTRL_VDD)

#define NULL	0

unsigned char cmds[13];

unsigned short BuildRGB(unsigned char R,unsigned char G,unsigned char B){return ((R>>3)<<11) | ((G>>2)<<5) | (B>>3);};     
unsigned char ExtractRFromRGB(unsigned short wRGB){return (unsigned char)((wRGB>>11)&0x1F);};
unsigned char ExtractGFromRGB(unsigned short wRGB){return (unsigned char)((wRGB>>5)&0x3F);};
unsigned char ExtractBFromRGB(unsigned short wRGB){return (unsigned char)(wRGB&0x1F);};

void OledrgbDevInit();
void OledrgbHostTerm();
void OledrgbHostInit();

void write_cmd1(unsigned char bVal);
void write_cmd2(unsigned char bVal1, unsigned char bVal2);
void write_cmd4(unsigned char *pCmd, int nCmd, unsigned char *pData, int nData);

void begin(void)
{
	OledrgbHostInit();
	OledrgbDevInit();
}

void end()
{
	OledrgbHostTerm();
}

void OledrgbHostInit()
{
	configure_spi(SPI_BASEADDR, SPI_FREQ_OF_OLED, SPI_MODE_OF_OLED, SPI_INDEX_FOR_OLED);

	REG32(OLED_CTRL_DC_SEL_ADDR) = 0;
	REG32(OLED_CTRL_RESET_ADDR) = 1;
	REG32(OLED_CTRL_VCCEN_ADDR) = 0;
	REG32(OLED_CTRL_PMODEN_ADDR) = 1;
}

void OledrgbHostTerm()
	{

	// Make the signal pins be inputs.
	REG32(OLED_CTRL_DC_SEL_ADDR) = 1;
	REG32(OLED_CTRL_RESET_ADDR) = 1;

	// Make power control pins be inputs. 
	REG32(OLED_CTRL_VCCEN_ADDR) = 0;
	REG32(OLED_CTRL_PMODEN_ADDR) = 1;
}

#define DELAY_TIME_MS	1000
void OledrgbDevInit()
	{	
		/* 
		Bring PmodEn HIGH
	*/

	REG32(OLED_CTRL_PMODEN_ADDR) = 1;
//	delay(10);
	delay_us(100000);
	//delay(DELAY_TIME_MS);
	
	REG32(OLED_CTRL_RESET_ADDR) = 1;
	delay_us(100);
	//delay(DELAY_TIME_MS);
	
	/* command un-lock
	*/
	write_cmd1(0xFD);
	write_cmd1(0x12);

	/* 5. Univision Initialization Steps
	*/

	// 5a) Set Display Off
	write_cmd1(CMD_DISPLAYOFF);
	
    	// 5b) Set Remap and Data Format
	write_cmd2(CMD_SETREMAP, 0x72);
	
	// 5c) Set Display Start Line
	write_cmd2(CMD_SETDISPLAYSTARTLINE, 0x00);

	// 5d) Set Display Offset
	write_cmd2(CMD_SETDISPLAYOFFSET, 0x00); //no offset

    	// 5e)
    	write_cmd1(CMD_NORMALDISPLAY);

	// 5f) Set Multiplex Ratio
	write_cmd2(CMD_SETMULTIPLEXRATIO, 0x3F); //64MUX    

	// 5g)Set Master Configuration
	write_cmd2(CMD_SETMASTERCONFIGURE, 0x8E);

	// 5h)Set Power Saving Mode
	write_cmd2(CMD_POWERSAVEMODE, 0x0B);    

	// 5i) Set Phase Length
	write_cmd2(CMD_PHASEPERIODADJUSTMENT, 0x31); //phase 2 = 14 DCLKs, phase 1 = 15 DCLKS            

	// 5j) Send Clock Divide Ratio and Oscillator Frequency
	write_cmd2(CMD_DISPLAYCLOCKDIV, 0xF0); //mid high oscillator frequency, DCLK = FpbCllk/2

	// 5k) Set Second Pre-charge Speed of Color A
	write_cmd2(CMD_SETPRECHARGESPEEDA, 0x64); //Set Second Pre-change Speed For ColorA

	// 5l) Set Set Second Pre-charge Speed of Color B
	write_cmd2(CMD_SETPRECHARGESPEEDB, 0x78); //Set Second Pre-change Speed For ColorB

	// 5m) Set Second Pre-charge Speed of Color C
	write_cmd2(CMD_SETPRECHARGESPEEDC, 0x64); //Set Second Pre-change Speed For ColorC

	// 5n) Set Pre-Charge Voltage
	write_cmd2(CMD_SETPRECHARGEVOLTAGE, 0x3A); // Pre-charge voltage =...Vcc    

	// 50) Set VCOMH Deselect Level
	write_cmd2(CMD_SETVVOLTAGE, 0x3E); // Vcomh = ...*Vcc

	// 5p) Set Master Current
	write_cmd2(CMD_MASTERCURRENTCONTROL, 0x06); 

	// 5q) Set Contrast for Color A
	write_cmd2(CMD_SETCONTRASTA, 0x91); //Set contrast for color A

	// 5r) Set Contrast for Color B
	write_cmd2(CMD_SETCONTRASTB, 0x50); //Set contrast for color B

	// 5s) Set Contrast for Color C
	write_cmd2(CMD_SETCONTRASTC, 0x7D); //Set contrast for color C

	write_cmd1(CMD_DEACTIVESCROLLING);   //disable scrolling
	
	// 5t) Set display ON

	// 5u) Clear Screen
	Clear();

	/* Turn on VCCEN and wait 25ms
	*/
	REG32(OLED_CTRL_VCCEN_ADDR) = 1;
	delay_us(100000);
	//delay(DELAY_TIME_MS);

	/* Send Display On command
	*/
	write_cmd1(CMD_DISPLAYON);

	delay_us(1000000);
	//delay_us(30000);
	//delay(300);
}

void Clear()
{
	cmds[0] = CMD_CLEARWINDOW; 		// Enter the “clear mode”
	cmds[1] = 0x00;					// Set the starting column coordinates
	cmds[2] = 0x00;					// Set the starting row coordinates
	cmds[3] = OLEDRGB_WIDTH - 1;	// Set the finishing column coordinates;
	cmds[4] = OLEDRGB_HEIGHT - 1;	// Set the finishing row coordinates;
	write_cmd4(cmds, 5, NULL, 0);
	delay_us(500);
	//delay(5);
}

void DrawRectangle(unsigned char c1, unsigned char r1, unsigned char c2, unsigned char r2, unsigned short lineColor, unsigned char bFill, unsigned short fillColor)
{
	cmds[0] = CMD_FILLWINDOW;		//fill window
	cmds[1] = (bFill ? ENABLE_FILL: DISABLE_FILL);
	cmds[2] = CMD_DRAWRECTANGLE;	//draw rectangle
	cmds[3] = c1;					// start column
	cmds[4] = r1;					// start row
	cmds[5] = c2;					// end column
	cmds[6] = r2;					//end row
	
	cmds[7] = ExtractRFromRGB(lineColor);	//R				
	cmds[8] = ExtractGFromRGB(lineColor);	//G
	cmds[9] = ExtractBFromRGB(lineColor);	//B
	
	
	if(bFill)
	{
	        cmds[10] = ExtractRFromRGB(fillColor);	//R			
	        cmds[11] = ExtractGFromRGB(fillColor);	//G
	        cmds[12] = ExtractBFromRGB(fillColor);	//B
	}
	else
	{
	        cmds[10] = 0;	//R			
	        cmds[11] = 0;	//G
	        cmds[12] = 0;	//B
	}
	//write_cmd4(cmds, bFill ? 13: 10, NULL, 0);
	write_cmd4(cmds, 13, NULL, 0);
	delay_us(500);
	//delay(5);
}

void DrawLine(unsigned char c1, unsigned char r1, unsigned char c2, unsigned char r2, unsigned short lineColor)
{
	cmds[0] = CMD_DRAWLINE; 		//draw line
	cmds[1] = c1;					// start column
	cmds[2] = r1;					// start row
	cmds[3] = c2;					// end column
	cmds[4] = r2;					//end row
	cmds[5] = ExtractRFromRGB(lineColor);	//R					
	cmds[6] = ExtractGFromRGB(lineColor);	//G
	cmds[7] = ExtractBFromRGB(lineColor);	//R
	
	write_cmd4(cmds, 8, NULL, 0);
	delay_us(500);
	//delay(5);	
}

void DrawPixel(unsigned char c, unsigned char r, unsigned short pixelColor)
{
#if 1
	DrawRectangle(c, r, c, r, pixelColor, 0, 0);
#else
/*
	cmds[0] = CMD_DRAWRECTANGLE;	//draw rectangle
	cmds[1] = c;					// start column
	cmds[2] = r;					// start row
	cmds[3] = c;					// end column
	cmds[4] = r;					//end row

	cmds[5] = ExtractRFromRGB(pixelColor);	//R					
	cmds[6] = ExtractGFromRGB(pixelColor);	//G
	cmds[7] = ExtractBFromRGB(pixelColor);	//R
	
	write_cmd4(cmds, 8, NULL, 0);
	delay_us(500);
	//delay(5);
*/
	unsigned char data[2];
	//set column start and end
	cmds[0] = CMD_SETCOLUMNADDRESS; 		
	cmds[1] = c;					// Set the starting column coordinates
	cmds[2] = OLEDRGB_WIDTH - 1;					// Set the finishing column coordinates

	//set row start and end
	cmds[3] = CMD_SETROWADDRESS; 		
	cmds[4] = r;					// Set the starting row coordinates
	cmds[5] = OLEDRGB_HEIGHT - 1;					// Set the finishing row coordinates

	data[0] = pixelColor >> 8;
	data[1] = pixelColor;
	
	write_cmd4(cmds, 6, data, 2);
	delay_us(500);
	//delay(5);	
#endif	
}

void DrawBitmap(unsigned char c1, unsigned char r1, unsigned char c2, unsigned char r2, unsigned char *pBmp)
{
	//set column start and end
	cmds[0] = CMD_SETCOLUMNADDRESS; 		
	cmds[1] = c1;			// Set the starting column coordinates
	cmds[2] = c2;			// Set the finishing column coordinates

	//set row start and end
	cmds[3] = CMD_SETROWADDRESS; 		
	cmds[4] = r1;			// Set the starting row coordinates
	cmds[5] = r2;			// Set the finishing row coordinates

	write_cmd4(cmds, 6, pBmp, (((c2 - c1 + 1)  * (r2 - r1 + 1)) << 1));
	delay_us(500);
	//delay(5);
}

void Copy(unsigned char c1, unsigned char r1, unsigned char c2, unsigned char r2, unsigned char c_new, unsigned char r_new)
{
	cmds[0] = CMD_COPYWINDOW;
	cmds[1] = c1;
	cmds[2] = r1;
	cmds[3] = c2;
	cmds[4] = r2;
	cmds[5] = c_new;
	cmds[6] = r_new;

	write_cmd4(cmds, 7, NULL, 0);
	delay_us(500);
	//delay(5);	
}

void DimWindow(unsigned char c1, unsigned char r1, unsigned char c2, unsigned char r2)
{
	cmds[0] = CMD_DIMWINDOW;
	cmds[1] = c1;
	cmds[2] = r1;
	cmds[3] = c2;
	cmds[4] = r2;

	write_cmd4(cmds, 5, NULL, 0);
	delay_us(500);
	//delay(5);	
}

void StartScrolling(unsigned char c_shift_size, unsigned char start_row_addr, unsigned char r_length, unsigned char r_shift_size, unsigned char time_interval)
{
	cmds[0] = CMD_CONTINUOUSSCROLLINGSETUP; 		
	cmds[1] = c_shift_size;
	cmds[2] = start_row_addr;
	cmds[3] = r_length;
	cmds[4] = r_shift_size;
	cmds[5] = time_interval;
	cmds[6] = CMD_ACTIVESCROLLING;

	write_cmd4(cmds, 7, NULL, 0);
	delay_us(500);
	//delay(5);	
}

void StopScrolling()
{
	cmds[0] = CMD_DEACTIVESCROLLING; 		
	write_cmd4(cmds, 1, NULL, 0);
	delay_us(500);
	//delay(5);	
}

void write_cmd1(unsigned char bVal)
{
	write_spi(SPI_BASEADDR, SPI_INDEX_FOR_OLED, 1, &bVal);
}

void write_cmd2(unsigned char bVal1, unsigned char bVal2)
{
	unsigned char cmd[2];
	cmd[0] = bVal1;
	cmd[1] = bVal2;

	write_spi(SPI_BASEADDR, SPI_INDEX_FOR_OLED, 2, cmd);
}
void write_cmd4(unsigned char *pCmd, int nCmd, unsigned char *pData, int nData)
{
	enable_spi(SPI_BASEADDR, SPI_INDEX_FOR_OLED);
	__write_spi(SPI_BASEADDR, SPI_INDEX_FOR_OLED, nCmd, pCmd);

	if(pData != NULL)
	{
		REG32(OLED_CTRL_DC_SEL_ADDR) = 1;
		__write_spi(SPI_BASEADDR, SPI_INDEX_FOR_OLED, nData, pData);
		REG32(OLED_CTRL_DC_SEL_ADDR) = 0;
	}
	disable_spi(SPI_BASEADDR, SPI_INDEX_FOR_OLED);
}

