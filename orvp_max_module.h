#ifndef __ORVP_MAX_MODULE_H__
#define __ORVP_MAX_MODULE_H__

#define SERIAL_CLOCK_FREQUENCY	400
#define I2C_BUFFER_LENGTH		32
#define MAX_I2C_ADDRESS 		0x57

// Register Address
#define FIFO_WRITE_POINTER_REG	0x04
#define OVER_FLOW_COUNTER_REG	0x05
#define FIFO_READ_POINTER_REG	0x06
#define FIFO_DATA_REG			0x07
#define FIFO_CONFIGURATION_REG	0x08
#define MODE_CONFIGURATION_REG	0x09
#define SPO2_CONFIGURATION_REG	0x0A
#define MULTI_LED_MODE_CONTROL_REG_1	0x11
#define MULTI_LED_MODE_CONTROL_REG_2	0x12 // not use
#define LED_PULSE_AMPLITUDE_REG_1		0x0C
#define LED_PULSE_AMPLITUDE_REG_2		0x0D
#define DIE_TEMPERATURE_INTEGER_REG		0x1F
#define DIE_TEMPERATURE_FRACTION_REG	0x20
#define DIE_TEMPERATURE_CONFIGURATION_REG	0x21

// FIFO Configuration[B7:B5](0x08)
#define FIFO_SAMPLE_AVERAGE_1	0x00
#define FIFO_SAMPLE_AVERAGE_2	0x20
#define FIFO_SAMPLE_AVERAGE_4	0x40
#define FIFO_SAMPLE_AVERAGE_8	0x60
#define FIFO_SAMPLE_AVERAGE_16	0x80
#define FIFO_SAMPLE_AVERAGE_32	0xA0

// MODE Configuration[B2:B0](0x09)
#define HEART_RATE_MODE			0x02
#define SPO2_MODE				0x03
#define MULTI_LED_MODE			0x07

// SPO2 Configuration[B6:B5](0x0A)
#define SPO2_ADC_RANGE_2048		0x00
#define SPO2_ADC_RANGE_4096		0x20
#define SPO2_ADC_RANGE_8192		0x40
#define SPO2_ADC_RANGE_16384	0x60

// SPO2 Configuration[B4:B2](0x0A)
#define SPO2_SAMPLE_RATE_50		0x00
#define SPO2_SAMPLE_RATE_100	0x04
#define SPO2_SAMPLE_RATE_200	0x08
#define SPO2_SAMPLE_RATE_400	0x0C
#define SPO2_SAMPLE_RATE_800	0x10
#define SPO2_SAMPLE_RATE_1000	0x14
#define SPO2_SAMPLE_RATE_1600	0x18
#define SPO2_SAMPLE_RATE_3200	0x1C

// SPO2 Configuration[B1:B0](0x0A)
#define LED_PULSE_WIDTH_69		0x00
#define LED_PULSE_WIDTH_118		0x01
#define LED_PULSE_WIDTH_215		0x02
#define LED_PULSE_WIDTH_411		0x03

// Multi-LED Mode Control[B6:B4],[B2:B0](0x11-0x12)
#define SLOT_RED_LED			0x01
#define SLOT_IR_LED				0x02

#define STORAGE_SIZE 			4

void max30102_start(unsigned char power_level, int sample_average, int mode, int sample_rate, int pulse_width, int adc_range);
void reset();
void set_fifo_sample_average(int sample_average);
void enable_fifo_rollover();
void set_mode(int mode);
void set_spo2_adc_range(int adc_range);
void set_spo2_sample_rate(int sample_rate);
void set_led_pulse_width(int pulse_width);
void set_led_pulse_amplitude_red(unsigned char power_level);
void set_led_pulse_amplitude_ir(unsigned char power_level);
void bitmask(unsigned char reg, unsigned char mask, int thing);
void clear_fifo();
void enable_led_slot(int slot_number, int device);
void get();
unsigned char get_read_pointer();
unsigned char get_write_pointer();
unsigned int check();
void max30102_baby_detect(char s1[]);
float max30102_read_temperature();
void max30102_send_temperature(float temperature, char s1[], char s2[]);

typedef struct Record{
    unsigned int red[STORAGE_SIZE];
    unsigned int ir[STORAGE_SIZE];
    unsigned char head;
    unsigned char tail;
} sense_struct;
  
sense_struct sense;

#endif
