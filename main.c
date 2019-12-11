#include "OLEDrgb.h"
#include "oled_rgb_char.h"
#include "orvp_max_module.h"

#include "ervp_cpg_api.h"
#include "ervp_multicore_synch.h"
#include "ervp_uart.h"
#include "ervp_printf.h"
#include "ervp_sprintf.h"

int main(){
	float temperature;
	char s1[20], s2[20];
	int grant = request_unique_grant(0,NUM_CORE);
	if(grant == 1){
		printf("Hello\n");

		max30102_start(0x1F, 4, 3, 400, 411, 4096);
		set_led_pulse_amplitude_red(0x0A);
		oled_start();

		while(1){
			max30102_baby_detect(s1); // read red, ir;
			temperature = max30102_read_temperature(); // read temperature
			max30102_send_temperature(temperature, s1, s2); // send temperature
			oled_print_page1(); // show temperature
			delay_ms(2000);
			oled_print_page2(s2); // show A/C, heater, setting
		}
		release_unique_grant(0,NUM_CORE);
	}
	printf("end\n");
	oled_end();
	return 0;
}
