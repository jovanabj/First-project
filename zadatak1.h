/* 
 * File:   zadatak1.h
 * Author: Jovana Bjelcevic, EE138/2017
 *         Jelena Lukic, EE141/2017
 *         Tamara Kovacevic, EE55/2017
 *
 * Created on 28.12.2020.
 */

#ifndef ADCT_H
#define	ADCT_H

#ifdef	__cplusplus
extern "C" {
#endif

#include<p30fxxxx.h>

//inicijalizacija uarta
void InitUART1(void);
//void ADCinit_analog(void);
void ADCinit(void);
//inicijalizacija tajmera 1
void Init_T1(void);
//inicijalizacija tajemra tajemra2
void Init_T2(void);

#ifdef	__cplusplus
}
#endif

#endif	/* ADCT_H */
