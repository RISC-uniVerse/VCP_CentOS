#include "orvp_max_module.h"

#include "orvp_i2c.h"
#include "ervp_memory_util.h"
#include "ervp_printf.h"
#include "ervp_uart.h"
#include "platform_info.h"
#include "ervp_multicore_synch.h"
#include "ervp_cpg_api.h"

int active_led;
int red, ir;

void max30102_start(unsigned char power_level, int sample_average, int mode, int sample_rate, int pulse_width, int adc_range){
	configure_i2c(0, SERIAL_CLOCK_FREQUENCY, 1, 0);

	reset();

	if(sample_average == 1) set_fifo_sample_average(FIFO_SAMPLE_AVERAGE_1);
	else if(sample_average == 2) set_fifo_sample_average(FIFO_SAMPLE_AVERAGE_2);
	else if(sample_average == 4) set_fifo_sample_average(FIFO_SAMPLE_AVERAGE_4);
	else if(sample_average == 8) set_fifo_sample_average(FIFO_SAMPLE_AVERAGE_8);
	else if(sample_average == 16) set_fifo_sample_average(FIFO_SAMPLE_AVERAGE_16);
	else if(sample_average == 32) set_fifo_sample_average(FIFO_SAMPLE_AVERAGE_32);
	else set_fifo_sample_average(FIFO_SAMPLE_AVERAGE_4);
	
	enable_fifo_rollover();

	if(mode == 2) set_mode(SPO2_MODE);
	else if(mode == 3) set_mode(MULTI_LED_MODE);
	else set_mode(HEART_RATE_MODE);

	if(adc_range < 4096) set_spo2_adc_range(SPO2_ADC_RANGE_2048);
	else if(adc_range < 8192) set_spo2_adc_range(SPO2_ADC_RANGE_4096);
	else if(adc_range < 16384) set_spo2_adc_range(SPO2_ADC_RANGE_8192);
	else if(adc_range == 16384) set_spo2_adc_range(SPO2_ADC_RANGE_16384);
	else set_spo2_adc_range(SPO2_ADC_RANGE_2048);

	if(sample_rate < 100) set_spo2_sample_rate(SPO2_SAMPLE_RATE_50);
	else if(sample_rate < 200) set_spo2_sample_rate(SPO2_SAMPLE_RATE_100);
	else if(sample_rate < 400) set_spo2_sample_rate(SPO2_SAMPLE_RATE_200);
	else if(sample_rate < 800) set_spo2_sample_rate(SPO2_SAMPLE_RATE_400);
	else if(sample_rate < 1000) set_spo2_sample_rate(SPO2_SAMPLE_RATE_800);
	else if(sample_rate < 1600) set_spo2_sample_rate(SPO2_SAMPLE_RATE_1000);
	else if(sample_rate < 3200) set_spo2_sample_rate(SPO2_SAMPLE_RATE_1600);
	else if(sample_rate == 3200) set_spo2_sample_rate(SPO2_SAMPLE_RATE_3200);
	else set_spo2_sample_rate(SPO2_SAMPLE_RATE_50);

	if(pulse_width < 118) set_led_pulse_width(LED_PULSE_WIDTH_69);
	else if(pulse_width < 215) set_led_pulse_width(LED_PULSE_WIDTH_118);
	else if(pulse_width < 411) set_led_pulse_width(LED_PULSE_WIDTH_215);
	else if(pulse_width == 411) set_led_pulse_width(LED_PULSE_WIDTH_411);
	else set_led_pulse_width(LED_PULSE_WIDTH_69);

	set_led_pulse_amplitude_red(power_level);
	set_led_pulse_amplitude_ir(power_level);

	if(mode > 1){
		enable_led_slot(1, SLOT_RED_LED);
		enable_led_slot(2, SLOT_IR_LED);	
		active_led = 2;
	}
	else{
		enable_led_slot(1, SLOT_RED_LED);
		active_led = 1;
	}

	clear_fifo();
}

void reset(){
	unsigned char data[1];
	data[0] = 0x00;
	bitmask(MODE_CONFIGURATION_REG, 0xBF, 0x40);
	while(1){
		read_i2c_r1dn(0, MAX_I2C_ADDRESS, MODE_CONFIGURATION_REG, data, 1);
		data[0] = data[0] & 0x40;
		if(data[0] == 0x00) break;
	}
}

void set_fifo_sample_average(int sample_average){
	bitmask(FIFO_CONFIGURATION_REG, 0x1F, sample_average);
}

void enable_fifo_rollover(){
	bitmask(FIFO_CONFIGURATION_REG, 0xEF, 0x10);
}

void set_mode(int mode){
	bitmask(MODE_CONFIGURATION_REG, 0xF8, mode);
}

void set_spo2_adc_range(int adc_range){
	bitmask(SPO2_CONFIGURATION_REG, 0x9F, adc_range);
}

void set_spo2_sample_rate(int sample_rate){
	bitmask(SPO2_CONFIGURATION_REG, 0xE3, sample_rate);
}

void set_led_pulse_width(int pulse_width){
	bitmask(SPO2_CONFIGURATION_REG, 0xFC, pulse_width);
}

void set_led_pulse_amplitude_red(unsigned char power_level){
	unsigned char data[1];
	data[0] = power_level;
	write_i2c_r1dn(0, MAX_I2C_ADDRESS, LED_PULSE_AMPLITUDE_REG_1, data, 1);
}

void set_led_pulse_amplitude_ir(unsigned char power_level){
	unsigned char data[1];
	data[0] = power_level;
	write_i2c_r1dn(0, MAX_I2C_ADDRESS, LED_PULSE_AMPLITUDE_REG_2, data, 1);
}

void bitmask(unsigned char reg, unsigned char mask, int thing){
	unsigned char data[1];
	data[0] = 0x00;
	read_i2c_r1dn(0, MAX_I2C_ADDRESS, reg, data, 1);
	data[0] = data[0] & mask;
	data[0] = data[0] | (unsigned char) thing;
	write_i2c_r1dn(0, MAX_I2C_ADDRESS, reg, data, 1);
}

void clear_fifo(){
	unsigned char data[1];
	data[0] = 0x00;
	write_i2c_r1dn(0, MAX_I2C_ADDRESS, FIFO_WRITE_POINTER_REG, data, 1);
	write_i2c_r1dn(0, MAX_I2C_ADDRESS, OVER_FLOW_COUNTER_REG, data, 1);
	write_i2c_r1dn(0, MAX_I2C_ADDRESS, FIFO_READ_POINTER_REG, data, 1);
}

void enable_led_slot(int slot_number, int device){
	switch (slot_number) {
		case (1):
		bitmask(MULTI_LED_MODE_CONTROL_REG_1, 0xF8, device);
		break;
		case (2):
		bitmask(MULTI_LED_MODE_CONTROL_REG_1, 0x8F, device << 4);
		break;
		default:
		break;
	}
}

void get(){
	if(check() > 0){
		red = sense.red[sense.head];
		ir = sense.ir[sense.head];
	}
	else{
		red = sense.red[(sense.head+3)%4];
		ir = sense.ir[(sense.head+3)%4];
	}
}

unsigned char get_read_pointer(){
	unsigned char data[1];
	data[0] = 0x00;
	read_i2c_r1dn(0, MAX_I2C_ADDRESS, FIFO_READ_POINTER_REG, data, 1);
	return data[0];
}

unsigned char get_write_pointer(){
	unsigned char data[1];
	data[0] = 0x00;
	read_i2c_r1dn(0, MAX_I2C_ADDRESS, FIFO_WRITE_POINTER_REG, data, 1);
	return data[0];
}

unsigned int check(){
    unsigned char read_pointer = get_read_pointer();
    unsigned char write_pointer = get_write_pointer();
    int number_of_samples = 0;
    int bytes_left_to_read;
    int to_get;
    int to_get_store;
    int i;
    int j = 0;
    unsigned char temp_data[I2C_BUFFER_LENGTH];
    unsigned char temp[STORAGE_SIZE];
    unsigned int temp_long;
   
    if(read_pointer != write_pointer){
       number_of_samples = write_pointer - read_pointer;
       if(number_of_samples < 0) number_of_samples += 32;
       bytes_left_to_read = number_of_samples * active_led * 3;
	   for(i=0; i<I2C_BUFFER_LENGTH; i++){
	   	temp_data[i] = 0x00;
	   }
	   for(i=0; i<4; i++){
		temp[i] = 0x00;
	   }
       while (bytes_left_to_read > 0){
      	to_get = bytes_left_to_read;
      	if (to_get > I2C_BUFFER_LENGTH) to_get = I2C_BUFFER_LENGTH - (I2C_BUFFER_LENGTH % (active_led * 3)); // to_get = 30
      		while(bytes_left_to_read > 0){
      			bytes_left_to_read -= to_get;
      			read_i2c_r1dn(0, MAX_I2C_ADDRESS, FIFO_DATA_REG, temp_data, to_get);
      		}
	    to_get_store = to_get;

        while (to_get > 0){
		    sense.head++;
			sense.head %= STORAGE_SIZE; 

			for(i=2; i>=0; i--){
				temp[i] = temp_data[j++];
			}

			memcpy(&temp_long, temp, sizeof(temp_long));
			temp_long &= 0x3FFFF;
			sense.red[sense.head] = temp_long;
		  
			if (active_led > 1){ // IR
				for(i=2; i>=0; i--){
				 temp[i] = temp_data[j++];
			}

			memcpy(&temp_long, temp, sizeof(temp_long));
			temp_long &= 0x3FFFF;
			sense.ir[sense.head] = temp_long;
		  }
		  if(j == to_get_store) j = 0;
		  to_get -= active_led * 3;
		}
	  }
	}
   return number_of_samples;
}

void max30102_baby_detect(char s1[]){
	printf("Baby Detect\n");
	get();
	printf("R[%d] IR[%d]\n", red, ir);
	if(ir >= 50000)	sprintf(s1, "%s", "NNYNN");
	else sprintf(s1, "%s", "NNNNN");
}

float max30102_read_temperature(){
	printf("Read Temperature\n");
	float temperature, tempint, tempfraction;
	unsigned char data[1], temp_enable[1];
	temperature = 0.0;
	data[0] = 0x00;
	temp_enable[0] = 0x01;

	write_i2c_r1dn(0, MAX_I2C_ADDRESS, DIE_TEMPERATURE_CONFIGURATION_REG, temp_enable, 1);
	read_i2c_r1dn(0, MAX_I2C_ADDRESS, DIE_TEMPERATURE_INTEGER_REG, data, 1);
	tempint = data[0];
	read_i2c_r1dn(0, MAX_I2C_ADDRESS, DIE_TEMPERATURE_FRACTION_REG, data, 1);
	tempfraction = data[0];
	temperature = tempint + (tempfraction * 0.0625);

	return temperature;
}

void max30102_send_temperature(float temperature, char s1[], char s2[]){
	printf("Send Information\n");
	char uart_data[100];
	if((int)temperature >= 0){
		printf("Temperature = +%d.%d C\n", (int)temperature, (int)((temperature-(int)temperature)*1000));
		sprintf(s2, "+%d.%d", (int)temperature, (int)((temperature-(int)temperature)*1000));
		sprintf(uart_data, "%s%s", s1, s2);
	}
	else{
		sprintf(s2, "-%d.%d C\n", (int)temperature, (int)((temperature-(int)temperature)*1000));
		sprintf(uart_data, "%s%s", s1, s2);
	}
	uart_puts(0, uart_data);
}