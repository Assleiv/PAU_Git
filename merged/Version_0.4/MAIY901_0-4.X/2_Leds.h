#ifndef LEDS_H
#define LEDS_H

/*******************************************************************************
*                               DEFINES                                        *
********************************************************************************
*		SETTINGS        *
************************/
// <editor-fold defaultstate="collapsed" desc="">

// </editor-fold>
/************************
*		FIXED           *
************************/
// <editor-fold defaultstate="collapsed" desc="">
#define _LED1_V         LATBbits.LATB11
#define _LED1_R         LATDbits.LATD8
#define _LED2_V         LATDbits.LATD9
#define _LED2_R         LATDbits.LATD10
    #define LED1_R          _LED1_V=1; _LED1_R=0
    #define LED1_V          _LED1_V=0; _LED1_R=1
    #define LED1_O          _LED1_V=0; _LED1_R=0
    #define LED1_OFF        _LED1_V=1; _LED1_R=1
    #define LED2_R          _LED2_V=1; _LED2_R=0
    #define LED2_V          _LED2_V=0; _LED2_R=1
    #define LED2_O          _LED2_V=0; _LED2_R=0
    #define LED2_OFF        _LED2_V=1; _LED2_R=1
//STEPS
#define LEDS_INIT   SET_READY(TASK2_ID);TASK[TASK2_ID].STEP=0
#define LEDS_ALIMS_GOOD SET_READY(TASK2_ID);TASK[TASK2_ID].STEP=3
// </editor-fold>
/*******************************************************************************
*		                   VARIABLES GLOBALES                                  *
*******************************************************************************/
// <editor-fold defaultstate="collapsed" desc="">

// </editor-fold>
/*******************************************************************************
*                               FONCTIONS                                      *
*******************************************************************************/
// <editor-fold defaultstate="collapsed" desc="">
extern void TASK2(void);
// </editor-fold>

#endif