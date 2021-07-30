#define PORTB *((volatile unsigned char*) 0x38)
#define DDRB *((volatile unsigned char*) 0x37)
#define PINB *((volatile unsigned char*) 0x36)
#define ADMUX *((volatile unsigned char*) 0x27)
#define ADCSRA *((volatile unsigned char*) 0x26)
#define ADCSRB *((volatile unsigned char*) 0x23)
#define ADCH *((volatile unsigned char*) 0x25)
#define ADCL *((volatile unsigned char*) 0x24)
#define DIDR0 *((volatile unsigned char*) 0x34)

#define BLINKS 3

int adc_res = 0b0; //variables for reading the ADC result registers

void blink(){
    for(unsigned char cycles = 0; cycles < BLINKS; cycles++){
        PORTB ^= (1<<1);
        for(volatile long time = 0; time < 10000; time++){
            PORTB &= (1<<1);
        }
        PORTB ^= (1<<1);
        for(volatile long time = 0; time < 10000; time++){
            PORTB &= (1<<1);
        }
    }

    if(ADCSRA & 1<<6){  //test if the ADC is running
        for(volatile long time = 0; time < 100000; time++){
            PORTB &= (1<<1);
        }   //some delay
        
        for(unsigned char cycles = 0; cycles < BLINKS; cycles++){
            PORTB ^= (1<<1);
            for(volatile long time = 0; time < 10000; time++){
                PORTB &= (1<<1);
            }
            PORTB ^= (1<<1);
            for(volatile long time = 0; time < 10000; time++){
                PORTB &= (1<<1);
            }
        }
    }
}

void brightness(int intensity){
    PORTB ^= (1<<1);
    for(volatile long time = 0; time < 1023-intensity; time++){
        PORTB &= (1<<1);
    }
    PORTB ^= (1<<1);
    for(volatile long time = 0; time < intensity; time++){
        PORTB &= (1<<1);
    }
}

int main(){
    //---Setup---//
    //Some of the registers are set to 0 as default, thus not mentioned here for saving some space
    DDRB |= (1<<1); //
    //DDRB = 2;
    //-------ADMUX setting---------
    //ADMUX |= (1<<5);    //Set ADLAR to 1 -> ADC results left-adjusted (datasheet p. 134)
    ADMUX |= (1<<1); //Set PB4 as the single ended input for the ADC
    //-------DIDR0 setting---------
    DIDR0 = (1<<4); //Disable digital input on ADC2 (PB4)
    //-------ADCSRB setting---------
    ADCSRB = 0; //free running mode, default value is 0 - the register probably doesn't have to be configured
    //-------ADCSRA setting---------
    ADCSRA |= (1<<7);   //Enable the ADC using ADEN bit
    ADCSRA |= (1<<6);   //Start the conversion (First 3 bits of ADCSRB are 0 -> free running mode -> ADC samples data all the time (datasheet p. 137))
    ADCSRA |= (1<<5);   //ADATE enable -> ADC auto trigger -> probably has to be on, otherwise the ADC doesn't react
    //ADCSRA |= (1<<3);   //Enable ADC interrupt mode
    
    while (1){
        while(PINB != 8){// || adc_res <= 0b1111101000){ //0b1111101000 == 1000 in DEC; need to shift ADCL 2 bits to the left because of the 10-bit value
            adc_res = ADCL;
            adc_res |= ADCH<<8;
            if(adc_res > 0){
                brightness(adc_res);
            }
        }
        blink();
    }
}
