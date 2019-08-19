#ifndef I2C_H
#define I2C_H

/*******************************************************************************
*                               DEFINES                                        *
********************************************************************************
*		SETTINGS        *
************************/
// <editor-fold defaultstate="collapsed" desc="">
#define TIMEOUT_I2C     100 //en ms, temps de traitement max avant déclanchement d'un défaut
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
#define EEPROM              0b01010000   //adresse I2C de l'EEPROM
#define EE_SLAVE_ADDR_2	0xA6
#define EE_SLAVE_ADDR_3	0xA4
#define MAX_EE_ADDR		0x07FF //Adresse la plus haute possible en EEPROM (ici 256Kb => 32ko => max 7FFF)
#define TEMPS_INTER_START_STOP 60 // x1000 cycle machines (doit faire 5ms) à 16MHz => 4Mips => 20

//STEP
#define ICSI2C_START    SET_READY(TASK6_ID);TASK[TASK6_ID].STEP=0;TASK[TASK6_ID].PHASE=AMBRION
// </editor-fold>
/*******************************************************************************
*		                   VARIABLES GLOBALES                                  *
*******************************************************************************/
// <editor-fold defaultstate="collapsed" desc="">
extern volatile unsigned int TimeOutI2c;
extern unsigned char CodeErreurI2c;
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
// </editor-fold>

#endif