#ifndef R232_H
#define R232_H

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

// </editor-fold>
/*******************************************************************************
*		                   VARIABLES GLOBALES                                  *
*******************************************************************************/
// <editor-fold defaultstate="collapsed" desc="">
#define RS232_RX_SIZE   50
#define MAX_INTER_DATA_MS   10  //en ms, durée max entre la réception de 2 octet
#define _VCCSW_EN   LATCbits.LATC14 //enable RS485
#define _EXT1_EN    LATDbits.LATD3  // enable RS232
extern volatile unsigned char IndBuffRx2;
extern volatile unsigned char RS232Rx[RS232_RX_SIZE];
extern volatile unsigned char DataUart2Dispo;
extern volatile unsigned int TimeoutUart2;
extern volatile unsigned char BufferRxUart2[RS232_RX_SIZE];
extern volatile unsigned char LgRx2Tmp;
extern unsigned char LgRx2;
extern unsigned int CodeErrRecu;
// </editor-fold>
/*******************************************************************************
*                               FONCTIONS                                      *
*******************************************************************************/
// <editor-fold defaultstate="collapsed" desc="">
extern void TASK8(void);
extern void Init_Uart2(void);
extern void ReceiveUart2(void);
extern void GsmToUc2(volatile unsigned char* DataRx);
extern void DebutTraceDebugConsole(void);
extern void StrToConsole(char *data);
// </editor-fold>

#endif