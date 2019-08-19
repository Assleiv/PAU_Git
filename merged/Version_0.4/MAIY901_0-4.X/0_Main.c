// <editor-fold defaultstate="collapsed" desc="includes">
#include "xc.h"
#include "Noyau.h"
#include "0_Main.h"
#include "1_Gsm.h"
#include "2_Leds.h"
#include "3_Chrg.h"
#include "4_Alims.h"
#include "5_IoExt.h"
#include "6_IcsI2c.h"
#include "7_Com485.h"
#include "8_Com232.h"
#include "9_Audio.h"

// </editor-fold>
//variables globales
// <editor-fold defaultstate="collapsed" desc="">

// </editor-fold>
//variables locales à la tâche
    unsigned char c0_1;
    
void TASK0 (void)
{
    switch (TASK[TASK0_ID].STEP)
    {
//------------------------------------INIT--------------------------------------
        case 0: // <editor-fold defaultstate="collapsed" desc="">
        { 
            LEDS_INIT;
            ALIMS_START;
            TASK[TASK0_ID].STEP++;
            break;
        }
        case 1: //
        {
            if(TASK[TASK4_ID].PHASE==AMBRION)   //mesures non effectuées
                break;
            if(State_alims.Indiv.V3v3Ok==0) //alims µc en dehors de la plage autorisée
                break;
            V4V2_EN;    //activation de l'alimentation du GSM
            TASK[TASK0_ID].STEP++;
            break;
        }
        case 2: //
        {
            if(State_alims.All!=ALIMS_ALL_GOOD) //alims en dehors de leur plage de régulation
                break;
            LEDS_ALIMS_GOOD;
            GSM_INIT;
            ICSI2C_START;
            TASK[TASK0_ID].STEP++;
            break;
        }
        case 3:
        {
            if(GsmMode==ACTIVE)
            {
                LED2_O;
                CHRG_OFF;
            }
            else if(GsmMode==CONNECTED)
            {
                LED2_V;
                CHRG_ON;
            }
            else
            {
                LED2_R;
            }
            break;
        }
        // </editor-fold>
    }
}

/*******************************************************************************
*                               FONCTIONS                                      *
*******************************************************************************/
