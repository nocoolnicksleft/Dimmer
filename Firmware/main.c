
#include <12F629.h>

// #fuses HS,NOWDT,NOPROTECT,PUT

#use delay(clock=4508000)

struct ADCON_layout 
{
  int ADON : 1;
  int GO : 1;
  int CSH0 : 1;
  int CSH1 : 1;
  int reserved : 2;
  int VCFG : 1;
  int ADFM : 1;

};

struct ADCON_layout ADCON0;

#byte ADCON0 = 0x1F


#define COUNTER_MAX 65535

#define CYCLE_UNIT 2

#define CYCLE_FIRE 65422

#define HALF_PERIOD 62717

#define DUTY_THREASHOLD_OFF 975

unsigned int16 duty_cycle = 0;

unsigned int8 fire_state = 0;

unsigned int16 t1 = 0;
unsigned int16 t2 = 0;

unsigned int16 analog_val;

unsigned int1 adc_flagged;


#INT_RA
void zero_detection()
{
  if (input_state(PIN_A2)) {
     set_timer1(t1);
     output_high(PIN_A4);
     fire_state = 1;
   output_high(PIN_A1);
  } else {
   output_low(PIN_A1);
  }

  adc_flagged = 1;
}

#INT_TIMER1
void fire()
{
 if (duty_cycle < DUTY_THREASHOLD_OFF) {

   if (fire_state == 1) {
    set_timer1(CYCLE_FIRE);
    output_low(PIN_A4);
    fire_state = 2;
   } else if (fire_state == 2) {
    set_timer1(t2);
    output_high(PIN_A4);
    fire_state = 3;
   } else if (fire_state == 3) {
    set_timer1(CYCLE_FIRE);
    output_low(PIN_A4);
    fire_state = 4;
   } else if (fire_state == 4) {
    set_timer1(0);
    output_high(PIN_A4);
    fire_state = 0;
   }

 } else {
   output_high(PIN_A4);
 }
}


void set_duty_cycle(int16 newvalue)
{
 duty_cycle = newvalue;  
 t1 = COUNTER_MAX - (duty_cycle * CYCLE_UNIT);
 t2 = HALF_PERIOD;
}


void main()

{

 fire_state = 0;

 duty_cycle = 0;

 output_low(PIN_A1);
 output_high(PIN_A4);
 output_high(PIN_A5);
 
 setup_adc_ports(sAN0);
 setup_adc(ADC_CLOCK_DIV_16 | VSS_VDD);
 set_adc_channel(0);

 setup_timer_1(T1_INTERNAL | T1_DIV_BY_4);

 enable_interrupts(GLOBAL);
 enable_interrupts(INT_TIMER1);
 enable_interrupts(INT_RA2);

 delay_ms(200);

 output_low(PIN_A5);

 while(1)
 {
   restart_wdt();

   if (adc_flagged) {
    adc_flagged = 0;
    analog_val = (read_adc(ADC_START_AND_READ) | 0b11111111100 );
    set_duty_cycle(analog_val / (unsigned int16)66);
   }

 }

}
