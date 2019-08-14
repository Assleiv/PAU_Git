#ifndef MAIN_H
#define MAIN_H

/*******************************************************************************
*                               DEFINES                                        *
********************************************************************************
*		SETTINGS        *
************************/
// <editor-fold defaultstate="collapsed" desc="">
#define VSOFT_MSB	0           // 0.
#define VSOFT_LSB	4   		// 4
// </editor-fold>
/************************
*		FIXED           *
************************/
// <editor-fold defaultstate="collapsed" desc="">
#define V4V2_EN     LATBbits.LATB10=1
#define V4V2_DIS    LATBbits.LATB10=0
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
extern void TASK0(void);
// </editor-fold>

#endif