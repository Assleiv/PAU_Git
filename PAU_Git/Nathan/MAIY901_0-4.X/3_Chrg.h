#ifndef CHRG_H
#define CHRG_H

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
#define CHRG_ON         LATBbits.LATB3=1    //permet de connecter le panneau solaire à la batterie (et donc d'autoriser la charge)
#define CHRG_OFF        LATBbits.LATB3=0
#define ALIM_ON         LATBbits.LATB2=1    //permet de connecter la batterie aux alimentations de la carte (et donc d'autoriser la batterie à débiter)
#define ALIM_OFF        LATBbits.LATB2=0
#define AN_SOL          5   //numéro channel correspondant au panneau solaire
#define AN_BATT         4   //numéro channel correspondant à la batterie
#define PONTDIV_SOL     11  //10k 1k
#define PONTDIV_BATT    11
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
extern void TASK3(void);
// </editor-fold>

#endif