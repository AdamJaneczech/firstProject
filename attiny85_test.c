#include <avr/io.h>
#include <avr/interrupt.h>

unsigned char adc_res = 0; //variables for reading the ADC result registers
unsigned char status = 0; //2nd bit -> 0 = ADC conversion, 1 = 1 blink / 1 sec; 1st bit -> PCINT

void blink(){
    //cli();
    PORTB ^= 1<<0;
    //sei();
}

void brightness(unsigned char intensity){
    if(intensity >= 252){
        OCR0A = 255;    //turn the LED off
    }
    else{
        OCR0A = intensity;  //8-bit values only -> set the PWM duty cycle
    }
}

ISR(PCINT0_vect){
    cli();
    if(status == 0b01 || status == 0b11){
        status ^= 1<<1;
        if(status == 0b01){
            TCCR0A = 0; //set the output pin to normal mode
        }
        else if(status == 0b11){
            TCCR0A = 0b11110011;    //first 2 digits -> fast PWM mode; last 4 ones -> PWM inverting mode set (with 1010 is PWM not inverted)
        }
    }
    status ^= 1<<0; //PCINT -> avoid double button clicking
    sei();
}

ISR(TIMER1_COMPA_vect){
    if(status == 0b00){
        blink();
    }
}

int main(){
    //-------SREG settings---------
    //SREG |= 1<<7;   //enable global interrupts using the I-bit (needs to be updated after an interrupt occurs); sei() could be probably used

    //---PIN SETTINGS---//
    //Some of the registers are set to 0 as default, thus not mentioned here for saving some space
    DDRB |= (1<<0); //LED pin -> OUTPUT
    //-------ADMUX setting---------
    ADMUX |= (1<<5);    //Set ADLAR to 1 -> ADC results left-adjusted (datasheet p. 134)
    ADMUX |= 1<<0;  //Set PB2 as the ADC input
    //-------DIDR0 setting---------
    DIDR0 = (1<<1); //Disable digital input on ADC1 (PB2)
    //-------ADCSRB setting---------
    ADCSRB = 0; //free running mode, default value is 0 - the register probably doesn't have to be configured
    //-------ADCSRA setting---------
    ADCSRA |= (1<<7);   //Enable the ADC using ADEN bit
    ADCSRA |= (1<<6);   //Start the conversion (First 3 bits of ADCSRB are 0 -> free running mode -> ADC samples data all the time (datasheet p. 137))
    ADCSRA |= (1<<5);   //ADATE enable -> ADC auto trigger -> probably has to be on, otherwise the ADC doesn't react
    
    //---TIMER SETTINGS---
    //-------TCCR0A setting---------
    TCCR0A = 0b11110011;    //first 2 digits -> fast PWM mode; last 4 ones -> PWM inverting mode set (with 1010 is PWM not inverted)
    //-------TCCR0B setting---------
    TCCR0B |= (1<<0);    //no prescaling set (PWM runs on system clock)
    //-------TCCR1 setting---------
    OCR1A = 244;    //count to 244 to get LED pin toggle every ~ 0.5 s with system clock 8 MHz & prescaler set to 16384 
    TCCR1 = 0b10001111; //set the Timer/Counter1 prescaler value to 16384 (currently synced mode selected)
    
    TIMSK |= 1<<6;
    //---INTERRUPT SETTINGS---
    //GIMSK |= 1<<6;  //Enable INT0
    GIMSK |= 1<<5;  //Enable PCINT
    PCMSK |= 1<<1;  //Enable PCINT1 for PB1
    //MCUCR |= 1<<1;
    //MCUCR |= 1<<0;  //Detect interrupts on rising edge (2 bits)

    sei();  //Enable interrupts
    while (1){
        if(status == 0b10){
            // || adc_res <= 0b1111101000){ //0b1111101000 == 1000 in DEC; need to shift ADCL 2 bits to the left because of the 10-bit value
            adc_res = ADCH; //read just ADCH with ADLAR in ADMUX set to 1 -> an 8-bit result
            //adc_res = ADCL;
            //adc_res |= ADCH<<8; -> for 10-bit results
            brightness(adc_res);
        }
        else if(status == 0b00){
            
        }
    }
}
