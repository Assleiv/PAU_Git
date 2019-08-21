#ifndef I2C_H
#define I2C_H

/*******************************************************************************
*                               DEFINES                                        *
********************************************************************************
*		SETTINGS        *
************************/
// <editor-fold defaultstate="collapsed" desc="">
#define TIMEOUT_I2C     200 //en ms, temps de traitement max avant déclanchement d'un défaut
#define BAUD_RATE_I2C   76  //à 16MHz : 100kHz->76, 400kHz->18
#define SIZE_I2C_RX     10  //en octet, taille du buffer de réception I2C
#define SIZE_I2C_TX     70  //en octet, taille du buffer de transmission I2C
// </editor-fold>
/************************
*		FIXED           *
************************/
// <editor-fold defaultstate="collapsed" desc="">
#define _INT_RTC            PORTFbits.RF6
#define I2C_SCL_SET_OUTPUT  TRISGbits.TRISG2=0
#define I2C_SCL_SET_INPOUT  TRISGbits.TRISG2=1
#define I2C_SDA_PORT        PORTGbits.RG3
#define _I2C_SCL            LATGbits.LATG2
#define I2C_DATA            I2C_writeBuffer[0]
#define PT_I2C_DATA         &I2C_writeBuffer[0]
#define ADR_8563_BASE_HEURE_DATE  0x02   //Registre de l'heure dans la RTC

#define ADDR_I2C_RTC        0xA2         //adresse I2C de la RTC
#define EEPROM              0b10100000   //adresse I2C de l'EEPROM
//STEP
#define ICSI2C_START    SET_READY(TASK6_ID);TASK[TASK6_ID].STEP=0;TASK[TASK6_ID].PHASE=AMBRION

struct STR_DATE
{
	unsigned char Heure;    // 6 bits
	unsigned char Minute;   // 7 bits
	unsigned char Seconde;  // 7 bits
	unsigned char Jour;     // 6 bits
	unsigned char Mois;     // 5 bits on a fait le bazar :5
	unsigned wd:3;     // 5 bits on a fait le bazar :3
	unsigned An:5;       // 8 Bits
};
extern struct STR_DATE Date; //Date envoyé
extern struct STR_DATE Date2;//Date reçu



// </editor-fold>
/*******************************************************************************
*		                   VARIABLES GLOBALES                                  *
*******************************************************************************/
// <editor-fold defaultstate="collapsed" desc="">
extern volatile unsigned int TimeOutI2c;
extern  unsigned char CodeErreurI2c;
// </editor-fold>
/*******************************************************************************
*                               FONCTIONS                                      *
*******************************************************************************/
// <editor-fold defaultstate="collapsed" desc="">
extern void TASK6(void);
extern unsigned char IdleI2c(volatile unsigned int* timeoutI2c);
extern unsigned char SendStartI2c(volatile unsigned int* timeoutI2c);
extern unsigned char SendAckI2c(unsigned char ack, volatile unsigned int* timeoutI2c);
extern unsigned char SendNackI2c(volatile unsigned int* timeoutI2c);
extern unsigned char SendRestartI2c(volatile unsigned int* timeoutI2c);
extern unsigned char SendAddI2c(unsigned char adresseSlave, unsigned char read_nwrite, volatile unsigned int* timeoutI2c);
extern unsigned char SendOctetI2c(unsigned char octet, volatile unsigned int* timeoutI2c);
extern unsigned char SendStopI2c(volatile unsigned int* timeoutI2c);
extern unsigned char AckI2cRecu(volatile unsigned int* timeoutI2c);
extern unsigned char MasterI2cReceiver(volatile unsigned int* timeoutI2c);
extern void WriteI2c(unsigned char adresseSlave, unsigned int registreSlave, unsigned char longueurData, unsigned char* dataTxI2C, unsigned char* codeErreur, volatile unsigned int* timeoutI2c);
extern void ReadI2c(unsigned char adresseSlave, unsigned int registreSlave, unsigned char longueurData, unsigned char* dataRxI2c, unsigned char* codeErreur, volatile unsigned int* timeoutI2c);
extern void DeblocageI2c(void);
extern void I2cInit(void);
extern void I2cEnd(void);
extern unsigned char LectureDateHeure(struct STR_DATE *pt);
extern unsigned char LectureRegistres_8563(unsigned char Adresse, unsigned char *DataRTC,unsigned char NbOctet);
extern unsigned char UCharBcdToUChar(unsigned char UcBcd);
extern unsigned char UCharToUCharBcd(unsigned char Uc);
extern unsigned char EcritureDateHeure(struct STR_DATE *pt);
extern unsigned char EcritureRegistres_8563(unsigned char Adresse, unsigned char *DataRTC,unsigned char NbOctet);
extern unsigned char StrCopyToChar(unsigned char *Out,unsigned char *In);
extern unsigned char ReturnError(unsigned char* CodeErreur);
// </editor-fold>

#endif