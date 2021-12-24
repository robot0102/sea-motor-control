#ifndef MODEL526_H
#define MODEL526_H

//Include
//#include <rtdm/udd.h>
#include <stdio.h>
#include <sys/io.h>
#include <errno.h>
#include <string.h> 
#include <stdint.h> 
#include <assert.h> 

/* Board description */
#define S526_GPCT_CHANS	4
#define S526_GPCT_BITS	24
#define S526_AI_CHANS	10	
/* 8 regular differential inputs
* channel 8 is "reference 0" (+10V)
* channel 9 is "reference 1" (0V) 
* */
#define S526_AI_BITS	16
#define S526_AI_TIMEOUT 100
#define S526_AO_CHANS	4
#define S526_AO_BITS	16
#define S526_DIO_CHANS	8
#define S526_DIO_BITS	1

/* Ports */
#define S526_IOSIZE		0x40  /* 64 bytes */
#define S526_DEFAULT_ADDRESS	0x2c0 /* Set by the headers on the board */
#define S526_DEFAULT_ADDRESS_2	0x2c0 /* Set by the headers on the board */

/* Registers */
#define REG_TCR 0x00
#define REG_WDC 0x02
#define REG_DAC 0x04
#define REG_ADC 0x06
#define REG_ADD 0x08
#define REG_DIO 0x0A
#define REG_IER 0x0C
#define REG_ISR 0x0E
#define REG_MSC 0x10
#define REG_C0L 0x12
#define REG_C0H 0x14
#define REG_C0M 0x16
#define REG_C0C 0x18
#define REG_C1L 0x1A
#define REG_C1H 0x1C
#define REG_C1M 0x1E
#define REG_C1C 0x20
#define REG_C2L 0x22
#define REG_C2H 0x24
#define REG_C2M 0x26
#define REG_C2C 0x28
#define REG_C3L 0x2A
#define REG_C3H 0x2C
#define REG_C3M 0x2E
#define REG_C3C 0x30
#define REG_EED 0x32
#define REG_EEC 0x34

#define ISR_ADC_DONE 0x4

/* BITS in the Register */
#define BIT_RD_START    0
#define BIT_RD_SEL      1
#define BIT_EN_CH0      5
#define BIT_EN_CH1      6
#define BIT_EN_CH2      7
#define BIT_EN_CH3      8
#define BIT_EN_CH4      9
#define BIT_EN_CH5     10
#define BIT_EN_CH6     11
#define BIT_EN_CH7     12
#define BIT_EN_REF_10V 13
#define BIT_EN_REF_0V  14
#define BIT_MULT_DELAY 15

/*ADC Params*/
//#define ADC_CONVERSION_FACTOR 3.05185e-4
#define ADC_CONVERSION_FACTOR 3.08677e-04
#define ADC_ZERO_FACTOR 0.013

#define DAC_CONVERSION_FACTOR 3249.0
#define DAC_ZERO_FACTOR 32768.0

#define ADDR_REG(dev_add,reg) (dev_add + (reg))

//define register struct
struct s526_reg_
{
	uint16_t IER;
	uint16_t DIO;
        uint16_t C0C;
        uint16_t C1C;
        uint16_t C2C;
        uint16_t C3C;
        uint16_t ACR;
};

//Function to initialize IO permissions
int s526_init(int dev_add);
//Function to read the ID and test board
int s526_read_id(int dev_add);
//Function to initialize ADC channels
void s526_adc_init(int *channels, int num_channels, int dev_add);
//Function to read adc channels
void s526_adc_read(int *channels, int num_channels, double *adc_sample_array, int dev_add);

//FUnction to initialize Digital IO
void s526_digitalIO_init(int port, int mode, int interrupt_condition, int dev_add);
//Function to Digital IO input
int s526_digitalIO_read(int port, int dev_add);
//FUnction to Digital IO output
void s526_digitalIO_set(int port,int dev_add);
void s526_digitalIO_clear(int port,int dev_add);
void s526_digitalIO_toggle(int port,int dev_add);



//Function to initialize counter and encoder channels
void s526_init_pulse_timer(int channel,int dev_add);
int s526_counter_read(int channel, int dev_add);
void s526_encoder_init(int channel_number, int dev_add);
int s526_encoder_read(int channel_number, int dev_add);
void s526_counter_set_control_status(int channel_number, int coun_reset, int count_load,
                                     int count_arm, int latch_select, int intettupt_enable, int dev_add);
//Function to read the encoder count
//Function to initialize the analog output channels
void s526_dac_init(int dev_add);
//Function to set the analog output
void s526_dac_write(double *vals, int nvals, int dev_add);

uint16_t s526_read_eeprom_word(uint8_t addr, int dev_add);
double s526_read_adc_calib(int dev_add);
double s526_read_dac_calib_a(int channel, int dev_add);
double s526_read_dac_calib_b(int channel, int dev_add);


int pwm_write(int channel_number, float period_us, float duty_cycle, int dev_add);
int pwm_write_2(int channel_number, float period_us, int dev_add); //assmue duty cycle is 0.5   

int pwm_write_position_pulse(int channel_number, float period_us, int dev_add);
int init_encoder_pos(int channel_number, int32_t des_pos, int dev_add);



#endif
