#ifndef GSM_H
#define GSM_H

/*******************************************************************************
*                               DEFINES                                        *
********************************************************************************
*		SETTINGS        *
************************/
// <editor-fold defaultstate="collapsed" desc="">
#define GSM_RX_SIZE         50  //taille du buffer de réception uart correspondant au gsm
#define MAX_INTER_DATA_MS   10  //en ms, durée max entre la réception de 2 octets
#define NB_MAX_REQ_GSM      10  // nombre max de requêtes gsm en attente de traitement
#define CODE_PIN_D1 '0'   //digit 1
#define CODE_PIN_D2 '0'   //digit 2
#define CODE_PIN_D3 '0'   //digit 3
#define CODE_PIN_D4 '0'   //digit 4
// </editor-fold>
/************************
*		FIXED           *
************************/
// <editor-fold defaultstate="collapsed" desc="">
#define _GSM_CTS        PORTEbits.RE5
#define _GSM_PWR_KEY    LATEbits.LATE3
#define GSM_PWR_KEY_ON  _GSM_PWR_KEY=1
#define GSM_PWR_KEY_OFF _GSM_PWR_KEY=0
#define CTS_PULLUP_ON   IOCPUEbits.IOCPE5=1
#define UART_GSM_NOT_READY  _GSM_CTS==1
#define NOREQENCOURS    255
#define GSM_INIT        SET_READY(TASK1_ID);TASK[TASK1_ID].STEP=0
#define GSM_ECOUTE      SET_READY(TASK1_ID);TASK[TASK1_ID].STEP=2;FlagTrameUart=0
#define GSM_PRETRAITEMENT_RECEPTION SET_WAIT(TASK1_ID,20);TASK[TASK1_ID].STEP=3
#define GSM_TRAITEMENT_REP_ATTENDUE SET_READY(TASK1_ID);TASK[TASK1_ID].STEP=4
#define GSM_TRAITEMENT_REP_NON_ATTENDUE SET_READY(TASK1_ID);TASK[TASK1_ID].STEP=100
#define GSM_TRAITEMENT_ERREUR   SET_WAIT(TASK1_ID,20);TASK[TASK1_ID].STEP=200
#define GSM_RESTART     SET_READY(TASK1_ID);TASK[TASK1_ID].STEP=250
// </editor-fold>
/*******************************************************************************
*		                   VARIABLES GLOBALES                                  *
*******************************************************************************/
// <editor-fold defaultstate="collapsed" desc="">
extern volatile unsigned char GsmRx[GSM_RX_SIZE];
extern volatile unsigned char DataUartDispo;
extern volatile unsigned int TimeoutUart;
extern volatile unsigned char BufferRxUart[GSM_RX_SIZE];
extern volatile unsigned char IndBuffRx;
extern volatile unsigned char LgRxTmp;
extern unsigned char LgRx;
extern unsigned int GsmCodeErrRecu;
typedef enum {NOTPWR, PWROFF, IDLE, ACTIVE, CONNECTED, INITIALAZING} GsmModeEnum;
typedef enum {OK, ERROR, NULL, AUTRE} GsmRepEnum;
typedef enum {PRESENCE_MODEM, ECHO_OFF, CFGERR, MODELE, NOREQ, IMEI, VSOFT, CCID, SIM_STAT, CODE_PIN, REBOOT, NETREG, COPS, SETCOPS, RSSI} GsmReqEnum;
typedef enum {GSM_NO_RESPONSE, GSM_INITIALIZED, GSM_WAITING} GsmStateEnum;
extern GsmStateEnum GsmState;
extern unsigned char IndGsmReq;
extern GsmModeEnum GsmMode;
extern GsmRepEnum GsmRepTypeRecu;
typedef struct
{
    GsmReqEnum Req;
    GsmRepEnum RepTypeAttendu;
    unsigned long Timeout;
}StructGsmReq;
extern StructGsmReq GsmReq[NB_MAX_REQ_GSM];
typedef struct
{
	unsigned char IMEI[21];
	unsigned char FirmwareVersion[21];
	unsigned char Modele[21];
	unsigned char CCID[31];
    unsigned char CodePin[5];
    unsigned char Rssi;
}INFGSM;
extern INFGSM InfoGSM;
extern unsigned char Tx[GSM_RX_SIZE];
// </editor-fold>
/*******************************************************************************
*                               FONCTIONS                                      *
*******************************************************************************/
// <editor-fold defaultstate="collapsed" desc="">
extern void TASK1(void);
extern void Init_Uart1(void);
extern void GsmToUc(volatile unsigned char* DataRx);
extern unsigned char ReadUart(void);
extern void ReceiveUart(void);
extern void StrPgmToGsm(const char *data);
extern void StrToGsm(unsigned char *data);
extern void InitVarGsm(void);
extern void CpTab(unsigned char* tabToCopy, unsigned char* tab, unsigned char lght);
extern unsigned long ConvertChaineEntier (unsigned char* chaine, unsigned char size);
extern void Disable_Uart1(void);
extern unsigned char StrCmpInBuff(unsigned char*Buff,const char * chain);
// </editor-fold>

#endif