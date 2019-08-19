// <editor-fold defaultstate="collapsed" desc="includes">
#include "xc.h"
#include "Noyau.h"
#include "0_Main.h"
#include "4_Alims.h"
// </editor-fold>
//variables globales
// <editor-fold defaultstate="collapsed" desc="">
UnionStateAlims State_alims;
V_ALIMS V_alims;
// </editor-fold>
//variables locales à la tâche
    unsigned char c1_4;
    unsigned int i1_4;
    float f1_4;
void TASK4 (void)
{
    switch (TASK[TASK4_ID].STEP)
    {
//------------------------------------START-------------------------------------
        case 0: // <editor-fold defaultstate="collapsed" desc="">
        {
            State_alims.All=0;  //les alims sont toutes initialisées à mauvaises, en attendant les 1ères mesures
            V_alims.V_pwr=0;
            V_alims.V_4v2=0;
            V_alims.V_3v3=0;
            V_alims.V_1v8=0;
            TASK[TASK4_ID].STEP++;
            break;
        }// </editor-fold>
//----------------------------------MESURES-------------------------------------
        case 1: // <editor-fold defaultstate="collapsed" desc="mesure V_PWR"> 
        {
            i1_4=ReadAdc(AN_VPWR);
            f1_4=(float)i1_4;
            TASK[TASK4_ID].STEP++;
            break;
        }
        case 2: //
        {
            f1_4=f1_4*VREF/ADC_MAX; //calcul
            V_alims.V_pwr=f1_4*PONTDIV_VPWR;
            if((V_alims.V_pwr>VPWR_MAX)||(V_alims.V_pwr<VPWR_MIN))  //comparaison avec les bornes
                State_alims.Indiv.VpwrOk=0;
            else
                State_alims.Indiv.VpwrOk=1;
            SET_WAIT(TASK4_ID,10);
            TASK[TASK4_ID].STEP++;
            break;
        }// </editor-fold>
        case 3: // <editor-fold defaultstate="collapsed" desc="mesure V_4V2"> 
        {
            i1_4=ReadAdc(AN_4V2);
            f1_4=(float)i1_4;
            TASK[TASK4_ID].STEP++;
            break;
        }
        case 4: //
        {
            f1_4=f1_4*VREF/ADC_MAX; //calcul
            V_alims.V_4v2=f1_4*PONTDIV_4V2;
            if((V_alims.V_4v2>V4V2_MAX)||(V_alims.V_4v2<V4V2_MIN))  //comparaison avec les bornes
                State_alims.Indiv.V4v2Ok=0;
            else
                State_alims.Indiv.V4v2Ok=1;
            SET_WAIT(TASK4_ID,10);
            TASK[TASK4_ID].STEP++;
            break;
        }// </editor-fold>
        case 5: // <editor-fold defaultstate="collapsed" desc="mesure V_3V3"> 
        {
            i1_4=ReadAdc(AN_3V3);
            f1_4=(float)i1_4;
            TASK[TASK4_ID].STEP++;
            break;
        }
        case 6: //
        {
            f1_4=f1_4*VREF/ADC_MAX; //calcul
            V_alims.V_3v3=f1_4*PONTDIV_3V3;
            if((V_alims.V_3v3>V3V3_MAX)||(V_alims.V_3v3<V3V3_MIN))  //comparaison avec les bornes
                State_alims.Indiv.V3v3Ok=0;
            else
                State_alims.Indiv.V3v3Ok=1;
            SET_WAIT(TASK4_ID,10);
            TASK[TASK4_ID].STEP++;
            break;
        }// </editor-fold>
        case 7: // <editor-fold defaultstate="collapsed" desc="mesure V_1V8"> 
        {
            i1_4=ReadAdc(AN_1V8);
            f1_4=(float)i1_4;
            TASK[TASK4_ID].STEP++;
            break;
        }
        case 8: //
        {
            f1_4=f1_4*VREF/ADC_MAX; //calcul
            V_alims.V_1v8=f1_4*PONTDIV_1V8;
            if((V_alims.V_1v8>V1V8_MAX)||(V_alims.V_1v8<V1V8_MIN))  //comparaison avec les bornes
                State_alims.Indiv.V1v8Ok=0;
            else
                State_alims.Indiv.V1v8Ok=1;
            TASK[TASK4_ID].PHASE=INITIALIZED;   //les 1ères mesures ont été effectuées
            SET_WAIT(TASK4_ID,PERIODE_MES_ALIMS);
            TASK[TASK4_ID].STEP=1;
            break;
        }// </editor-fold>
    }
}

/*******************************************************************************
*                               FONCTIONS                                      *
*******************************************************************************/
