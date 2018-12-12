
#define ADS7142_I2C_ADDR          0X18

// command opcodes //

#define SINGLE_READ               0X10
#define SINGLE_WRITE              0X08
#define SET_BIT                   0X18
#define CLEAR_BIT                 0X20
#define CONTINOUS_READ            0X30
#define CONTINOUS_WRITE           0X28



// register addresses //

#define WKEY                      0X17
#define DEVICE_RESET              0X14
#define OFFSET_CAL                0X15
#define OPMODE_SEL                0X1C

#define OPMODE_I2CMODE_STATUS     0X00
#define CHANNEL_INPUT_CFG         0X24
#define AUTO_SEQ_CHEN             0X20
#define START_SEQUENCE            0X1E

#define ABORT_SEQUENCE            0X1F
#define SEQUENCE_STATUS           0X04
#define OSC_SEL                   0X18
#define NCLK_SEL                  0X19

#define DATA_BUFFER_OPMODE        0X2C
#define DOUT_FORMAT_CFG           0X28
#define DATA_BUFFER_STATUS        0X01
#define ACC_EN                    0X30


#define ACC_CH0_LSB               0X08
#define ACC_CH0_MSB               0X09
#define ACC_CH1_LSB               0X0A
#define ACC_CH1_MSB               0X0B

#define ACCUMULATOR_STATUS        0X02
#define ALERT_DWC_EN              0X37
#define ALERT_CHEN                0X34

#define DWC_HTH_CH0_LSB           0X38
#define DWC_HTH_CH0_MSB           0X39
#define DWC_LTH_CH0_LSB           0X3A
#define DWC_LTH_CH0_MSB           0X3B

#define DWC_HYS_CH0               0X40

#define DWC_HTH_CH1_LSB           0X3C
#define DWC_HTH_CH1_MSB           0X3D
#define DWC_LTH_CH1_LSB           0X3E
#define DWC_LTH_CH1_MSB           0X3F

#define DWC_HYS_CH1               0X41

#define PRE_ALERT_MAX_EVENT_COUNT 0X36

#define ALERT_TRIG_CHID           0X03
#define ALERT_LOW_FLAGS           0X0C
#define ALERT_HIGH_FLAGS          0X0E
