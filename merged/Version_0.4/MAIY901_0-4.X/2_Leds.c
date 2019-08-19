// <editor-fold defaultstate="collapsed" desc="includes">
#include "xc.h"
#include "Noyau.h"
#include "0_Main.h"
#include "2_Leds.h"
// </editor-fold>
//variables globales
// <editor-fold defaultstate="collapsed" desc="">

// </editor-fold>
//variables locales à la tâche
    unsigned char c1_2;
    
void TASK2 (void)
{
    switch (TASK[TASK2_ID].STEP)
    {
//--------------------------------LEDS_INIT-------------------------------------
        case 0: // <editor-fold defaultstate="collapsed" desc="">
        { 
            LED1_V;
            LED2_V;
            SET_WAIT(TASK2_ID,500);
            TASK[TASK2_ID].STEP++;
            break;
        }
        case 1: //
        {
            LED1_O;
            LED2_O;
            SET_WAIT(TASK2_ID,500);
            TASK[TASK2_ID].STEP++;
            break;
        }
        case 2: //
        {
            LED1_R;
            LED2_R;
            SET_WAIT(TASK2_ID,500);
            TASK[TASK2_ID].STEP-=2;
            break;
        }// </editor-fold>
//--------------------------------LEDS_ALIMS_GOOD-------------------------------
        case 3: // <editor-fold defaultstate="collapsed" desc="">
        { 
            LED1_V;
            LED2_OFF;
            SET_STOP(TASK2_ID);
            break;
        }// </editor-fold>
    }
}

/*******************************************************************************
*                               FONCTIONS                                      *
*******************************************************************************/
