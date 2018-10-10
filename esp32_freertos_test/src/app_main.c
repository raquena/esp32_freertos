
// IMPORTANT NOTES /////////////////////////////////////////////////////////////

// - ADC2 CAN'T BE USED TOGETHER WITH WIFI, SO VIRTUALLY, ONLY ONE ADC AVAILABLE.
// - THERE'S AN INTERNAL HALL EFFECT SENSOR, IT PREVENTS TO USE D0 WHEN USED.
// - THE I2S PORT CAN BE USED TO GET REGULARLY ADC SAMPLES, AND DMA THEM TO MEMORY
// - THERE'S A LOGGING LIBRARY, WHICH MIGHT BE INTERESTING TO SAVE STATUS INFORMATION BETWEEN RESETS.

// DEBUGGING FLAGS /////////////////////////////////////////////////////////////
#define DEBUG_SERIAL

// first example with the ESP32 and the FreeRTOS

#include <stdio.h>
#include "nvs_flash.h"          // necessary for NON-VOLATILE-STORAGE (config values and so on)
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "driver/adc.h"


// GLOBAL DEFINITIONS //////////////////////////////////////////////////////////

static const adc_channel_t ADC1_CHANNEL = ADC_CHANNEL_6; //GPIO34 if ADC1

// PIN DEFINITION //

static const unsigned char LED_GPIO = 5; // pin where the LED is connected.
#define SHORT_DELAY 100
#define LONG_DELAY SHORT_DELAY*20

// ADC STUFF //

#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES 64 //Multisampling


// GLOBAL VARIABLES, SEMAPOHERS, MAILBOXES /////////////////////////////////////

// improvement: make it a circular buffer, and periodically print the stored values.
unsigned int adcVal = 0;    // value to store the readings of the ADC;



// TASKS ///////////////////////////////////////////////////////////////////////

// task for printing hello world in the serial port //
void hello_task(void *pvParameter){
  while(1){
    printf("Still Alive!\n");
    vTaskDelay(10000 / portTICK_RATE_MS);
  }
}
// read adc task //
void adc_read_task(void){
  while(1){
#ifdef DEBUG_SERIAL
    printf("Reading ADC\n");
#endif
  adcVal = adc1_get_voltage(ADC1_CHANNEL);
  char buffer[30];
  itoa(adcVal,buffer,10);
  printf("ADCval: %s\n",buffer);                 // spit the adc value in UART.
  vTaskDelay(1 / portTICK_RATE_MS);                 // plotting evert 1ms (1000Hz)
  }
}
// BLINK THE INCLUDED LED //

void blinky(void *pvParameter){


    gpio_pad_select_gpio(LED_GPIO); // ??? check the proces out
    // Set the GPIO as a push/pull output.
    // we make the setup everytime we call the task.
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    while(1) {
        // Blink off (output low)
        gpio_set_level(LED_GPIO, 0);
        vTaskDelay(1000 / portTICK_RATE_MS);
        // Blink on (output high)
        gpio_set_level(LED_GPIO, 1);
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}



// MAIN FUNCTION ///////////////////////////////////////////////////////////////

void app_main(){

    nvs_flash_init();   // Initializes non-volatile memory, to read/store config data

    // adc configuration //
    //gives problems adc_power_on();                             // switches on the ADC module.
    adc1_config_width(ADC_WIDTH_BIT_12);        // maximum resolution for the ADC
    adc1_config_channel_atten(ADC1_CHANNEL,ADC_ATTEN_DB_0); // zero attenuation for the adc input (max. Range = 1.1V)
    adc_set_clk_div(1);                          // continue looking for docu, this should set the adc speed.

    // creates task with a given name, 2048 bytes of space, dunno, priority 5, dunno;
    xTaskCreate(&hello_task, "hello_task", 2048, NULL, 5, NULL);
    xTaskCreate(&blinky, "blinky_task", 1024, NULL, 4, NULL);
    // reading the ADC
    xTaskCreate(&adc_read_task,"adc_read_task", 2048,NULL,4,NULL);

}
