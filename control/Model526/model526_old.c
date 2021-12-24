#include "model526.h"
#include <assert.h>

static struct s526_reg_ s526_reg; 
// Calibration Parameters. 
static double C_ref = 0; 
static double DACa[4] = {1.0, 1.0, 1.0, 1.0}; 
static double DACb[4] = {0.0, 0.0, 0.0, 0.0}; 
static int s526_init_flag = 0; 

int s526_init() 
{ 
    // Set permissions for the range of IO addresses
    // int ret = ioperm(S526_ADDR, S526_IOSIZE, 1);
    if(s526_init_flag == 0) 
    {
        int ret = iopl(3); 

        printf("Reading board id..."); 
        int init = s526_read_id(); 
        if((init == 0x526b) || (init == 0x526a)) 
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

int s526_read_id()
{
    //Read the id word
    int id = inw(S526_DEFAULT_ADDRESS + REG_EEC);
    return id;
}

uint16_t s526_read_eeprom_word(uint8_t addr)
{
    uint16_t w;
    uint16_t rw;
    const uint16_t sbit = 1 << 7;

    outw(sbit, ADDR_REG(REG_ISR));
    w = ((addr & 0b111111) << 3) | (2 << 1) | 1;

    outw(w, ADDR_REG(REG_EEC));


    printf("Reading EEPROM at address %04x\n", addr);
    while(!(sbit & inw(ADDR_REG(REG_ISR)))){}
    outw(sbit, ADDR_REG(REG_ISR));
    rw = inw(ADDR_REG(REG_EED));
    printf("Read done.\n");

    return rw;
}

double s526_read_adc_calib()
{
    uint16_t doublebuf[4]; 
    double calibval; 
    
    doublebuf[0] = s526_read_eeprom_word((uint8_t) 0x20);
    doublebuf[1] = s526_read_eeprom_word((uint8_t) 0x21);
    doublebuf[2] = s526_read_eeprom_word((uint8_t) 0x22);
    doublebuf[3] = s526_read_eeprom_word((uint8_t) 0x23);
    memcpy(&calibval, doublebuf, sizeof(double));

    return calibval;
}

double s526_read_dac_calib_a(int channel)
{
    int8_t start_addr;
    uint16_t doublebuf[4];
    double val;
    
    start_addr = channel*8;

    doublebuf[0] = s526_read_eeprom_word(start_addr);
    doublebuf[1] = s526_read_eeprom_word(start_addr + 1);
    doublebuf[2] = s526_read_eeprom_word(start_addr + 2);
    doublebuf[3] = s526_read_eeprom_word(start_addr + 3);

    memcpy(&val, doublebuf, sizeof(double));

    return val;    
}

double s526_read_dac_calib_b(int channel)
{
    int8_t start_addr;
    uint16_t doublebuf[4];
    double val;
    
    start_addr = channel*8;

    doublebuf[0] = s526_read_eeprom_word(start_addr + 4);
    doublebuf[1] = s526_read_eeprom_word(start_addr + 5);
    doublebuf[2] = s526_read_eeprom_word(start_addr + 6);
    doublebuf[3] = s526_read_eeprom_word(start_addr + 7);

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

void s526_adc_init(int *channels, int num_channels)
{    
    //Write setting to device
    s526_reg.ACR = 0x0;
    s526_reg.ACR = s526_gen_adc_conf(channels, num_channels);
    //printf("Writing: %4x\n", s526_reg.ACR);
    outw(s526_reg.ACR, ADDR_REG(REG_ADC));
    //Enable the ADC interrupt in the ISR
    s526_reg.IER |= ISR_ADC_DONE;
    //printf("Writing: %4x\n", s526_reg.IER);
    outw(s526_reg.IER, ADDR_REG(REG_IER));
    //Reset the ISR bit
    outw(ISR_ADC_DONE, ADDR_REG(REG_ISR));
}

//Function to read the adc of the model 526
void s526_adc_read(int *channels, int num_channels, double *adc_sample_array)
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
    outw(w, ADDR_REG(REG_ADC));

    //Wait for ISR bit to set signalling end of conversion
    //for(j=0; j<400; j++)
    for(;;)
    {
        if(ISR_ADC_DONE & inw(ADDR_REG(REG_ISR)))
        {
	    //printf("ADC INTR DETECTED at %d \n", j);
	    //printf("Resetting INTR bit...\n");
	    outw(ISR_ADC_DONE, ADDR_REG(REG_ISR));
	    break;
        }
    }

    //Loop over channels and read
    for(i=0; i<num_channels; i++)
    {
        //Set multiplexer bits and start conversion bit
        w = s526_reg.ACR | (channels[i] << 1);
        //printf("Writing: %4x\n", w);
        outw(w, ADDR_REG(REG_ADC));
        //printf("Writing: %4x\n", w);
        //Write converted value to output array
	adreg = inw(ADDR_REG(REG_ADD));
	adc_sample_array[i] = ADC_CONVERSION_FACTOR * 4.0 * adreg;
    }

    
}

//init DAC
void s526_dac_init()
{
    int idx;
    //reset DAC data
    uint16_t w=0x0008;
    outw(w, ADDR_REG(REG_DAC));
    //Enable the DAC interrupt in the ISR
    outw((s526_reg.IER | 0x02), ADDR_REG(REG_IER));

    DACa[0] = s526_read_dac_calib_a(0);
    DACa[1] = s526_read_dac_calib_a(1);
    DACa[2] = s526_read_dac_calib_a(2);
    DACa[3] = s526_read_dac_calib_a(3);

    DACb[0] = s526_read_dac_calib_b(0);
    DACb[1] = s526_read_dac_calib_b(1);
    DACb[2] = s526_read_dac_calib_b(2);
    DACb[3] = s526_read_dac_calib_b(3);
}

//Function to write to DAC
void s526_dac_write(double *vals, int nvals)
{
    uint16_t w=0x0000;
    int i, j;
    float a;
    double C;
    
    for(i = 0; i < nvals; i++)
    {
        // Select the channel
        w |= i << 1;
        outw(w, ADDR_REG(REG_DAC));
	
        // Write data to the channel
        //C = vals[i]*DACa[i] + DACb[i];
        C = vals[i]*3249.0 + 32768.0;
        //outw((uint16_t) (65535*(C + 10.0)/20.0), ADDR_REG(REG_ADD));
        outw((uint16_t) C, ADDR_REG(REG_ADD));

        // Start conversion
        w = 1;
        outw(w, ADDR_REG(REG_DAC));
        // Wait for ISR bit to set singalling end of conversion
        while(!(0x02 & inw(ADDR_REG(REG_ISR)))){}
        // Reset the interrupt bit
        outw(0x02, ADDR_REG(REG_ISR));
    }	 
}

/************************************************************************
 *************Function to initialize the digital I/O
 *@para: port, the port number needed to set
 *          0 1 2 3 4 5 6 7
 *       mode, input or output
 *          0-input  1-output
 *       interrupt_condition, interrupt condition, just for input using
 *          0-interrupt on a rising edge
 *          1-interrupt on a failing edge
 *@return: NULL
 *
 *@note: port 0 1 2 3 is group 1, can be set different interrupt condition
 *       port 4 5 6 7 is group 2, can ONLY set same interrupt condition
 ***********************************************************************
 */
void s526_digitalIO_init(int port, int mode, int interrupt_condition)
{
    if((port>=0)&&(port<=7))
    {
        if(port < 4)
        {
            s526_reg.DIO |= (mode << 10);
            if(mode == 0)
            {
                switch(port)
                {
                    case 0: s526_reg.DIO |= (interrupt_condition << 12);
                        break;
                    case 1: s526_reg.DIO |= (interrupt_condition << 13);
                        break;
                    case 2: s526_reg.DIO |= (interrupt_condition << 14);
                        break;
                    case 3: s526_reg.DIO |= (interrupt_condition << 15);
                        break;
                }
                outw(s526_reg.DIO, ADDR_REG(REG_DIO));
                s526_reg.IER |= (port << 8);
                outw(s526_reg.IER, ADDR_REG(REG_IER));
            }
        }
        else if(port > 3)
        {
            s526_reg.DIO |= (mode << 11);
            s526_reg.DIO |= (interrupt_condition << 8);
            outw(s526_reg.DIO, ADDR_REG(REG_DIO));
        }
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
 int s526_digitalIO_read(int port)
 {
     uint8_t ret=0;
     ret = 0xff & inw(ADDR_REG(REG_DIO));
     if( ret & (1 << port))
        return 1;
     else
         return 0;
 }

 /************************************************************************
 *************Function to write digital I/O
 *@para: port, the port number needed to set
 *          0 1 2 3 4 5 6 7
 *@return: NULL
 *
 *@note:
 ***********************************************************************
 */
void s526_digitalIO_write(int port, int set)
{
    s526_reg.DIO |= (set << port);
    outw(s526_reg.DIO, ADDR_REG(REG_DIO));
}

// Function to initialize counter for measuring pulse width.
void s526_init_pulse_timer(int channel_number) {
    const int mode1 = 0x55AC;
    const int mode2 = 0x35AC;
    const int control = 0xC80F;

    // Make sure that channel number is withing bounds.
    assert((channel_number <4) && (channel_number >=0));

    // Setup mode register
    outw(mode1, ADDR_REG(REG_C0M + 8*channel_number));
    // Zero the pre-load registers
    outw(0x0000, ADDR_REG(REG_C0L + 8*channel_number));
    outw(0x0000, ADDR_REG(REG_C0H + 8*channel_number));

    outw(mode2, ADDR_REG(REG_C0M + 8*channel_number));
    // Zero the pre-load registers.
    outw(0x0000, ADDR_REG(REG_C0L + 8*channel_number));
    outw(0x0000, ADDR_REG(REG_C0H + 8*channel_number));

    // Setup the control register
    outw(control, ADDR_REG(REG_C0C + 8*channel_number));
}

//Function to initialize encoder
void s526_encoder_init(int channel_number)
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
	(COUT_POL_NORMAL) |
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
    outw(w, ADDR_REG(REG_C0M + channel_number * 8));
    // Reset the counter
    outw(1<<15, ADDR_REG(REG_C0C + channel_number * 8));
}

//Function to read the encoder count.
int s526_encoder_read(int channel_number)
{
    int ret;
    // Always read low word first
    ret = inw(ADDR_REG(REG_C0L + 8 * channel_number));
    ret |= inw(ADDR_REG(REG_C0H + 8 * channel_number)) << 16;
    ret = (ret << 8)/256;

//    ret = inw(ADDR_REG(REG_C0H + 8 * channel_number)) << 16;
//    ret = inw(ADDR_REG(REG_C0L + 8 * channel_number));
    return ret;
}

int s526_counter_read(int channel_number) {
    int ret;

    // Make sure that channel number is withing bounds.
    assert((channel_number <4) && (channel_number >=0));

    // Read the low word first
    ret = inw(ADDR_REG(REG_C0L + 8*channel_number));
    ret |= inw(ADDR_REG(REG_C0H + 8*channel_number)) << 16;
    ret = (ret << 8)/256;

    return ret;
}

//Function to set counter control register
void s526_counter_set_control_status(int channel_number, int count_reset, int count_load, int count_arm, int latch_select,
                                 int intettupt_enable)
{
    switch(channel_number)
    {
    case 0:
        s526_reg.C0C |= (count_reset << 15);
        s526_reg.C0C |= (count_load << 14);
        s526_reg.C0C |= (count_arm << 13);
        s526_reg.C0C |= (latch_select << 10);
        s526_reg.C0C |= (intettupt_enable << 6);
        outw(s526_reg.C0C, ADDR_REG(REG_C0C));
        break;
    case 1:
        s526_reg.C1C |= (count_reset << 15);
        s526_reg.C1C |= (count_load << 14);
        s526_reg.C1C |= (count_arm << 13);
        s526_reg.C1C |= (latch_select << 10);
        s526_reg.C1C |= (intettupt_enable << 6);
        outw(s526_reg.C1C, ADDR_REG(REG_C1C));
        break;
    case 2:
        s526_reg.C2C |= (count_reset << 15);
        s526_reg.C2C |= (count_load << 14);
        s526_reg.C2C |= (count_arm << 13);
        s526_reg.C2C |= (latch_select << 10);
        s526_reg.C2C |= (intettupt_enable << 6);
        outw(s526_reg.C2C, ADDR_REG(REG_C2C));
        break;
    case 3:
        s526_reg.C3C |= (count_reset << 15);
        s526_reg.C3C |= (count_load << 14);
        s526_reg.C3C |= (count_arm << 13);
        s526_reg.C3C |= (latch_select << 10);
        s526_reg.C3C |= (intettupt_enable << 6);
        outw(s526_reg.C3C, ADDR_REG(REG_C3C));
        break;
    }
}

//Function to read counter status register
/* void s526_counter_control_status(int channel_number, int *INDEX_status, int *COUT_status, */
/*                                  int *capture_event_status) */
/* { */
/*     INDEX_status = 0x20 & inw(ADDR_REG(REG_C0C + channel_number * 8)); */
/*     COUT_status = 0x10 & inw(ADDR_REG(REG_C0C + channel_number * 8)); */
/*     capture_event_status = 0x0F & inw(ADDR_REG(REG_C0C + channel_number * 8)); */
/* } */
