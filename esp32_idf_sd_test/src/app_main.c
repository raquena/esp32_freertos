

#include <stdio.h>
#include <string.h>
#include "driver/i2c.h"


// errors and logs //

#include "esp_err.h"
#include "esp_log.h"

// sd card libraries //

#include "esp_vfs.h"
#include "esp_vfs_fat.h"

#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"


// gpio //
#define SQUARE_SIGNAL_PIN                   14    // used to generate a square signal. to check i2c's protocol
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<SQUARE_SIGNAL_PIN) | (1ULL<<SQUARE_SIGNAL_PIN))

// variables
static const char *TAG = "example";

// This example can use SDMMC and SPI peripherals to communicate with SD card.
// By default, SDMMC peripheral is used.
// To enable SPI mode, uncomment the following line:

#define USE_SPI_MODE

// When testing SD and SPI modes, keep in mind that once the card has been
// initialized in SPI mode, it can not be reinitialized in SD mode without
// toggling power to the card.

#ifdef USE_SPI_MODE
// Pin mapping when using SPI mode.
// With this mapping, SD card can be used both in SPI and 1-line SD mode.
// Note that a pull-up on CS line is required in SD mode.
#define PIN_NUM_MISO 19
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  18
#define PIN_NUM_CS   5
#endif //USE_SPI_MODE


// i2c //

static void square_gpio_init(){

  gpio_config_t io_conf;          // varaible containing configuration of specific pin.

  //disable interrupt
  io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
  //set as output mode
  io_conf.mode = GPIO_MODE_OUTPUT;
  //bit mask of the pins that you want to set,e.g.GPIO18/19
  io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
  //disable pull-down mode
  io_conf.pull_down_en = 1;
  //disable pull-up mode
  io_conf.pull_up_en = 0;
  //configure GPIO with the given settings
   gpio_config(&io_conf);
}


TaskHandle_t pin_toggle_handle;
static void pin_toggle_task(){

  //square_gpio_init();

  static uint32_t count = 0; // simply used to toggle the IO pin.


  while(1){
    printf("Toggling\n");
    gpio_set_level(SQUARE_SIGNAL_PIN, 1);
    count +=1;  // increase count
    count = count%2;
    vTaskDelay(1);
    gpio_set_level(SQUARE_SIGNAL_PIN, 0);
    vTaskDelay(1);
  }
}


static void sd_card_init(){

    // SD CARD INIT STUFF //

  #ifndef USE_SPI_MODE
      ESP_LOGI(TAG, "Using SDMMC peripheral");
      sdmmc_host_t host = SDMMC_HOST_DEFAULT();

      // This initializes the slot without card detect (CD) and write protect (WP) signals.
      // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
      sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

      // To use 1-line SD mode, uncomment the following line:
      // slot_config.width = 1;

      // GPIOs 15, 2, 4, 12, 13 should have external 10k pull-ups.
      // Internal pull-ups are not sufficient. However, enabling internal pull-ups
      // does make a difference some boards, so we do that here.
      gpio_set_pull_mode(15, GPIO_PULLUP_ONLY);   // CMD, needed in 4- and 1- line modes
      gpio_set_pull_mode(2, GPIO_PULLUP_ONLY);    // D0, needed in 4- and 1-line modes
      gpio_set_pull_mode(4, GPIO_PULLUP_ONLY);    // D1, needed in 4-line mode only
      gpio_set_pull_mode(12, GPIO_PULLUP_ONLY);   // D2, needed in 4-line mode only
      gpio_set_pull_mode(13, GPIO_PULLUP_ONLY);   // D3, needed in 4- and 1-line modes

  #else
  ESP_LOGI(TAG, "Using SPI peripheral");

  sdmmc_host_t host = SDSPI_HOST_DEFAULT();
  sdspi_slot_config_t slot_config = SDSPI_SLOT_CONFIG_DEFAULT();
  slot_config.gpio_miso = PIN_NUM_MISO;
  slot_config.gpio_mosi = PIN_NUM_MOSI;
  slot_config.gpio_sck  = PIN_NUM_CLK;
  slot_config.gpio_cs   = PIN_NUM_CS;
  // This initializes the slot without card detect (CD) and write protect (WP) signals.
  // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
  #endif //USE_SPI_MODE

  // Options for mounting the filesystem.
  // If format_if_mount_failed is set to true, SD card will be partitioned and
  // formatted in case when mounting fails.
  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
      .format_if_mount_failed = false,
      .max_files = 5,
      .allocation_unit_size = 16 * 1024
  };

  // Use settings defined above to initialize SD card and mount FAT filesystem.
  // Note: esp_vfs_fat_sdmmc_mount is an all-in-one convenience function.
  // Please check its source code and implement error recovery when developing
  // production applications.
  sdmmc_card_t* card;
  esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);

  if (ret != ESP_OK) {
      if (ret == ESP_FAIL) {
          ESP_LOGE(TAG, "Failed to mount filesystem. "
              "If you want the card to be formatted, set format_if_mount_failed = true.");
      } else {
          ESP_LOGE(TAG, "Failed to initialize the card (%s). "
              "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
      }
      return;
  }


  // Card has been initialized, print its properties
     sdmmc_card_print_info(stdout, card);

     // Use POSIX and C standard library functions to work with files.
     // First create a file.
     ESP_LOGI(TAG, "Opening file");
     FILE* f = fopen("/sdcard/hello.txt", "w");
     if (f == NULL) {
         ESP_LOGE(TAG, "Failed to open file for writing");
         return;
     }
     fprintf(f, "Hello %s!\n", card->cid.name);
     fclose(f);
     ESP_LOGI(TAG, "File written");

     // Check if destination file exists before renaming
     struct stat st;
     if (stat("/sdcard/foo.txt", &st) == 0) {
         // Delete it if it exists
         unlink("/sdcard/foo.txt");
     }


     // Rename original file
     ESP_LOGI(TAG, "Renaming file");
     if (rename("/sdcard/hello.txt", "/sdcard/foo.txt") != 0) {
         ESP_LOGE(TAG, "Rename failed");
         return;
     }

     // Open renamed file for reading
     ESP_LOGI(TAG, "Reading file");
     f = fopen("/sdcard/foo.txt", "r");
     if (f == NULL) {
         ESP_LOGE(TAG, "Failed to open file for reading");
         return;
     }
     char line[64];
     fgets(line, sizeof(line), f);
     fclose(f);
     // strip newline
     char* pos = strchr(line, '\n');
     if (pos) {
         *pos = '\0';
     }
     ESP_LOGI(TAG, "Read from file: '%s'", line);

     // All done, unmount partition and disable SDMMC or SPI peripheral
     esp_vfs_fat_sdmmc_unmount();
     ESP_LOGI(TAG, "Card unmounted");


}
TaskHandle_t sd_task_handle;
static void sd_task() {
  printf("sd task\n");

  sd_card_init();
  const uint32_t imax = 10;
  const uint32_t jmax = 1000;
  const uint32_t kmax = 1000;

  const uint32_t delay = 1;

  static uint32_t i;
  static uint32_t j;
  static uint32_t k;
  static uint32_t n;

  static uint32_t fnum = 0;


    // SD CARD INIT STUFF //

  #ifndef USE_SPI_MODE
      ESP_LOGI(TAG, "Using SDMMC peripheral");
      sdmmc_host_t host = SDMMC_HOST_DEFAULT();

      // This initializes the slot without card detect (CD) and write protect (WP) signals.
      // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
      sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

      // To use 1-line SD mode, uncomment the following line:
      // slot_config.width = 1;

      // GPIOs 15, 2, 4, 12, 13 should have external 10k pull-ups.
      // Internal pull-ups are not sufficient. However, enabling internal pull-ups
      // does make a difference some boards, so we do that here.
      gpio_set_pull_mode(15, GPIO_PULLUP_ONLY);   // CMD, needed in 4- and 1- line modes
      gpio_set_pull_mode(2, GPIO_PULLUP_ONLY);    // D0, needed in 4- and 1-line modes
      gpio_set_pull_mode(4, GPIO_PULLUP_ONLY);    // D1, needed in 4-line mode only
      gpio_set_pull_mode(12, GPIO_PULLUP_ONLY);   // D2, needed in 4-line mode only
      gpio_set_pull_mode(13, GPIO_PULLUP_ONLY);   // D3, needed in 4- and 1-line modes

  #else
  ESP_LOGI(TAG, "Using SPI peripheral");

  sdmmc_host_t host = SDSPI_HOST_DEFAULT();
  sdspi_slot_config_t slot_config = SDSPI_SLOT_CONFIG_DEFAULT();
  slot_config.gpio_miso = PIN_NUM_MISO;
  slot_config.gpio_mosi = PIN_NUM_MOSI;
  slot_config.gpio_sck  = PIN_NUM_CLK;
  slot_config.gpio_cs   = PIN_NUM_CS;
  // This initializes the slot without card detect (CD) and write protect (WP) signals.
  // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
  #endif //USE_SPI_MODE

  // Options for mounting the filesystem.
  // If format_if_mount_failed is set to true, SD card will be partitioned and
  // formatted in case when mounting fails.
  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
      .format_if_mount_failed = false,
      .max_files = 10,
      .allocation_unit_size = 16 * 1024
  };

  // Use settings defined above to initialize SD card and mount FAT filesystem.
  // Note: esp_vfs_fat_sdmmc_mount is an all-in-one convenience function.
  // Please check its source code and implement error recovery when developing
  // production applications.
  sdmmc_card_t* card;
  esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);

  if (ret != ESP_OK) {
      if (ret == ESP_FAIL) {
          ESP_LOGE(TAG, "Failed to mount filesystem. "
              "If you want the card to be formatted, set format_if_mount_failed = true.");
      } else {
          ESP_LOGE(TAG, "Failed to initialize the card (%s). "
              "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
      }
      return;
  }


  // Card has been initialized, print its properties
     sdmmc_card_print_info(stdout, card);

     // Use POSIX and C standard library functions to work with files.
     // First create a file.
     ESP_LOGI(TAG, "Opening file");
     FILE* f = fopen("/sdcard/hello.txt", "w");
     if (f == NULL) {
         ESP_LOGE(TAG, "Failed to open file for writing");
         return;
     }
     fprintf(f, "Hello %s!\n", card->cid.name);
     fclose(f);
     ESP_LOGI(TAG, "File written");

     // Check if destination file exists before renaming
     struct stat st;
     if (stat("/sdcard/foo.txt", &st) == 0) {
         // Delete it if it exists
         unlink("/sdcard/foo.txt");
     }


     // Rename original file
     ESP_LOGI(TAG, "Renaming file");
     if (rename("/sdcard/hello.txt", "/sdcard/foo.txt") != 0) {
         ESP_LOGE(TAG, "Rename failed");
         return;
     }

     // Open renamed file for reading
     ESP_LOGI(TAG, "Reading file");
     f = fopen("/sdcard/foo.txt", "r");
     if (f == NULL) {
         ESP_LOGE(TAG, "Failed to open file for reading");
         return;
     }
     char line[64];
     fgets(line, sizeof(line), f);
     fclose(f);
     // strip newline
     char* pos = strchr(line, '\n');
     if (pos) {
         *pos = '\0';
     }
     ESP_LOGI(TAG, "Read from file: '%s'", line);

     // All done, unmount partition and disable SDMMC or SPI peripheral
     esp_vfs_fat_sdmmc_unmount();
     ESP_LOGI(TAG, "Card unmounted");


  while(1){
    // mount card //
    esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);
    // create a new file:
    ESP_LOGI(TAG, "Opening file");

// THIS IS TO CREATE A FULL NEW PATH AND NAME FOR THE FILE /////////////////////
////////////////////////////////////////////////////////////////////////////////

    static char fileNum[10];    // to make a string out of the file number.
    itoa(fnum,fileNum,10);                         //takes fnum number, transforms it into a string, stores in buffer, number in base 10.
    printf(fileNum);
    printf("\n");
    char path[] = "/sdcard/_";
    //const char *line1 = "hello";
    //const char *line2 = "world";

    size_t l1 = strlen(path);
    size_t l2 = strlen(fileNum);

char *fullPath = malloc(l1 + l2 + 1);
if (!fullPath) abort();

memcpy(fullPath,        path, l1);
memcpy(fullPath + l1, fileNum, l2);
fullPath[l1 + l2] = '\0';

////////////////////////////////////////////////////////////////////////////////

    FILE* f = fopen(fullPath, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, "Hello %s!\n", card->cid.name);
    fclose(f);
    fnum += 1;                      // increases the file index
    ESP_LOGI(TAG, "File written");




    vTaskDelay(500);  // returns the execution to the scheduler for 100ms
  }
}

TaskHandle_t heavy_task_handle;
static void heavy_task() {
  const uint32_t imax = 10;
  const uint32_t jmax = 1000;
  const uint32_t kmax = 1000;

  const uint32_t delay = 1;

  static uint32_t i;
  static uint32_t j;
  static uint32_t k;
  static uint32_t n;

  while(1){
    printf("Heavy shit\n");
    for(i = 0; i < imax; i++){
      for(j = 0; j < jmax; j++){
        for(k = 0; k < kmax; k++){
            n += 1;                   // DUMMY WORKLOAD
        }
      }
    }
    //gpio_set_level(SQUARE_SIGNAL_PIN, 1);

    vTaskDelay(5);

    for(i = 0; i < imax; i++){
      for(j = 0; j < jmax; j++){
        for(k = 0; k < kmax; k++){
            n += 1;                   // DUMMY WORKLOAD
        }
      }
    }
    //gpio_set_level(SQUARE_SIGNAL_PIN, 0);

    vTaskDelay(5);  // returns the execution to the scheduler for 100ms
  }
}


// MAIN //
void app_main()
{
  printf("Main\n\n");

    uint16_t i = 0;
    uint16_t a = 0;

    square_gpio_init();

////// everything related with sd card initialization //////////////////////////
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////

    //xTaskCreate(pin_toggle_task, "pin_toggle_task", 2048, NULL, 7, pin_toggle_handle);
    //xTaskCreate(heavy_task, "Heavy Task", 2048, NULL, 5, heavy_task_handle);
    xTaskCreate(sd_task, "sd_task", 1024 * 4, (void* ) 0, 6, sd_task_handle);

}
