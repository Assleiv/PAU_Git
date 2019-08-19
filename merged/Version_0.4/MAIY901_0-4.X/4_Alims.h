#ifndef ALIM_H
#define ALIM_H

/*******************************************************************************
*                               DEFINES                                        *
********************************************************************************
*		SETTINGS        *
************************/
// <editor-fold defaultstate="collapsed" desc="">
#define VPWR_MAX    15.0
#define VPWR_MIN    10.5
#define V4V2_MAX    4.3
#define V4V2_MIN    4.0
#define V3V3_MAX    3.5
#define V3V3_MIN    3.1
#define V1V8_MAX    1.9
#define V1V8_MIN    1.7
#define PERIODE_MES_ALIMS   1000    //période de scrutation des tensions d'alim (en ms)
// </editor-fold>
/************************
*		FIXED           *
************************/
// <editor-fold defaultstate="collapsed" desc="">
#define AN_VPWR         6   //numéro channel correspondant à l'entrée puissance (12v Batt à l'entrée des alims)
#define AN_4V2          7   //numéro channel correspondant à la tension de sortie du régulateur 4v2 (pour GSM)
#define AN_3V3          8   //numéro channel correspondant à la tension de sortie du régulateur 3v3
#define AN_1V8          9   //numéro channel correspondant à la tension de sortie du régulateur 1v8 (translateur de niveau µc/GSM)
#define PONTDIV_VPWR    11  //100k 10k
#define PONTDIV_4V2     11  //100k 10k
#define PONTDIV_3V3     11  //100k 10k
#define PONTDIV_1V8     11  //100k 10k
#define ALIMS_ALL_GOOD  0x0F
//STEPS
#define ALIMS_START     SET_READY(TASK4_ID);TASK[TASK4_ID].STEP=0;TASK[TASK4_ID].PHASE=AMBRION
// </editor-fold>
/*******************************************************************************
*		                   VARIABLES GLOBALES                                  *
*******************************************************************************/
// <editor-fold defaultstate="collapsed" desc="">
typedef struct
{
	float V_pwr;
	float V_4v2;
    float V_3v3;
    float V_1v8;
}V_ALIMS;
extern V_ALIMS V_alims;
typedef struct
{
    unsigned VpwrOk:1;
    unsigned V4v2Ok:1;
    unsigned V3v3Ok:1;
    unsigned V1v8Ok:1;
    unsigned unused:4;
}STATE_ALIMS;
typedef union
{
    STATE_ALIMS Indiv;
    unsigned char All;
}UnionStateAlims;
extern UnionStateAlims State_alims;
// </editor-fold>
/*******************************************************************************
*                               FONCTIONS                                      *
*******************************************************************************/
// <editor-fold defaultstate="collapsed" desc="">
extern void TASK4(void);
// </editor-fold>

#endif