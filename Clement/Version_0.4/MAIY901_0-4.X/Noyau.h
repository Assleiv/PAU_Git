#ifndef NOY_H
#define NOY_H

/*******************************************************************************
*                               DEFINES                                        *
********************************************************************************
*		SETTINGS        *
************************/
// <editor-fold defaultstate="collapsed" desc="">
#define OSC_FREQ    16000   //fréquence OSC en kHz, attention il ne s'agit pas ici d'un paramètre mais d'une info pour les autres taches
#define FCY 16000000UL      // idem mais en Hz (nécessaire à la fonction delay)
#define EEPROM_USINE    // si décommenté, le flashage de la carte réécrit toute l'EEPROM selon les valeurs du code
#define VREF        3.33 // tension de reference (pour adc)
#define ADC_MAX     1023    //10bits (pour calcul des tensions)
// </editor-fold>
/************************
*		FIXED           *
************************/
// <editor-fold defaultstate="collapsed" desc="">
//STATE Tâches
#define READY	0
#define WAIT	1
#define HALTE	2
//PHASE Tâches
#define AMBRION     0
#define INITIALIZED 1
#define ENCOURS     2
#define ENDEFAUT    3
#define END         4
//ID Tâches
#define TASK0_ID    0
#define TASK1_ID	1
#define TASK2_ID	2
#define TASK3_ID	3
#define TASK4_ID	4
#define TASK5_ID	5
#define TASK6_ID	6
#define TASK7_ID	7
#define TASK8_ID	8
#define TASK9_ID	9
//Nb Tâches
#define NB_TASK     10

#define MSQ_READY	1
#define MSQ_WAIT	2
// </editor-fold>
/*******************************************************************************
*		                   VARIABLES GLOBALES                                  *
*******************************************************************************/
// <editor-fold defaultstate="collapsed" desc="">
typedef struct 
{
	unsigned char STATE;
	unsigned int DELAY;
	unsigned char STEP;
    unsigned char PHASE;
}STR_TASK;

extern volatile STR_TASK TASK[NB_TASK];

extern unsigned char IdxCurLectRxGSM_IT_Rep;
extern unsigned char IdxCurRxGSM_IT;


// </editor-fold>
/*******************************************************************************
*                               FONCTIONS                                      *
*******************************************************************************/
// <editor-fold defaultstate="collapsed" desc="">
extern void SET_READY(unsigned char x);
extern void SET_STOP(unsigned char x);
extern void SET_WAIT(unsigned char x, unsigned int y);
extern void Init_Osc(void);  // initialisation de l'oscillateur
extern void Init_Pins(void); // initialisation des I/O microcontrôleur
extern void Init_Interrupt(void);    // initialisation des interruptions
extern void Init_Tasks(void);    //Configuration parametres taches au demarrage
extern void Init_Adc(void);     // initialisation du module adc
extern unsigned int ReadAdc(unsigned char channel); //fonction de lecture adc
// </editor-fold>
#endif