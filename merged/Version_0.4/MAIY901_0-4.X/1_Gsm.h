#ifndef GSM_H
#define GSM_H 

#include"stdtype.h"
#include"pau_cst.h"

/*******************************************************************************
*                               DEFINES                                        *
********************************************************************************
*		SETTINGS        *
************************/
// <editor-fold defaultstate="collapsed" desc="">
#define GSM_RX_SIZE         250  //taille du buffer de réception uart correspondant au gsm
#define MAX_INTER_DATA_MS   10  //en ms, durée max entre la réception de 2 octets
#define NB_MAX_REQ_GSM      10  // nombre max de requêtes gsm en attente de traitement
#define CODE_PIN_D1 '0'   //digit 1
#define CODE_PIN_D2 '0'   //digit 2
#define CODE_PIN_D3 '0'   //digit 3
#define CODE_PIN_D4 '0'   //digit 4

#define REPONSE_AUTO		0 // à aller chercher dans un autre .h de fred
#define REPONSE_MANUELLE	1 // à aller chercher dans un autre .h de fred

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
typedef enum {NOTPWR, PWROFF, IDLE, ACTIVE, CONNECTED, INITIALAZING, SLEEP} GsmModeEnum;
typedef enum {GSM_NO_RESPONSE,GSM_POWER_ON,GSM_POWER_OFF,GSM_CONFIG, GSM_INITIALIZED, GSM_WAITING,GSM_ERROR,GSM_NO_SIGNAL, GSM_PRET, GSM_SLEEP} GsmStateEnum;
extern GsmStateEnum GsmState;
extern unsigned char IndGsmReq;
extern GsmModeEnum GsmMode;
extern unsigned int TimeOutGSM_T1;


extern unsigned char LigneGSMRecue[256]; // à trouver dans un autre .h
extern unsigned char NbRepCLCC;// à trouver dans un autre .h
extern unsigned char _OLD_GSM_State; // à trouver dans un autre .h

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

struct STR_CAR_ATTENDU 
{
    unsigned char Car;
    unsigned Attendu : 1;
    unsigned Recu : 1;
    unsigned b2 : 1;
    unsigned b3 : 1;
    unsigned b4 : 1;
    unsigned b5 : 1;
    unsigned b6 : 1;
    unsigned b7 : 1;
};

extern struct STR_CAR_ATTENDU CtrlCarAttendu;

struct STR_CTRL_OK 
{
    unsigned char Idx;
    unsigned Attendu : 1;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            
    unsigned Recu : 1;
    unsigned b2 : 1;
    unsigned b3 : 1;
    unsigned b4 : 1;
    unsigned b5 : 1;
    unsigned b6 : 1;
    unsigned b7 : 1;
};
    
extern struct STR_CTRL_OK CtrlOk;
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
extern unsigned char StrnCompare(unsigned char* Buff,const char * chain,unsigned char lengt);
extern unsigned char UnsignedStrnCompare(unsigned char*Buff,unsigned char *chain,unsigned char lengt);
extern unsigned char StrCopyToChar(unsigned char *Out,unsigned char *In);
extern unsigned char AscToHex(char Asc);
extern void PositionneAttendOk(void);
extern void ReceptionCaractereGSM(unsigned char Car);
// </editor-fold>

#endif