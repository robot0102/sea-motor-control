/***************************************
*MODEL 526 Custom Driver
Setup instructions
1) Check base address jumpers J1. Make sure they match #define S526_DEFAULT_ADDRESS. 
(The jumpers configures bit 6 to 15 of boards base address. Inserted jumpers represents 0. )

2) Before using DAC, ensure analog output feedback jumper is inserted in J2.
BUT, if feedback provided through Feedback Pin of J3 Analog connector, remove the corresponding J2 pin

3) Before using DAC, calibrate by running s526_dac_init() 
Go to s526_dac_write(), adjust hardcoded calculation of #define DAC_CONVERSION_FACTOR and #define DAC_ZERO_FACTOR

4) Before using ADC, calibrate by running s526_read_adc_calib()
Go to s526_adc_read(), adjust hardcoded calculation of #define ADC_CONVERSION_FACTOR and #define ADC_ZERO_FACTOR

****************************************/


#include "model526.h"

static struct s526_reg_ s526_reg;
// Calibration Parameters.
static double C_ref = 0;
static double DACa[4] = {1.0, 1.0, 1.0, 1.0};
static double DACb[4] = {0.0, 0.0, 0.0, 0.0};
static int s526_init_flag = 0;

int s526_init(int dev_add)
{
    //Set permissions for the range of IO addresses
    //int ret = ioperm(S526_ADDR, S526_IOSIZE, 1);
    if(s526_init_flag == 0)
    {
        int ret = iopl(3);

        printf("Reading board id...");
        int init = s526_read_id(dev_add);
        if(((init == 0x526b) || (init == 0x526a)))
        {
            printf("Read correct board id: %04x \n", init);
            return ret;
        }
        else
        {
            return -526;
        }
        s526_init_flag = 1;
    }
    
    return 2;
}

int s526_read_id(int dev_add)
{
 

    //Read the id word

    int  id = inw(dev_add + REG_EEC);

    return id;
}

uint16_t s526_read_eeprom_word(uint8_t addr, int dev_add)
{
    uint16_t w;
    uint16_t rw;
    const uint16_t sbit = 1 << 7;


    outw(sbit, ADDR_REG(dev_add, REG_ISR));
    w = ((addr & 0b111111) << 3) | (2 << 1) | 1;

    outw(w, ADDR_REG(dev_add, REG_EEC));


    printf("Reading EEPROM at address %04x\n", addr);
    while(!(sbit & inw(ADDR_REG(dev_add, REG_ISR)))){}
    outw(sbit, ADDR_REG(dev_add, REG_ISR));
    rw = inw(ADDR_REG(dev_add, REG_EED));
    printf("Read done.\n");

   

    return rw;
}

double s526_read_adc_calib(int dev_add)
{
    //read reference volatge values
    
    uint16_t w = 0x0000, w1=0x0000;
    int i;
    int j;
    int16_t adreg = 0;
    int16_t adc_ref_array[2];
    
    //Start conversion
    w = s526_reg.ACR | 0x0001;
    outw(w, ADDR_REG(dev_add, REG_ADC));

    //Wait for ISR bit to set signalling end of conversion
    for(;;)
    {
        if(ISR_ADC_DONE & inw(ADDR_REG(dev_add, REG_ISR)))
        {
	    outw(ISR_ADC_DONE, ADDR_REG(dev_add, REG_ISR));
	    break;
        }
    }

    //Loop over channels and read
    for(i=8; i<10; i++)
    {
        //Set multiplexer bits and start conversion bit
        w = (i << 1); // channel 8 -> 10V reference, 9->0 reference 
        outw(w, ADDR_REG(dev_add, REG_ADC));
        //Write converted value to output array
        adc_ref_array[i-8] = inw(ADDR_REG(dev_add, REG_ADD));
    }
    //printf("adc_ref_array[0] = %d\n",adc_ref_array[0]);
    //printf("adc_ref_array[1] = %d\n",adc_ref_array[1]);
 
    //read EEPROM calibration data
    uint16_t doublebuf[4];
    double calibval;
    double ADCconvert,ADCzero;
    
    doublebuf[0] = s526_read_eeprom_word((uint8_t) 0x20, dev_add);
    doublebuf[1] = s526_read_eeprom_word((uint8_t) 0x21, dev_add);
    doublebuf[2] = s526_read_eeprom_word((uint8_t) 0x22, dev_add);
    doublebuf[3] = s526_read_eeprom_word((uint8_t) 0x23, dev_add);


    memcpy(&calibval, doublebuf, sizeof(double));

    //set ADC_CONVERSION_FACTOR to this
    ADCconvert = calibval/ (adc_ref_array[0] - adc_ref_array[1])*10000;
    ADCzero = adc_ref_array[1]*ADCconvert;  
    printf("set ADC_CONVERSION_FACTOR to %lf e-04 \n", ADCconvert);
    printf("set ADC_ZERO_FACTOR to %lf e-04 \n", ADCzero);

    return calibval;
}

double s526_read_dac_calib_a(int channel, int dev_add)
{
    int8_t start_addr;
    uint16_t doublebuf[4];
    double val;
    
    start_addr = channel*8;

    doublebuf[0] = s526_read_eeprom_word(start_addr, dev_add);
    doublebuf[1] = s526_read_eeprom_word(start_addr + 1, dev_add);
    doublebuf[2] = s526_read_eeprom_word(start_addr + 2, dev_add);
    doublebuf[3] = s526_read_eeprom_word(start_addr + 3, dev_add);

    memcpy(&val, doublebuf, sizeof(double));

    return val;    
}

double s526_read_dac_calib_b(int channel, int dev_add)
{
    int8_t start_addr;
    uint16_t doublebuf[4];
    double val;
    
    start_addr = channel*8;

    doublebuf[0] = s526_read_eeprom_word(start_addr + 4, dev_add);
    doublebuf[1] = s526_read_eeprom_word(start_addr + 5, dev_add);
    doublebuf[2] = s526_read_eeprom_word(start_addr + 6, dev_add);
    doublebuf[3] = s526_read_eeprom_word(start_addr + 7, dev_add);

    memcpy(&val, doublebuf, sizeof(double));

    return val;    
}




uint16_t s526_gen_adc_conf(int *channels, int num_channels)
{
    int i;
    uint16_t word;
    
    //Start off with enabling multiplexer delay
    word = 0x8000;
    //Set channel conversion bits
    for(i=0; i<num_channels; i++)
    {
        word |= 1<<(channels[i] + 5);
    }
    
    // Enable references too.
    word |= 1 << (8+5);
    word |= 1 << (9+5);
    return word;
}

void s526_adc_init(int *channels, int num_channels, int dev_add)
{    
    //Write setting to device
    s526_reg.ACR = 0x0;
    s526_reg.ACR = s526_gen_adc_conf(channels, num_channels);
    //printf("Writing: %4x\n", s526_reg.ACR);
    outw(s526_reg.ACR, ADDR_REG(dev_add, REG_ADC));
    //Enable the ADC interrupt in the ISR
    s526_reg.IER |= ISR_ADC_DONE;
    //printf("Writing: %4x\n", s526_reg.IER);
    outw(s526_reg.IER, ADDR_REG(dev_add, REG_IER));
    //Reset the ISR bit
    outw(ISR_ADC_DONE, ADDR_REG(dev_add, REG_ISR));
}

//Function to read the adc of the model 526
void s526_adc_read(int *channels, int num_channels, double *adc_sample_array, int dev_add)
{
    uint16_t w = 0x0000, w1=0x0000;
    int i;
    int j;
    int16_t adreg = 0;
     
    
    //Start out with the existing config word
    //w = s526_gen_adc_conf(channels, num_channels);

    //Start conversion
    w = s526_reg.ACR | 0x0001;
    //printf("Writing: %4x\n", w);
    outw(w, ADDR_REG(dev_add, REG_ADC));

    //Wait for ISR bit to set signalling end of conversion
    //for(j=0; j<400; j++)
    for(;;)
    {
        if(ISR_ADC_DONE & inw(ADDR_REG(dev_add, REG_ISR)))
        {
	    //printf("ADC INTR DETECTED at %d \n", j);
	    //printf("Resetting INTR bit..\n");
	    outw(ISR_ADC_DONE, ADDR_REG(dev_add, REG_ISR));
	    break;
        }
    }

    //Loop over channels and read
    for(i=0; i<num_channels; i++)
    {
        //Set multiplexer bits and start conversion bit
        w = s526_reg.ACR | (channels[i] << 1);
        //printf("Writing: %4x\n", w);
        outw(w, ADDR_REG(dev_add, REG_ADC));
        //printf("Writing: %4x\n", w);
        //Write converted value to output array
	  adreg = inw(ADDR_REG(dev_add, REG_ADD));
    adc_sample_array[i] = (ADC_CONVERSION_FACTOR * adreg - ADC_ZERO_FACTOR) * 72; //360deg/5V = 72
    }  
}





//init DAC
void s526_dac_init(int dev_add)
{
    int idx;
    //reset DAC data
    uint16_t w=0x0008;
    outw(w, ADDR_REG(dev_add, REG_DAC));
    //Enable the DAC interrupt in the ISR
    outw((s526_reg.IER | 0x02), ADDR_REG(dev_add, REG_IER));

    DACa[0] = s526_read_dac_calib_a(0, dev_add);
    DACa[1] = s526_read_dac_calib_a(1, dev_add);
    DACa[2] = s526_read_dac_calib_a(2, dev_add);
    DACa[3] = s526_read_dac_calib_a(3, dev_add);

    DACb[0] = s526_read_dac_calib_b(0, dev_add);
    DACb[1] = s526_read_dac_calib_b(1, dev_add);
    DACb[2] = s526_read_dac_calib_b(2, dev_add);
    DACb[3] = s526_read_dac_calib_b(3, dev_add);

#ifndef NDEBUG
    printf("DACa[0] = %lf, DACb[0] = %lf\n",DACa[0], DACb[0]);
    printf("DACa[1] = %lf, DACb[1] = %lf\n",DACa[1], DACb[1]);
    printf("DACa[2] = %lf, DACb[2] = %lf\n",DACa[2], DACb[2]);
    printf("DACa[3] = %lf, DACb[3] = %lf\n",DACa[3], DACb[3]);
#endif
}




//Function to write to DAC
void s526_dac_write(double *vals, int nvals, int dev_add)
{
    uint16_t w=0x0000;
    int i, j;
    float a;
    double C;
    
    for(i = 0; i < nvals; i++)
    {
        // Select the channel
        w |= i << 1;
        outw(w, ADDR_REG(dev_add, REG_DAC));
	
        // Write data to the channel
        //C = vals[i]*DACa[i] + DACb[i];
        C = vals[i]*DACa[i] + DACb[i];
        //outw((uint16_t) (65535*(C + 10.0)/20.0), ADDR_REG(dev_add, REG_ADD));

        //outw((uint16_t) C, ADDR_REG(dev_add, REG_ADD)); //don't need to cast data type
        outw(C, ADDR_REG(dev_add, REG_ADD));

        // Start conversion
        w = 1;
        outw(w, ADDR_REG(dev_add, REG_DAC));
        // Wait for ISR bit to set singalling end of conversion
        while(!(0x02 & inw(ADDR_REG(dev_add, REG_ISR)))){}
        // Reset the interrupt bit
        outw(0x02, ADDR_REG(dev_add, REG_ISR));
    }	 
}




/************************************************************************
 *************Function to initialize the digital I/O
 *@para: port, the port number needed to set
 *          0 1 2 3 4 5 6 7
 *       mode, input or output (configurable as group 1 or 2) 
 *          0-input  1-output 2-input with interupt 3-output with interupt
 *       interrupt_condition, interrupt condition, just for input using
 *          0-interrupt on a rising edge
 *          1-interrupt on a failing edge
 *@return: NULL
 *
 *@note: port 0 1 2 3 is group 1, can be set different interrupt condition BUT can ONLY set same mode
 *       port 4 5 6 7 is group 2, can ONLY set same interrupt condition and mode
 ***********************************************************************
 */
void s526_digitalIO_init(int port, int mode, int interrupt_condition, int dev_add)
{
    if((port>=0)&&(port<=7))
    {
        //set interrupt
        if(mode == 2 || mode == 3 ){
            switch(port){
                case 0: s526_reg.DIO |= (interrupt_condition << 12);
                    break;
                case 1: s526_reg.DIO |= (interrupt_condition << 13);
                    break;
                case 2: s526_reg.DIO |= (interrupt_condition << 14);
                    break;
                case 3: s526_reg.DIO |= (interrupt_condition << 15);
                    break;
                default: s526_reg.DIO |= (interrupt_condition << 8);
            }
            s526_reg.IER |= ( 1<< (8+port));
            outw(s526_reg.IER, ADDR_REG(dev_add, REG_IER));
        }
        //set mode
        if(mode == 1 || mode == 3){
            if (port<4){
                s526_reg.DIO |= (1 << 10);
            }
            else{
                s526_reg.DIO |= (1 << 11);
            }
        }
        else{
            if (port<4){
                s526_reg.DIO &= ~(1 << 10);
            }
            else{
                s526_reg.DIO &= ~(1 << 11);
            }
        }
        outw(s526_reg.DIO, ADDR_REG(dev_add, REG_DIO));

    }
    else
    {
        printf("Set the wrong port\n");
    }
}

/************************************************************************
 *************Function to read digital I/O
 *@para: port, the port number needed to set
 *          0 1 2 3 4 5 6 7
 *@return: 1 set
 *
 *@note:
 ***********************************************************************
 */
 int s526_digitalIO_read(int port, int dev_add)
 {
     uint8_t ret=0;
     ret = 0xff & inw(ADDR_REG(dev_add, REG_DIO));
     if( ret & (1 << port))
        return 1;
     else
         return 0;
 }

/************************************************************************
 *************Function to initialize the digital I/O
 *@para: channel, the port number needed to set
 *          0 1 2 3 4 5 6 7
 *       state, input or output (configurable as group 1 or 2) 
 *          0-input  1-output 2-input with interupt 3-output with interupt
 *       interrupt_condition, interrupt condition, just for input using
 *          0-interrupt on a rising edge
 *          1-interrupt on a failing edge
 *@return: NULL
 *
 *@note: port 0 1 2 3 is group 1, can be set different interrupt condition BUT can ONLY set same mode
 *       port 4 5 6 7 is group 2, can ONLY set same interrupt condition and mode
 ***********************************************************************
 */
void s526_digitalIO_set(int port, int dev_add)
{
    //Function: 0 -> 1
    //          1 -> 1
    
    s526_reg.DIO |= (1 << port);
    outw(s526_reg.DIO, ADDR_REG(dev_add, REG_DIO));
}

void s526_digitalIO_clear(int port, int dev_add)
{
    //Function: 0 -> 0
    //          1 -> 0

    s526_reg.DIO &= ~(1 << port);
    outw(s526_reg.DIO, ADDR_REG(dev_add, REG_DIO));
}

void s526_digitalIO_toggle(int port, int dev_add)
{
    //Function: 0 -> 1
    //          1 -> 0

    s526_reg.DIO ^= 1 << port;
    outw(s526_reg.DIO, ADDR_REG(dev_add, REG_DIO));
}

// Function to initialize counter for measuring pulse width.
void s526_init_pulse_timer(int channel_number, int dev_add) {
    const int mode1 = 0x55AC;
    const int mode2 = 0x35AC;
    const int control = 0xC80F;

    // Make sure that channel number is withing bounds.
    assert((channel_number <4) && (channel_number >=0));

    // Setup mode register
    outw(mode1, ADDR_REG(dev_add, REG_C0M + 8*channel_number));
    // Zero the pre-load registers
    outw(0x0000, ADDR_REG(dev_add, REG_C0L + 8*channel_number));
    outw(0x0000, ADDR_REG(dev_add, REG_C0H + 8*channel_number));

    outw(mode2, ADDR_REG(dev_add, REG_C0M + 8*channel_number));
    // Zero the pre-load registers.
    outw(0x0000, ADDR_REG(dev_add, REG_C0L + 8*channel_number));
    outw(0x0000, ADDR_REG(dev_add, REG_C0H + 8*channel_number));

    // Setup the control register
    outw(control, ADDR_REG(dev_add, REG_C0C + 8*channel_number));
}



//Function to initialize encoder
void s526_encoder_init(int channel_number, int dev_add)
{
    uint16_t w = 0;

    const int PR0 = 0;
    const int LATCH_ON_READ = 0;
    const int COUNT_QUADRATURE = 0;
    const int COUNT_UP = 0;
    const int CLK_QUAD = 3;
    const int CEN_ON = 1;
    const int HW_CEN = 0;
    const int AUTOLOAD = 0;
    const int COUT_POL_NORMAL = 0;
    const int COUT_SRC_RTGL = 1;

    w = (PR0 << 14) |
	(LATCH_ON_READ << 13) |
	(COUNT_QUADRATURE << 12) |
	(COUNT_UP << 11) |
	(CLK_QUAD << 9) |
	(CEN_ON << 7) |
	(HW_CEN << 5) |
	(AUTOLOAD << 2) |
	(COUT_POL_NORMAL<<1) |
	(COUT_SRC_RTGL);
    /* w |= (PR_REG << 14); */
    /* w |= (LATCH_CTRL << 13); */
    /* w |= (COUNT_DIRECTION_CTRL << 12); */
    /* w |= (COUNT_DIRECTION << 11); */
    /* w |= (CLK_SRC << 9); */
    /* w |= (CEN_CONTROL << 7); */
    /* w |= (HW_CEN << 5); */
    /* w |= (AUTO_LOAD << 2); */
    /* w |= (COUNT_POL <<1); */
    /* w |= COUT_SRC; */
    // Write counter mode settings
    outw(w, ADDR_REG(dev_add, REG_C0M + channel_number * 8));   
    // Reset the counter
    outw(1<<15, ADDR_REG(dev_add, REG_C0C + channel_number * 8));

}

//Function to read the encoder count.
int s526_encoder_read(int channel_number, int dev_add)
{
    int ret;
    // Always read low word first
    ret = inw(ADDR_REG(dev_add, REG_C0L + 8 * channel_number));
    ret |= inw(ADDR_REG(dev_add, REG_C0H + 8 * channel_number)) << 16;
    ret = (ret << 8)/256;

    return ret;
}



int s526_counter_read(int channel_number, int dev_add) {
    int ret;

    // Make sure that channel number is withing bounds.
    assert((channel_number <4) && (channel_number >=0));

    // Read the low word first
    ret = inw(ADDR_REG(dev_add, REG_C0L + 8*channel_number));
    ret |= inw(ADDR_REG(dev_add, REG_C0H + 8*channel_number)) << 16;
    ret = (ret << 8)/256;

    return ret;
}

//Function to set counter control register
void s526_counter_set_control_status(int channel_number, int coun_reset, int count_load, int count_arm, int latch_select,
                                 int intettupt_enable, int dev_add)
{
    switch(channel_number)
    {
    case 0:
        s526_reg.C0C |= (coun_reset << 15);
        s526_reg.C0C |= (count_load << 14);
        s526_reg.C0C |= (count_arm << 13);
        s526_reg.C0C |= (latch_select << 10);
        s526_reg.C0C |= (intettupt_enable << 6);
        outw(s526_reg.C0C, ADDR_REG(dev_add, REG_C0C));
        break;
    case 1:
        s526_reg.C1C |= (coun_reset << 15);
        s526_reg.C1C |= (count_load << 14);
        s526_reg.C1C |= (count_arm << 13);
        s526_reg.C1C |= (latch_select << 10);
        s526_reg.C1C |= (intettupt_enable << 6);
        outw(s526_reg.C1C, ADDR_REG(dev_add, REG_C1C));
        break;
    case 2:
        s526_reg.C2C |= (coun_reset << 15);
        s526_reg.C2C |= (count_load << 14);
        s526_reg.C2C |= (count_arm << 13);
        s526_reg.C2C |= (latch_select << 10);
        s526_reg.C2C |= (intettupt_enable << 6);
        outw(s526_reg.C2C, ADDR_REG(dev_add, REG_C2C));
        break;
    case 3:
        s526_reg.C3C |= (coun_reset << 15);
        s526_reg.C3C |= (count_load << 14);
        s526_reg.C3C |= (count_arm << 13);
        s526_reg.C3C |= (latch_select << 10);
        s526_reg.C3C |= (intettupt_enable << 6);
        outw(s526_reg.C3C, ADDR_REG(dev_add, REG_C3C));
        break;
    }
}

//Function to read counter status register
/* void s526_counter_control_status(int channel_number, int *INDEX_status, int *COUT_status, */
/*                                  int *capture_event_status) */
/* { */
/*     INDEX_status = 0x20 & inw(ADDR_REG(dev_add, REG_C0C + channel_number * 8)); */
/*     COUT_status = 0x10 & inw(ADDR_REG(dev_add, REG_C0C + channel_number * 8)); */
/*     capture_event_status = 0x0F & inw(ADDR_REG(dev_add, REG_C0C + channel_number * 8)); */
/* } */

//Function to set PWM mode

int pwm_write(int channel_number, float period_us, float duty_cycle, int dev_add)
{
    //arguments: (period_us) Input in microsecond (duty_cycle) Input between 0-1
    //For high speed pulse, minimum pulse width is 0.25us for duty cyce of 0.5
    //Minimum high_period and low_period is 0.125us 

    uint32_t period_count,high_count,low_count;
    if (duty_cycle ==0.0||period_us==0.0){
        //stop the motor
        high_count=0;
        low_count=0;
    }else if (period_us<0.25){
        printf("period too short");
        return -1;
    }else if (duty_cycle>1 || duty_cycle <0){
        printf("duty cycle invalid. please enter one between 0 to 1 ");
        return -1;
    }else{ 
        period_count = (int)(27*period_us); //  27 = 27000000 MHz / 1000000  
        high_count = (int)(period_count*duty_cycle);
        low_count = (period_count - high_count);
    }

        //always load highword before low word
        //Load Preload Register 0
        outw(0x1C85, ADDR_REG(dev_add, REG_C0M + channel_number * 8));
        outw((low_count>>16)&0xFFFF, ADDR_REG(dev_add, REG_C0H + channel_number * 8));
        outw((low_count&0xFFFF), ADDR_REG(dev_add, REG_C0L + channel_number * 8));

        //Load Preload Register 1
        outw(0x5C85, ADDR_REG(dev_add, REG_C0M + channel_number * 8));
        outw((high_count>>16)&0xFFFF, ADDR_REG(dev_add, REG_C0H + channel_number * 8));
        outw((high_count&0xFFFF), ADDR_REG(dev_add, REG_C0L + channel_number * 8));
    
}

int pwm_write_2(int channel_number, float period_us, int dev_add)
{
    //assume duty cycle is 0.5

    //arguments: (period_us) Input in microsecond
    //For high speed pulse, minimum pulse width is 0.25us 

    uint32_t period_count,high_count,count_hw,count_lw;
    if (period_us==0.0){
         //stop the motor
        count_hw =0;
        count_lw =0;
    }else if (period_us<0.25){
        printf("period too short");
        return -1;
    }else{
        
        period_count = (int)(27*period_us); //  27 = 27000000 MHz / 1000000  
        high_count = (int)(period_count/2) ;

        count_hw = (high_count&0xffff0000)>>16;
        count_lw = (high_count&0xffff);
        
    }

        //always load highword before low word
        //Load Preload Register 0
        outw(0x1C85, ADDR_REG(dev_add, REG_C0M + channel_number * 8));
        outw(count_hw, ADDR_REG(dev_add, REG_C0H + channel_number * 8));
        outw(count_lw, ADDR_REG(dev_add, REG_C0L + channel_number * 8));

        //Load Preload Register 1
        outw(0x5C85, ADDR_REG(dev_add, REG_C0M + channel_number * 8));
        outw(count_hw, ADDR_REG(dev_add, REG_C0H + channel_number * 8));
        outw(count_lw, ADDR_REG(dev_add, REG_C0L + channel_number * 8));
        
        
}



//Function to count encoder output for Delta Motor position control 
int init_encoder_pos(int channel_number, int32_t des_pos, int dev_add)
{
    //arguments: (des_pos) Input deisred number of pulse to look out for
    //For high speed pulse, minimum pulse width is 0.25us 

/**********************************
This functions initializes a counter to count the encoder's pulse output.

Encoder_count < des_pos : counter output 1
Encoder_count >= des_pos : counter output 0

To use, connect the output of the encoder counter to the Enable pin of the PWM generator
Intended for use with function ( pwm_write_position_pulse ) for position control of Delta Motor
************************************/

    uint16_t w = 0;

    const int PR0 = 0;
    const int LATCH_ON_READ = 0;
    const int COUNT_QUADRATURE = 0; // count direction -> quadrature
    const int COUNT_DOWN = 1;
    const int CLK_QUAD = 0b11;     //clock source -> quadrature x4
    const int CEN_ON = 0b10;       //count enable -> hardware count control
    const int HW_CEN = 0b11;       //hardware count source -> NOT RCAP
    const int AUTOLOAD = 0;
    const int COUT_POL_NORMAL = 1;  //cout polarity -> inverted
    const int COUT_SRC_RCAP = 0;    //cout source - RCAP

    w = (PR0 << 14) |
    (LATCH_ON_READ << 13) |
    (COUNT_QUADRATURE << 12) |
    (COUNT_DOWN << 11) |
    (CLK_QUAD << 9) |
    (CEN_ON << 7) |
    (HW_CEN << 5) |
    (AUTOLOAD << 2) |
    (COUT_POL_NORMAL<<1) |
    (COUT_SRC_RCAP);
   
     
    // Write counter mode settings
 //   outw(w, ADDR_REG(dev_add, REG_C0M + channel_number * 8));  


    //write desired count
    uint32_t period_count,high_count,count_hw,count_lw;
    if (des_pos==0.0){
         //stop the motor
        count_hw =0;
        count_lw =0;
    }else if (des_pos>8000000 || des_pos<-8000000 ){
        printf("position out of range");
        return -1;
    }else{
        count_hw = (des_pos&0xffff0000)>>16;
        count_lw = (des_pos&0xffff);
        
    }

    //always load highword before low word
    //Load Preload Register 0
    outw(w, ADDR_REG(dev_add, REG_C0M + channel_number * 8));
    outw(count_hw, ADDR_REG(dev_add, REG_C0H + channel_number * 8));
    outw(count_lw, ADDR_REG(dev_add, REG_C0L + channel_number * 8));

//    outw(0x1d62, ADDR_REG(dev_add, REG_C0M + channel_number * 8));
//   outw(0x1, ADDR_REG(dev_add, REG_C0H + channel_number * 8));
//   outw(0x3c68, ADDR_REG(dev_add, REG_C0L + channel_number * 8));
    // Reset the counter
    outw(1<<15, ADDR_REG(dev_add, REG_C0C + channel_number * 8));
    //load the counter from PR0
    outw(1<<14, ADDR_REG(dev_add, REG_C0C + channel_number * 8));
    //fires oneshot
    outw(1<<3, ADDR_REG(dev_add, REG_C0C + channel_number * 8)); 
}

int pwm_write_position_pulse(int channel_number, float period_us, int dev_add)
{
    //assume duty cycle is 0.5
    // removed floating point and division operation

    //arguments: (highperiod_us) Input in microsecond. Time period pulse remain high. 
    //Minimum input is 1. In other words, minimum period is 2us. 

/**********************************
This functions generates PWM pulse
PWM pulse only generated when counter Enable is high

To use, link Enable pin of the PWM generator with desired control signal
Intended for use with function ( init_encoder_pos ) for position control of Delta Motor
************************************/
    uint16_t u,w = 0;

    const int PR0 = 0;
    const int PR1 = 1;
    const int LATCH_ON_READ = 0;
    const int COUNT_SOFTWARE = 1; // count direction -> software
    const int COUNT_DOWN = 1;
    const int CLK_INTERNAL = 0b10;     //clock source -> quadrature x4
    const int CEN_ON = 0b10;       //count enable -> hardware count control
    const int HW_CEN = 0;       //hardware count source -> CEN
    const int AUTOLOAD = 0b001; //autoload -> when rollover
    const int COUT_POL_NORMAL = 0;  //cout polarity -> normal
    const int COUT_SRC_RTGL = 1;    //cout source - RTGL

    u = (PR0 << 14) |
    (LATCH_ON_READ << 13) |
    (COUNT_SOFTWARE << 12) |
    (COUNT_DOWN << 11) |
    (CLK_INTERNAL << 9) |
    (CEN_ON << 7) |
    (HW_CEN << 5) |
    (AUTOLOAD << 2) |
    (COUT_POL_NORMAL<<1) |
    (COUT_SRC_RTGL);

    w = (PR1 << 14) | u;

    uint32_t period_count,high_count,count_hw,count_lw;
    if (period_us==0.0){
         //stop the motor
        count_hw =0;
        count_lw =0;
    }else if (period_us<0.25){
        printf("period too short");
        return -1;
    }else{
        
        period_count = (int)(27*period_us); //  27 = 27000000 MHz / 1000000  
        high_count = (int)(period_count/2) ;

        count_hw = (high_count&0xffff0000)>>16;
        count_lw = (high_count&0xffff);
        
    }

    //always load highword before low word
    //Load Preload Register 0
    outw(0x1C85, ADDR_REG(dev_add, REG_C0M + channel_number * 8));
    outw(count_hw, ADDR_REG(dev_add, REG_C0H + channel_number * 8));
    outw(count_lw, ADDR_REG(dev_add, REG_C0L + channel_number * 8));

    //Load Preload Register 1
    outw(0x5C85, ADDR_REG(dev_add, REG_C0M + channel_number * 8));
    outw(count_hw, ADDR_REG(dev_add, REG_C0H + channel_number * 8));
    outw(count_lw, ADDR_REG(dev_add, REG_C0L + channel_number * 8));
        
}

