// <editor-fold defaultstate="collapsed" desc="includes">
#include "xc.h"
#include "Noyau.h"
#include "0_Main.h"
#include "5_IoExt.h"
// </editor-fold>
//variables globales
// <editor-fold defaultstate="collapsed" desc="">

// </editor-fold>
//variables locales à la tâche
    unsigned char c1_5;
    
void TASK5 (void)
{
    switch (TASK[TASK5_ID].STEP)
    {
//------------------------------------INIT--------------------------------------
        case 0: // <editor-fold defaultstate="collapsed" desc="">
        { 
            TASK[TASK5_ID].STEP++;
            break;
        }
        case 1: //
        {
            SET_WAIT(TASK5_ID,100);
            break;
        }
        case 2: //
        {
            break;
        }
        case 3:
        {
            break;
        }// </editor-fold>
    }
}

/*******************************************************************************
*                               FONCTIONS                                      *
*******************************************************************************/
