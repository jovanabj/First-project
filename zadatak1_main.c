/* File:   zadatak1_main.c
 * Author:  Jovana Bjelcevic, EE138/2017
 *          Jelena Lukic, EE141/2017
 *          Tamara Kovacevic, EE55/2017
 *
 * Zadatak: Realizovati sistem za proveru zaposlenih prilikom dolaska na posao, koji
 * se sastoji iz provere alkohola i lozinke. Lozinku je moguce pogresiti tri puta.
 * Nakon treceg puta smatra se da osoba ne zna lozinku i pristup joj nije dozvoljen. 
 * Takodje pristup nije dozvoljen i ako ima alkohola u krvi. 
 * Ako je lozinka ispravno uneta vrata, koja simulira servo motor u polozaju +90,
 * se automatski otvaraju i zatvaraju (servo motor u polozaju -90) nakon prolaska osobe, sto detektuje pir senzor.
 * 
 * LOZINKA: B2C3
 * 
 * Created on December 22, 2020, 4:16 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <p30fxxxx.h>
#include "zadatak1.h"
#include "glcd.h"
#define DRIVE_A PORTCbits.RC13 //touch
#define DRIVE_B PORTCbits.RC14


unsigned int sirovi0, sirovi1, sirovi2;
unsigned int temp0, temp1, stoperica=0, stoperica2=0;
unsigned int X, Y, x_vrednost, y_vrednost;
unsigned char tacna_lozinka=0, zatvorena_vrata_pir_senzorom=0;
unsigned char buff[4];
unsigned int j=0;
unsigned int pokusaj=0;

_FOSC(CSW_FSCM_OFF & XT_PLL4);//instruction takt je isti kao i kristal 10MHz
_FWDT(WDT_OFF);
//_FGS(CODE_PROT_OFF);



void __attribute__((__interrupt__)) _U1RXInterrupt(void) 
{
    IFS0bits.U1RXIF = 0;
} 
    
void __attribute__((__interrupt__)) _ADCInterrupt(void) 
{
	sirovi0=ADCBUF0;  //MQ3				
    sirovi1=ADCBUF1;  // touch
    sirovi2=ADCBUF2;  //touch
    
    temp0=sirovi1;
	temp1=sirovi2;
    IFS0bits.ADIF = 0;
} 
   
void __attribute__ ((__interrupt__)) _T1Interrupt(void) // svakih 100us, korisceno za PWM
{
	TMR1 =0;
    stoperica2++;//brojac za funkciju delay_100us
	IFS0bits.T1IF = 0; 
       
}

void __attribute__ ((__interrupt__)) _T2Interrupt(void) // svakih 1ms, korisceno za delay
{
	TMR2 =0;
    stoperica++;//brojac za funkciju delay_ms
	IFS0bits.T2IF = 0; 
       
}
    void WriteUART1(unsigned int data)
    {
	while (U1STAbits.TRMT==0);
    if(U1MODEbits.PDSEL == 3)
        U1TXREG = data;
    else
        U1TXREG = data & 0xFF;
    }
    
    void WriteUART1dec2string(unsigned int data)
    {
	unsigned char temp;

	temp=data/1000;
	WriteUART1(temp+'0');
	data=data-temp*1000;
	temp=data/100;
	WriteUART1(temp+'0');
	data=data-temp*100;
	temp=data/10;
	WriteUART1(temp+'0');
	data=data-temp*10;
	WriteUART1(data+'0');
    }
       
    void delay_100us(unsigned int vreme){
        stoperica2 = 0;
		while(stoperica2 < vreme);
    }
    
    void delay_ms(unsigned int vreme){
        stoperica = 0;
		while(stoperica < vreme);
    }
    
    void InitPins(void) //funkcija za inicijalizaciju pinova
    {
        ADPCFGbits.PCFG6=0; // ANALOGNI za MQ3
        TRISBbits.TRISB6=1;    //mq3
        TRISAbits.TRISA11=0;   // buzzer
        TRISCbits.TRISC13=0;   //touch screen
        TRISCbits.TRISC14=0;   //touch screen
        ADPCFGbits.PCFG8=0; // analogni za TOUCH
        TRISBbits.TRISB8=1;   //touch screen 
        ADPCFGbits.PCFG9=0; // analogni za TOUCH
        TRISBbits.TRISB9=1;   //touch screen
        ADPCFGbits.PCFG11=1; //digitalni pin za SERVO motor
        TRISBbits.TRISB11=0;
        ADPCFGbits.PCFG12=1; //digitalni pin za PIR senzor
        TRISBbits.TRISB12=1;
    }
       
    void Touch_Panel(void){
        
	DRIVE_A = 1;  //vode horizontalni tranzisoti
	DRIVE_B = 0;
     LATCbits.LATC13=1;
     LATCbits.LATC14=0;

	delay_ms(500); //cekamo jedno vreme da se odradi AD konverzija
	
	x_vrednost = temp0;//temp0 je vrednost koju nam daje AD konvertor na BOTTOM pinu		
    
	
     LATCbits.LATC13=0; // vode vertikalni tranzistori
     LATCbits.LATC14=1;
    DRIVE_A = 0;  
	DRIVE_B = 1;
	delay_ms(500); //cekamo jedno vreme da se odradi AD konverzija
	
	y_vrednost = temp1;// temp1 je vrednost koji nam daje AD konvertor LEFT pinu
	
    X =(x_vrednost-300)*0.0391;
	Y =(y_vrednost-895)*0.0277; //Skaliranje Y-koordinate
    
}
    
void RS232_putst(register const char *str)
{
    while((*str) != 0)
    {
        WriteUART1(*str);
        if(*str == 13) WriteUART1(10);
        if(*str == 10) WriteUART1(13);
        str++;
    }
}

void Write_GLCD(unsigned int data)
{
unsigned char temp;

temp=data/1000;
Glcd_PutChar(temp+'0');
data=data-temp*1000;
temp=data/100;
Glcd_PutChar(temp+'0');
data=data-temp*100;
temp=data/10;
Glcd_PutChar(temp+'0');
data=data-temp*10;
Glcd_PutChar(data+'0');
}

    void iscrtavanje_panela(void){
        GLCD_ClrScr();
            GoToXY(20,0);
            GLCD_Printf("UNESITE LOZINKU");
                        
            GoToXY(16,2);
            GLCD_Printf ("1");
            
            GoToXY(46,2);
            GLCD_Printf ("2");
 
            GoToXY(76,2);
            GLCD_Printf ("3");

            GoToXY(106,2);
            GLCD_Printf ("4");

            GoToXY(16,5);
            GLCD_Printf ("A");

            GoToXY(46,5);
            GLCD_Printf ("B");

            GoToXY(76,5);
            GLCD_Printf ("C");

            GoToXY(106,5);
            GLCD_Printf ("D");

            GoToXY(20,7);
        GLCD_Printf("LOZINKA:");
            
 //ZA KOORDINATE           
      /*  GoToXY(0,7);
        GLCD_Printf ("X=");
        GoToXY(9,7);
        Write_GLCD(X);
	        
		GoToXY(64,7);
        GLCD_Printf ("Y=");
        GoToXY(73,7);
        Write_GLCD(Y);*/
        }      
    
    void servo(void){
        delay_ms(2);
        int m=0;
        if(tacna_lozinka==0){
            for(m=0; m<10; m++){
            LATBbits.LATB11=1;
            delay_100us(10);
            LATBbits.LATB11=0;
            delay_100us(190);
            LATBbits.LATB11=1; 
            } 
            RS232_putst("Vrata su zatvorena. Pristup nije odobren.");
            WriteUART1(13);            
        }
        if(zatvorena_vrata_pir_senzorom==1){ //osoba je prosla kroz vrata
            GLCD_ClrScr(); //ZA NOVU OSOBU KOJA MOZDA NIJE PIJANA
            GoToXY(15,3);
            GLCD_Printf("PROVERA ALKOHOLA");
            delay_ms(8000);
            zatvorena_vrata_pir_senzorom=0;
            }
        if(tacna_lozinka==1){   
            RS232_putst("Vrata su se otvorila");
            WriteUART1(13);
            for(m=0; m<10; m++){ //vrata otvorena
            LATBbits.LATB11=1;
            delay_100us(20);
            LATBbits.LATB11=0;
            delay_100us(180);
            LATBbits.LATB11=1; 
            }
            delay_ms(5000);
            RS232_putst("Delay 5s");
            WriteUART1(13);
            if(PORTBbits.RB12){ //pir senzor
                RS232_putst("Pokret je detektovan");
                WriteUART1(13);
                tacna_lozinka=0; //da bi se vrata zatvorila
                RS232_putst("Osoba je pristupila objektu");
                zatvorena_vrata_pir_senzorom=1;               
        }
        } 
    }
             
    void lozinka(void) //funkcija za proveru ispravnosti lozinke
    {
            int i=0;
            
            if((X>1) && (X<30) && (Y>32) && (Y<62) ){ //1
                WriteUART1('1');         
                WriteUART1(13);
                buff[j]='1';
                j++;
            } 
            if((X>32) && (X<62) && (Y>32) && (Y<62) ){ //2
                WriteUART1('2');
                WriteUART1(13);
                buff[j]='2';
                j++;
            } 
            if((X>64) && (X<94) && (Y>32) && (Y<62) ){ //3
                WriteUART1('3');
                WriteUART1(13);
                buff[j]='3';
                j++;
            } 
            if((X>96) && (X<126) && (Y>32) && (Y<62) ){ //4
                WriteUART1('4');
                WriteUART1(13);       
                buff[j]='4';
                j++;
            }  
            if((X>1) && (X<30) && (Y>4) && (Y<30) ){ //A
                WriteUART1('A');
                WriteUART1(13);
                buff[j]='A';
                j++;
            } 
            if((X>32) && (X<62) && (Y>4) && (Y<30) ){ //B
                WriteUART1('B');
                WriteUART1(13);
                buff[j]='B';
                j++;
            } 
            if((X>64) && (X<94) && (Y>4) && (Y<30) ){ //C
                WriteUART1('C');
                WriteUART1(13);  
                buff[j]='C';
                j++;
            } 
            if((X>96) && (X<126) && (Y>4) && (Y<30) ){ //D
                WriteUART1('D');
                WriteUART1(13);        
                buff[j]='D';
                j++;
            }
            GoToXY(75,7);
            switch (j){
                case 1: GLCD_Printf("*");  break;
                case 2: GLCD_Printf("**");  break;
                case 3: GLCD_Printf("***");  break;
                case 4: GLCD_Printf("****"); delay_ms(1000);  break;                
                };
            if(j==4){
                j=0;
                
                if(buff[0]=='B' && buff[1]=='2' && buff[2]=='C' && buff[3]=='3'){  
                RS232_putst("Lozinka ispravno uneta");
                WriteUART1(13);
 
                 GLCD_ClrScr();
                 delay_ms(2);
                 GoToXY(30,3);
                 GLCD_Printf("DOBRO DOSLI");
                 tacna_lozinka=1;
                               
                 pokusaj=0; //ako pogodim resetuje pokusaj da sledeci korisnik moze gresiti
                } 
                else {
                    tacna_lozinka=0;
                 pokusaj++;
                 RS232_putst("Lozinka neispravno uneta");
                 WriteUART1(13);
                 
                 if(pokusaj==1 || pokusaj==2){
                 GLCD_ClrScr();
                 delay_ms(2);
                 GoToXY(8,3);
                 GLCD_Printf("LOZINKA NEISPRAVNA");
                 GoToXY(11,4);
                 GLCD_Printf("POKUSAJTE PONOVO");
                 delay_ms(1000);
                 }
                if(pokusaj==3){
                    GLCD_ClrScr();
                    delay_ms(2);
                    GoToXY(2,3);
                    GLCD_Printf("PRISTUP NIJE ODOBREN"); 
                    for(i=0; i<70; i++){
                    LATAbits.LATA11 = 1;
                    delay_ms(15);
                    LATAbits.LATA11=0;
                    delay_ms(3);
                    } 

                    pokusaj=0; // da bi sledeci korisnik mogao da pokusava
                    
                    GLCD_ClrScr();
                    GoToXY(15,3);                                      
                    GLCD_Printf("PROVERA ALKOHOLA"); //ceka novog korisnika
                    delay_ms(8000);
                }
                }                
            } 
    }
      
int main(int argc, char** argv) {
    InitPins();
    ADCinit();
    InitUART1();
    Init_T2();
    Init_T1();
    ConfigureAllPins();
    GLCD_LcdInit();
    
    GLCD_ClrScr();
    delay_ms(100);
    ADCON1bits.ADON=1;//pocetak AD konverzije 
                                            
    GLCD_ClrScr();
    GoToXY(15,3);            
    GLCD_Printf("PROVERA ALKOHOLA");
    delay_ms(8000);       
        
    while(1){
        int i=0;
        
        if(sirovi0>1000){  //ZA PIJANU OSOBU CUJE SE BUZZER I NA LCD PISE PRISTUP NIJE ODOBREN                 
            for(i=0; i<70; i++){
                GLCD_ClrScr();
                GoToXY(2,3);
                GLCD_Printf("PRISTUP NIJE ODOBREN");
                LATAbits.LATA11 = 1;
                delay_ms(15);
                LATAbits.LATA11=0;
                delay_ms(3);               
            }
            
            GLCD_ClrScr(); //ZA NOVU OSOBU KOJA MOZDA NIJE PIJANA
            GoToXY(15,3);
            GLCD_Printf("PROVERA ALKOHOLA");
            delay_ms(8000);
            }
            else {         //AKO OSOBA NIJE PIJANA         
            iscrtavanje_panela();
            lozinka();
            servo();            
            Touch_Panel(); 
            }                                
        
        WriteUART1dec2string(sirovi0);
        WriteUART1(13);
        delay_ms(200);
                            
    }
    
    return (EXIT_SUCCESS);
}

