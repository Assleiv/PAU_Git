// <editor-fold defaultstate="collapsed" desc="includes">
#include "xc.h"
//#if !defined(FCY)
//    #define FCY 16000000UL      // idem mais en Hz (nécessaire à la fonction delay)
//#endif
#include "Noyau.h"
#include "0_Main.h"
#include "6_IcsI2c.h"
#include <libpic30.h>
// </editor-fold>
//variables globales
// <editor-fold defaultstate="collapsed" desc="">
    volatile unsigned int TimeOutI2c;
    unsigned char CodeErreurI2c;
// </editor-fold>
//variables locales à la tâche
    unsigned char c1_6;
    unsigned char I2C_writeBuffer[SIZE_I2C_TX];
    unsigned char I2C_readBuffer[SIZE_I2C_RX];
    
void TASK6 (void)
{
    switch (TASK[TASK6_ID].STEP)
    {
//------------------------------------INIT--------------------------------------
        case 0: // <editor-fold defaultstate="collapsed" desc="">
        {
            I2cInit();  //initialisation de l'I2c
            CodeErreurI2c=0;
            TimeOutI2c=0;
            TASK[TASK6_ID].STEP++;
            break;
        }
        case 1: //Test Ecriture /lecture (à retirer par la suite)
        {
            I2C_DATA = 22;  //juste pour vérifier que l'écriture/lecture fonctionne, (à retirer par la suite)
            WriteI2c(EEPROM, 0, 1, PT_I2C_DATA, &CodeErreurI2c, &TimeOutI2c);
            TASK[TASK6_ID].STEP++;
            SET_WAIT(TASK6_ID,10);
            break;
        }
        case 2: //
        {
            ReadI2c(EEPROM,0,1,&I2C_readBuffer[0],&CodeErreurI2c,&TimeOutI2c);
            if(I2C_readBuffer[0]==22)
            {
                TASK[TASK6_ID].PHASE=INITIALIZED;
            }
            else
            {
                TASK[TASK6_ID].PHASE=ENDEFAUT;
            }
            TASK[TASK6_ID].STEP++;
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

// <editor-fold defaultstate="collapsed" desc="fonctions I2C">
unsigned char IdleI2c(volatile unsigned int* titi)
{
    while(((I2C1CONL&0x001F)!=0)||(I2C1STATbits.TRSTAT==1))
    {
        if(*titi==0)
        {
            return 1;
        }
    }
    return 0;
}
unsigned char SendStartI2c(volatile unsigned int* titi)
{
    if(IdleI2c(titi)==1)
    {
        return 1;
    }
    I2C1CONLbits.SEN=1;
    while(I2C1CONLbits.SEN==1)
    {
        if(*titi==0)
        {
            return 1;
        }
    }
    return 0;
}
unsigned char SendAckI2c(unsigned char ack, volatile unsigned int* toto)
{
    if(IdleI2c(toto)==1)
    {
        return 1;
    }
    I2C1CONLbits.ACKDT=(unsigned int)ack; //0:Ack, 1:Nack
    I2C1CONLbits.ACKEN=1;
    return 0;
}
unsigned char SendNackI2c(volatile unsigned int* toto)
{
    if(IdleI2c(toto)==1)
    {
        return 1;
    }
    I2C1CONLbits.ACKDT=1;
    I2C1CONLbits.ACKEN=1;
    return 0;
}
unsigned char SendRestartI2c(volatile unsigned int* titi)
{
    if(IdleI2c(titi)==1)
    {
        return 1;
    }
    I2C1CONLbits.RSEN=1;
    while(I2C1CONLbits.RSEN==1)
    {
        if(*titi==0)
        {
            return 1;
        }
    }
    return 0;
}
unsigned char SendAddI2c(unsigned char adresseSlave, unsigned char read_nwrite, volatile unsigned int* tata)
{
    if(IdleI2c(tata)==1)
    {
        return 1;
    }
    if(read_nwrite==1)  //read
    {
        I2C1TRN=(adresseSlave<<1)|0b00000001;
    }
    else    //write
    {
        I2C1TRN=(adresseSlave<<1)&0b11111110;
    }
    if(IdleI2c(tata)==1)
    {
        return 1;
    }
    if(AckI2cRecu(tata)==1)
    {
        return 1;
    }
    return 0;
}
unsigned char SendOctetI2c(unsigned char octet, volatile unsigned int* tata)
{
    if(IdleI2c(tata)==1)
    {
        return 1;
    }
    I2C1TRN=octet;
    if(IdleI2c(tata)==1)
    {
        return 1;
    }
    if(AckI2cRecu(tata)==1)
    {
        return 1;
    }
    return 0;
}
unsigned char SendStopI2c(volatile unsigned int* titi)
{
    if(IdleI2c(titi)==1)
    {
        return 1;
    }
    I2C1CONLbits.PEN=1;
    while(I2C1CONLbits.PEN==1)
    {
        if(*titi==0)
        {
            return 1;
        }
    }
    return 0;
}
unsigned char AckI2cRecu(volatile unsigned int* toto)
{
    if(IdleI2c(toto)==1)
    {
        return 1;
    }
    while(I2C1STATbits.ACKSTAT==1)
    {
        if(*toto==0)
        {
            return 1;
        }
    }
    return 0;
}
unsigned char MasterI2cReceiver(volatile unsigned int* tata)
{
    if(IdleI2c(tata)==1)
    {
        return 1;
    }
    I2C1CONLbits.RCEN=1;            // master as reveiver --> clock for receiving data (8 CLK)
    while(I2C1CONLbits.RCEN==1)
    {
        if(*tata==0)
        {
            return 1;
        }
    }
    if(IdleI2c(tata)==1)
    {
        return 1;
    }
    return 0;
}
void WriteI2c(unsigned char adresseSlave, unsigned int registreSlave, unsigned char longueurData, unsigned char* dataTxI2C, unsigned char* codeErreur, volatile unsigned int* timeoutI2c)
{
    unsigned char i;
    *timeoutI2c=TIMEOUT_I2C;
    *codeErreur=1;
    if(IdleI2c(timeoutI2c)==1)
    {
        return;
    }
    *codeErreur=2;
    if(SendStartI2c(timeoutI2c)==1)
    {
        return;
    }
    *codeErreur=3;
    if(SendAddI2c(adresseSlave,0,timeoutI2c)==1)
    {
        return;
    }
    *codeErreur=4;
    switch (adresseSlave)   // en fonction de l'adresse (du composant donc) on va renseigner 1 ou 2 octets pour le registre
    {
        case(EEPROM):   //nécessite 2 octets d'adressage mémoire
        {
            i=registreSlave>>8;
            if(SendOctetI2c((unsigned char)i,timeoutI2c)==1)
            {
                return;
            }
            //break;    le fait de ne pas breaker va nous permettre de passer à default et donc d'envoyer le 2nd octet
        }
        default:
        {
            if(SendOctetI2c((unsigned char)registreSlave,timeoutI2c)==1)
            {
                return;
            }
            break;
        }
    }
    *codeErreur=5;
    for(i=0;i<longueurData;i++)
    {
        if(SendOctetI2c(dataTxI2C[i],timeoutI2c)==1)
        {
            return;
        }
    }
    *codeErreur=6;
    if(SendStopI2c(timeoutI2c)==1)
    {
        return;
    }
    *codeErreur=0;
}
void ReadI2c(unsigned char adresseSlave, unsigned int registreSlave, unsigned char longueurData, unsigned char* dataRxI2c, unsigned char* codeErreur, volatile unsigned int* timeoutI2c)
{
    unsigned char i;
    *timeoutI2c=TIMEOUT_I2C;
    *codeErreur=1;
    if(IdleI2c(timeoutI2c)==1)
    {
        return;
    }
    *codeErreur=2;
    if(SendStartI2c(timeoutI2c)==1)
    {
        return;
    }
    *codeErreur=3;
    if(SendAddI2c(adresseSlave,0,timeoutI2c)==1)
    {
        return;
    }
    *codeErreur=4;
    switch (adresseSlave)   // en fonction de l'adresse (du composant donc) on va renseigner 1 ou 2 octets pour le registre
    {
        case(EEPROM):   //nécessite 2 octets d'adressage mémoire
        {
            i=registreSlave>>8;
            if(SendOctetI2c((unsigned char)i,timeoutI2c)==1)
            {
                return;
            }
            //break;    le fait de ne pas breaker va nous permettre de passer à default et donc d'envoyer le 2nd octet
        }
        default:
        {
            if(SendOctetI2c((unsigned char)registreSlave,timeoutI2c)==1)
            {
                return;
            }
            break;
        }
    }
    *codeErreur=5;
    if(SendRestartI2c(timeoutI2c)==1)
    {
        return;
    }
    *codeErreur=6;
    if(SendAddI2c(adresseSlave,1,timeoutI2c)==1)
    {
        return;
    }
    *codeErreur=7;
    for(i=0;i<longueurData;i++)
    {
        if(MasterI2cReceiver(timeoutI2c)==1)
        {
            return;
        }
        dataRxI2c[i]=I2C1RCV;           // READ DATA
        if((i+1)==longueurData)         //à la dernière réception
        {
            if(SendAckI2c(1,timeoutI2c)==1) //Nack
            {
                return;
            }
            else {}
        }
        else
        {
           if (SendAckI2c(0,timeoutI2c)==1) //Ack
           {
                return;
           }
        }
    }
    *codeErreur=8;
    if(SendStopI2c(timeoutI2c)==1)
    {
        return;
    }
    *codeErreur=0;
}
void DeblocageI2c(void)
{
    I2C_SCL_SET_OUTPUT;
    while(I2C_SDA_PORT==0) //TANT QUE SDA = 0, on fait des clock sur SCL
    {
        //ATTENTION, la tempo doit etre calée en fonction de Fosc (pour faire des clock a 100kH ou 400kHz)
        _I2C_SCL=1;
        __delay_us(5);
        _I2C_SCL=0;
        __delay_us(5);
    }
    I2C_SCL_SET_INPOUT; //On remet SCL en entrée car sinon, on ne peut pas configurer l'I2c correctement (voir Doc)
}
void I2cInit(void)
{
    I2C1STAT = 0x0;
    I2C1CONL = 0x8000;
    I2C1BRG = BAUD_RATE_I2C;// Baud Rate Generator Value 18;
        
}
void I2cEnd(void)
{
    I2C1CONLbits.I2CEN = 0; //I2C disable
}
// </editor-fold>

//inline void i2c1_waitForEvent(uint16_t *timeout)
//{
//    //uint16_t to = (timeout!=NULL)?*timeout:100;
//    //to <<= 8;
//    if((IFS1bits.MI2C1IF == 0) && (IFS1bits.SI2C1IF == 0))
//    {
//        while(1)// to--)
//        {
//            if(IFS1bits.MI2C1IF || IFS1bits.SI2C1IF) break;
//            __delay_us(100);
//        }
//    }
//}
