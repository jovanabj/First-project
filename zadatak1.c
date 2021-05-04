#include<p30fxxxx.h>
#include "zadatak1.h"

#define TMR2_period 10000 /*  Fosc = 10MHz, 1/Fosc = 0.1us !!!, 0.1us * 10000 = 1ms  */
#define TMR1_period 1000 /* Fosc = 10MHz, 1/Fosc = 0.1us !!!, 0.1us * 1000 = 100us  */

void InitUART1(void)
{
U1BRG=0x0040;//baud rate 9600
U1MODEbits.ALTIO = 0; //ne koristimo alternativne pinove
IEC0bits.U1RXIE = 0;
U1STA&=0xfffc; 
U1MODEbits.UARTEN=1;
U1STAbits.UTXEN=1;
}

void Init_T1(void) 
{
	TMR1 = 0;
	PR1 = TMR1_period; //100us
	
	T1CONbits.TCS = 0; // 0 = Internal clock (FOSC/4)
	//IPC1bits.T2IP = 3 // T1 interrupt pririty (0-7)
	//SRbits.IPL = 3; // CPU interrupt priority is 3(11)
	IFS0bits.T1IF = 0; // clear interrupt flag
	IEC0bits.T1IE = 1; // enable interrupt

	T1CONbits.TON = 1; // T1 on 
}

void Init_T2(void) 
{
	TMR2 = 0;
	PR2 = TMR2_period; //1ms
	
	T2CONbits.TCS = 0; // 0 = Internal clock (FOSC/4)
	//IPC1bits.T2IP = 3 // T2 interrupt pririty (0-7)
	//SRbits.IPL = 3; // CPU interrupt priority is 3(11)
	IFS0bits.T2IF = 0; // clear interrupt flag
	IEC0bits.T2IE = 1; // enable interrupt

	T2CONbits.TON = 1; // T2 on 
}

void ADCinit(void)
{

//ADCON1
ADCON1bits.ADSIDL = 0;
ADCON1bits.FORM=0;
ADCON1bits.SSRC=7;
//na kraju  ADCON1bits.ASAM=1;
ADCON1bits.SAMP=1;
//ADCON1bits.ADON=1;      //ovo na kraju, bice u mainu definisano


//ADCON2

ADCON2bits.VCFG=7;
ADCON2bits.CSCNA=1;
ADCON2bits.SMPI=2; //prekid se zavrsava posle konverzija odbiraka sa 3 ulaza
ADCON2bits.BUFM=0;
ADCON2bits.ALTS=0;


//ADCON3:

ADCON3bits.SAMC=31;
ADCON3bits.ADRC=1;
ADCON3bits.ADCS=31;


//ADCHS: 

ADCHSbits.CH0NB=0;
ADCHSbits.CH0NA=0;
ADCHSbits.CH0SA=0;
ADCHSbits.CH0SB=0;

//ADCSSL: 
ADCSSL=0b0000001101000000; // RB6, RB8 i RB9 setovan kao analogni ulaz
ADCON1bits.ASAM=1; //setovanjem ovog bita vrsi se autokonverzija na kraju podesavanja svih registara

IFS0bits.ADIF=1; //interrupt flag
IEC0bits.ADIE=1; //dozvoljava ADC

}



