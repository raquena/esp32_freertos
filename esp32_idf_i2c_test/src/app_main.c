

#include <stdio.h>
#include "driver/i2c.h"

// registers of the ADS7142 //
#include "ADS7142.h"


// i2c //
#define I2C_EXAMPLE_MASTER_SCL_IO          25               /*!< gpio number for I2C master clock */
#define I2C_EXAMPLE_MASTER_SDA_IO          26               /*!< gpio number for I2C master data  */
#define I2C_PORT_A I2C_NUM_1
#define I2C_EXAMPLE_MASTER_FREQ_HZ         400000           /*!< I2C master clock frequency */
// i2c protocol //
#define WRITE_BIT                          I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT                           I2C_MASTER_READ  /*!< I2C master read */
#define I2C_MASTER_TX_BUF_DISABLE          0                /*!< I2C master do not need buffer */
#define I2C_MASTER_RX_BUF_DISABLE          0

#define ACK_CHECK_EN                       0x1              /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS                      0x0              /*!< I2C master will not check ack from slave */
#define ACK_VAL                            0x0              /*!< I2C ack value */
#define NACK_VAL                           0x1              /*!< I2C nack value */

// ina219 //
#define INA219_I2C_ADDR                    0x45             /*!< slave address for BH1750 sensor */
#define BUS_VOLTAGE_REG                    0X04
#define CONFIG_REG                         0X05

// cap1188 //

#define CAP_1188_I2C_ADDR                   0X29
#define LED_OUTPUT_REG                      0X71
#define GENERAL_STATUS_REG                  0X02


// gpio //
#define SQUARE_SIGNAL_PIN                   14    // used to generate a square signal. to check i2c's protocol
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<SQUARE_SIGNAL_PIN) | (1ULL<<SQUARE_SIGNAL_PIN))


static esp_err_t ADS7142_reg_write(uint8_t reg, uint8_t val){
  int ret = 0;
  uint8_t n = 0;    // counter to make several readings.
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  printf("w reg ADS7142\n");

  gpio_set_level(SQUARE_SIGNAL_PIN, 1); // TO MEASURE TASK TIME //
  cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ADS7142_I2C_ADDR << 1 | WRITE_BIT, ACK_CHECK_EN);      // read slave address
  i2c_master_write_byte(cmd, SINGLE_WRITE, ACK_CHECK_EN);      // read slave address
  i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);                            // slave register
  i2c_master_write_byte(cmd, val, ACK_CHECK_EN);                                  // slave register
  i2c_master_stop(cmd);
  ret = i2c_master_cmd_begin(I2C_PORT_A, cmd, 1000 / portTICK_RATE_MS);
  gpio_set_level(SQUARE_SIGNAL_PIN, 0);   // TO MEASURE TASK TIME //
  i2c_cmd_link_delete(cmd);


  return(ret);

}
static esp_err_t ADS7142_reg_read(uint8_t reg){
  int ret = 0;
  uint8_t val = 0;
  static uint32_t count = 0;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  // INITIALIZATION OF THE DEVICE PART 1//
  printf("r reg ADS7142\n");

  gpio_set_level(SQUARE_SIGNAL_PIN, 1); // TO MEASURE TASK TIME //

  cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);

  i2c_master_write_byte(cmd, ADS7142_I2C_ADDR << 1 | WRITE_BIT, ACK_CHECK_EN);      // read slave address
  i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);                                    // slave register
  i2c_master_write_byte(cmd, SINGLE_READ, ACK_CHECK_EN);                            // slave register

  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ADS7142_I2C_ADDR << 1 | READ_BIT, ACK_CHECK_EN);       // read slave address
  i2c_master_read_byte(cmd, &val, ACK_CHECK_EN);                                    // slave register

  i2c_master_stop(cmd);

  ret = i2c_master_cmd_begin(I2C_PORT_A, cmd, 1000 / portTICK_RATE_MS);
  //vTaskDelay(10);   // this should give the control to another task while the i2c thingie is executed
  i2c_cmd_link_delete(cmd);

  gpio_set_level(SQUARE_SIGNAL_PIN, 0); // TO MEASURE TASK TIME //

  printf("%d\n",count);    // i2c crashing--> always ath the same place?
  count += 1;

  return(ret);

}


// struct for the CAP1188: //
typedef struct str_cap1188 {
  uint8_t i2c_por;
  //uint8_t i2c_port = I2C_PORT_A;
  //uint8_t i2c_address = CAP_1188_I2C_ADDR;
  //void (*CAP1188_write)

} cap1188;

cap1188 touch_sensor;
//touch_sensor.i2c_port = 1;
// i2c //
static esp_err_t CAP1188_write(){
  int ret = 0;
  uint8_t n = 0;    // counter to make several readings.
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  printf("Write Cap1188\n");

  gpio_set_level(SQUARE_SIGNAL_PIN, 1); // TO MEASURE TASK TIME //
  cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, CAP_1188_I2C_ADDR << 1 | WRITE_BIT, ACK_CHECK_EN);      // read slave address
  i2c_master_write_byte(cmd, LED_OUTPUT_REG, ACK_CHECK_EN);                            // slave register
  i2c_master_write_byte(cmd, 0xF0, ACK_CHECK_EN);                                  // slave register
  i2c_master_stop(cmd);
  ret = i2c_master_cmd_begin(I2C_PORT_A, cmd, 1000 / portTICK_RATE_MS);
  gpio_set_level(SQUARE_SIGNAL_PIN, 0);   // TO MEASURE TASK TIME //
  i2c_cmd_link_delete(cmd);


  return(ret);

}
static esp_err_t CAP1188_read(){
  int ret = 0;
  uint8_t val = 0;
  static uint32_t count = 0;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  // INITIALIZATION OF THE DEVICE PART 1//
  printf("Read Cap1188\n");

  gpio_set_level(SQUARE_SIGNAL_PIN, 1); // TO MEASURE TASK TIME //

  cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);

  i2c_master_write_byte(cmd, CAP_1188_I2C_ADDR << 1 | WRITE_BIT, ACK_CHECK_EN);      // read slave address
  i2c_master_write_byte(cmd, LED_OUTPUT_REG, ACK_CHECK_EN);                           // slave register

  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, CAP_1188_I2C_ADDR << 1 | READ_BIT, ACK_CHECK_EN);      // read slave address
  i2c_master_read_byte(cmd, &val, ACK_CHECK_EN);                           // slave register

  i2c_master_stop(cmd);

  ret = i2c_master_cmd_begin(I2C_PORT_A, cmd, 1000 / portTICK_RATE_MS);
  //vTaskDelay(10);   // this should give the control to another task while the i2c thingie is executed
  i2c_cmd_link_delete(cmd);

  gpio_set_level(SQUARE_SIGNAL_PIN, 0); // TO MEASURE TASK TIME //

  printf("%d\n",count);    // i2c crashing--> always ath the same place?
  count += 1;

  return(ret);

}

static esp_err_t INA219_init(){
  int ret = 0;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  // INITIALIZATION OF THE DEVICE PART 1//

  cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, INA219_I2C_ADDR << 1 | WRITE_BIT, ACK_CHECK_EN);      // read slave address
  i2c_master_write_byte(cmd, CONFIG_REG, ACK_CHECK_EN);                           // slave register
  i2c_master_write_byte(cmd, 0x10, ACK_CHECK_EN);                           // slave register
  i2c_master_write_byte(cmd, 0x00, ACK_CHECK_EN);                           // slave register
  i2c_master_stop(cmd);

  ret = i2c_master_cmd_begin(I2C_PORT_A, cmd, 1000 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);


  // INITIALIZATION OF THE DEVICE PART 2//

  cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, INA219_I2C_ADDR << 1 | WRITE_BIT, ACK_CHECK_EN);      // read slave address
  i2c_master_write_byte(cmd, 0X00, ACK_CHECK_EN);                           // slave register
  i2c_master_write_byte(cmd, 0x39, ACK_CHECK_EN);                           // slave register
  i2c_master_write_byte(cmd, 0x9F, ACK_CHECK_EN);                           // slave register
  i2c_master_stop(cmd);

  ret = i2c_master_cmd_begin(I2C_PORT_A, cmd, 1000 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);

  return(ret);

}
static esp_err_t INA219_read(){
    uint16_t * p_data = 0;
    uint16_t data = 0;
    int ret = 0;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    // WRITE THE CONFIG REGISTER //

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, INA219_I2C_ADDR << 1 | WRITE_BIT, ACK_CHECK_EN);      // read slave address
    i2c_master_write_byte(cmd, CONFIG_REG, ACK_CHECK_EN);                           // slave register
    i2c_master_write_byte(cmd, 0x10, ACK_CHECK_EN);                           // slave register
    i2c_master_write_byte(cmd, 0x00, ACK_CHECK_EN);                           // slave register
    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(I2C_PORT_A, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    // SET THE REGISTER POINTER OF THE SLAVE TO BUS_VOLTAGE_REG //

    printf("Configuration REGISTER write DONE\n");

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, INA219_I2C_ADDR << 1 | WRITE_BIT, ACK_CHECK_EN);      // read slave address
    i2c_master_write_byte(cmd, BUS_VOLTAGE_REG, ACK_CHECK_EN);                   // read slave address
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_PORT_A, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    printf("Point to the right slave register DONE\n");


    // READ VOLTAGE_VAL REGISTER //

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, INA219_I2C_ADDR << 1 | READ_BIT, ACK_CHECK_EN);      // read slave address
    i2c_master_read_byte(cmd, data, ACK_CHECK_EN);
    i2c_master_read_byte(cmd, data, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_PORT_A, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);



    printf("Read from I2C slave done\n");

    //i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
        return ret;
    }
    return ret;
}

static void i2c_master_init(){
    int i2c_master_port = I2C_PORT_A;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_EXAMPLE_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_DISABLE;
    conf.scl_io_num = I2C_EXAMPLE_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_DISABLE;
    conf.master.clk_speed = I2C_EXAMPLE_MASTER_FREQ_HZ;
    i2c_param_config(i2c_master_port, &conf);
    i2c_driver_install(i2c_master_port, conf.mode,
                       I2C_MASTER_RX_BUF_DISABLE,
                       I2C_MASTER_TX_BUF_DISABLE, 0);
}

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

TaskHandle_t i2c_handle;
static void i2c_task() {
  while(1){
    uint8_t data = 0;
    ADS7142_reg_write(0x00, 0x00);    // write register 0 with all zeroes.
    ADS7142_reg_read(0x00);    // write register 0 with all zeroes.

    //CAP1188_write();      // writes simple command i2c bus
    //CAP1188_read();

    vTaskDelay(1);  // returns the execution to the scheduler for 100ms
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
    square_gpio_init();
    printf("Gpio Init\n");
    i2c_master_init();  // i2c initialization //
    printf("i2c intialized\n");
    //xTaskCreate(pin_toggle_task, "pin_toggle_task", 2048, NULL, 7, pin_toggle_handle);
    //xTaskCreate(heavy_task, "Heavy Task", 2048, NULL, 5, heavy_task_handle);
    xTaskCreate(i2c_task, "i2c_task", 1024 * 3, (void* ) 0, 6, i2c_handle);

}
