//ext osc. 20MHz //CKDIV8

int main(void){

    //CTC mode (timer 1)
    TCCR1A = 0;
    TCCR1B |= (1<<WGM12) | (1<<CS10); //mode 4, prescaler /1
    OCR1A = (2500-1); //interrupt every 1ms 
    TIFR |= (1<<OCF1A); //clr existing flags

    while(1){
        while (!(TIFR &(1<<OCF1A)))
        TIFR |= (1<<OCF1A); //clr flags

        /* continue */

    }
    
    //set up PWM when alarm = 1    
    //initialize PWM, N=8
    ICR1 = 156; //TOP value
    OCR1A = 78; //50% duty cycle at equilibrium
    TCCR0A = (1<<WGM00) | (1<<COM0A1); //set OC0A on compare-match when down-count
    TCCR0B = (1<<WGM02) | (1<<CS01); //mode 5, prescaler /8

    return (0);
}

/* CTC mode initialization (timer 1)
freq calc:
f_OCR0A = f_clk/(2*N*(1+OCR0A))
f_OCR0A = 1ms = 1000Hz
f_clk = 20MHz/8 = 2.5MHz
N*(1+TOP) = 2500 ---------> N = 1
*\

/* PWM initialization (timer 0)
freq calc:
f_PWM = f_clk/(2*N*TOP)
f_PWM = 1ms = 1000Hz
f_clk = 20MHz/8 = 2.5MHz
N*TOP = 1250 ------> N = 8
TOP = 156
*\