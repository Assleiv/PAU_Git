// <editor-fold defaultstate="collapsed" desc="includes">
#include "xc.h"
//#if !defined(FCY)
//    #define FCY 16000000UL      // idem mais en Hz (nécessaire à la fonction delay)
//#endif
#include "Noyau.h"
#include "0_Main.h"
#include "6_IcsI2c.h"
#include "8_Com232.h"
#include <libpic30.h>
#include<stdio.h>
#include<string.h>
// </editor-fold>
//variables globales
// <editor-fold defaultstate="collapsed" desc="">
    volatile unsigned int TimeOutI2c;
// </editor-fold>
//variables locales à la tâche
    unsigned char c1_6;
    unsigned char CodeErreurI2c;
    unsigned char I2C_writeBuffer[SIZE_I2C_TX];
    unsigned char I2C_writeBuffer2[SIZE_I2C_TX];
    unsigned char I2C_readBuffer[SIZE_I2C_RX];
    unsigned char I2C_readBuffer2[SIZE_I2C_RX];
    unsigned char ErrPCF8363;
    struct STR_DATE Date;
    struct STR_DATE Date2;
    unsigned char MemTemp[256];
    
void TASK6 (void)
{
    switch (TASK[TASK6_ID].STEP)
    {
//------------------------------------INIT--------------------------------------
        case 0: // <editor-fold defaultstate="collapsed" desc="">
        {
            I2cInit();  //initialisation de l'I2c
            CodeErreurI2c=0;
            TimeOutI2c=20;
            TASK[TASK6_ID].STEP++;
            break;
        }
        case 1: //Test Ecriture EEPROM
        { 
            I2C_writeBuffer[0]='B';
            I2C_writeBuffer[1]='o';
            I2C_writeBuffer[2]='n';
            I2C_writeBuffer[3]='j';
            I2C_writeBuffer[4]='o';
            I2C_writeBuffer[5]='u';
            I2C_writeBuffer[6]='r';
            WriteI2c(EEPROM, 0, 7, I2C_writeBuffer, &CodeErreurI2c, &TimeOutI2c);
            ReturnError(&CodeErreurI2c);
            TASK[TASK6_ID].STEP++;
            SET_WAIT(TASK6_ID,10);
            break;
        }
        case 2: //Test Lecture EEPROM
        {
            ReadI2c(EEPROM,0,7,I2C_readBuffer,&CodeErreurI2c,&TimeOutI2c);
            ReturnError(&CodeErreurI2c);
            StrToConsole((char*)I2C_readBuffer);
            if(I2C_readBuffer[0]=='B')
            {
                TASK[TASK6_ID].PHASE=INITIALIZED;
            }
            else
            {
                TASK[TASK6_ID].PHASE=ENDEFAUT;
            }
            TASK[TASK6_ID].STEP++;
            SET_WAIT(TASK6_ID,10);
            break;
        }
        case 3:
        {
            Date.An=19; //initialisation de la date et de l'heure
            Date.Mois=8;
            Date.Jour=20;
            Date.wd=2;
            Date.Heure=16;
            Date.Minute=22;
            Date.Seconde=30;
            EcritureDateHeure(&Date); TASK[TASK6_ID].STEP++;
            SET_WAIT(TASK6_ID,10);
        }
    case 4:
    {
        LectureDateHeure(&Date2);
        char test[40];
        sprintf(test,"%02uh%02um%02us %02u %02u %02u %02u \n",Date2.Heure,Date2.Minute,Date2.Seconde,Date2.Jour,Date2.wd,Date2.Mois,Date2.An);
        StrToConsole(test); //affiche la date lue sur la console
        SET_WAIT(TASK6_ID,1000);
        TASK[TASK6_ID].STEP++;
        break;
    }
    case 5:
    {
        break;
    }
        // </editor-fold>
    }
}

/*******************************************************************************
*                               FONCTIONS                                      *
*******************************************************************************/

// <editor-fold defaultstate="collapsed" desc="fonctions I2C">
unsigned char IdleI2c(volatile unsigned int* TOut) //Attend que le bus soit prêt pour l'envoi d'une nouvelle donnée
{
    while(((I2C1CONL&0x001F)!=0)||(I2C1STATbits.TRSTAT==1))
    {
        if(*TOut==0)
        {
            return 1;
            StrToConsole("ERROR: TIME OUT IDLE");
        }
    }
    return 0;
}
unsigned char SendStartI2c(volatile unsigned int* TOut) //Envoi de la commande de start de communication
{
    if(IdleI2c(TOut)==1)
    {
        return 1;
    }
    I2C1CONLbits.SEN=1;
    while(I2C1CONLbits.SEN==1)
    {
        if(*TOut==0)
        {
            StrToConsole("ERROR: TIME OUT START");
            return 1;
        }
    }
    return 0;
}
unsigned char SendAckI2c(unsigned char ack, volatile unsigned int* TOut) //Envoi du 'Ack' attestant la réception d'un octet par le maître
{
    if(IdleI2c(TOut)==1)
    {
        return 1;
    }
    I2C1CONLbits.ACKDT=(unsigned int)ack; //0:Ack, 1:Nack
    I2C1CONLbits.ACKEN=1;
    return 0;
}
unsigned char SendNackI2c(volatile unsigned int* TOut) //Envoi du 'NAck' attestant la fin de ce qu'on voulait lire
{
    if(IdleI2c(TOut)==1)
    {
        return 1;
    }
    I2C1CONLbits.ACKDT=1;
    I2C1CONLbits.ACKEN=1;
    return 0;
}
unsigned char SendRestartI2c(volatile unsigned int* TOut) // Envoi de la fonction de restart entre deux communications distinctes
{
    if(IdleI2c(TOut)==1)
    {
        return 1;
    }
    I2C1CONLbits.RSEN=1;
    while(I2C1CONLbits.RSEN==1)
    {
        if(*TOut==0)
        {
            StrToConsole("ERROR: TIME OUT RESTART");
            return 1;
        }
    }
    return 0;
}



unsigned char SendAddI2c(unsigned char adresseSlave, unsigned char read_nwrite, volatile unsigned int* TOut) //Envoi de l'adresse de l'esclave
{
unsigned char Ret;
    
    if(IdleI2c(TOut)==1)
    {
        return 1;
    }
    if(read_nwrite==1)  //read
    {
        I2C1TRN=(adresseSlave)|0b00000001;
    }
    else    //write
    {
        I2C1TRN=(adresseSlave);
    }
    if(IdleI2c(TOut)==1)
    {
        return 1;
    }
    Ret=AckI2cRecu(TOut);
    if(Ret!=0)
    {
        StrToConsole("ERROR: TIME OUT ADDRESS");
        return 1;
    }
    return 0;
}


unsigned char SendOctetI2c(unsigned char octet, volatile unsigned int* TOut) //envoie un octet de donnée
{
    if(IdleI2c(TOut)==1)
    {
        return 1;
    }
    I2C1TRN=octet;
    if(IdleI2c(TOut)==1)
    {
        return 1;
    }
    if(AckI2cRecu(TOut)==1)
    {
        StrToConsole("ERROR: TIME OUT DATA");
        return 1;
    }
    return 0;
}
unsigned char SendStopI2c(volatile unsigned int* TOut) //Envoi du 'stop' signifiant la fin de communication
{
    if(IdleI2c(TOut)==1)
    {
        return 1;
    }
    I2C1CONLbits.PEN=1;
    while(I2C1CONLbits.PEN==1)
    {
        if(*TOut==0)
        {
            StrToConsole("ERROR: TIME OUT STOP");
            return 1;
        }
    }
    return 0;
}
unsigned char AckI2cRecu(volatile unsigned int* TOut) // Chercher si l'esclave a bien renvoyé le 'ack' pour savoir si il a bien lu
{
    if(IdleI2c(TOut)==1)
    {
        return 1;
    }
    
    while(I2C1STATbits.ACKSTAT==1)
    {
        if(*TOut==0)
        {
            StrToConsole("ERROR: TIME OUT ACK NOT RECEIVED");
            return 2;
        }
    }
    return 0;
}
unsigned char MasterI2cReceiver(volatile unsigned int* TOut) // Le maitre se prépare à recevoir des données
{
    if(IdleI2c(TOut)==1)
    {
        return 1;
    }
    I2C1CONLbits.RCEN=1;            // master as reveiver --> clock for receiving data (8 CLK)
    while(I2C1CONLbits.RCEN==1)
    {
        if(*TOut==0)
        {
            StrToConsole("ERROR: TIME OUT MASTER NOT READY TO RECEIVE");
            return 1;
        }
    }
    if(IdleI2c(TOut)==1)
    {
        return 1;
    }
    return 0;
}

//Fonction compilant toutes celles précédentes afin d'écrire à un esclave dans un registre particulier
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
//Fonction compilant toutes celles précédentes afin de lire les réponses d'un esclave en particulier
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
    *codeErreur=7;
    if(SendRestartI2c(timeoutI2c)==1)
    {
        return;
    }
    *codeErreur=8;
    if(SendAddI2c(adresseSlave,1,timeoutI2c)==1)
    {
        return;
    }
    *codeErreur=9;
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
    *codeErreur=6;
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
void I2cInit(void) //Initialise le maitre I2C
{
    I2C1STAT = 0x0;
    I2C1CONL = 0x8000;
    I2C1BRG = BAUD_RATE_I2C;// Baud Rate Generator Value 18;
        
}
void I2cEnd(void) // Désactive l'I2C
{
    I2C1CONLbits.I2CEN = 0; //I2C disable
}

unsigned char UCharBcdToUChar(unsigned char UcBcd) //Convertie les octets du décimal au BCD pour la RTC
{
unsigned char Retour;

Retour=(UcBcd&0xf0);
Retour=Retour>>4;
Retour=Retour*10;
UcBcd=(UcBcd&0x0f);
Retour=Retour+UcBcd;

return(Retour);
}



unsigned char LectureRegistres_8563(unsigned char Adresse, unsigned char *DataRTC,unsigned char NbOctet) //Lit dans un registre de la RTC
{
unsigned char EESlaveAddr;

CodeErreurI2c=0;
EESlaveAddr=ADDR_I2C_RTC;
ReadI2c(EESlaveAddr,Adresse,NbOctet,DataRTC,&CodeErreurI2c,&TimeOutI2c);
ReturnError(&CodeErreurI2c);
return 1;
}

unsigned char LectureDateHeure(struct STR_DATE *pt) // permet de lire la date et l'heure sur la RTC
{
unsigned char DataRTC[7];
unsigned char Retour;
    
memset(pt,0,sizeof(struct STR_DATE));
    
if(LectureRegistres_8563(ADR_8563_BASE_HEURE_DATE, DataRTC,7)==1) // Soucis, peut importe l'adresse, rien ne change.
  {
  pt->Heure=UCharBcdToUChar(DataRTC[2] &0x3f);
  pt->Minute=UCharBcdToUChar(DataRTC[1] &0x7f);
  pt->Seconde=UCharBcdToUChar(DataRTC[0] &0x7f);
  pt->Jour=UCharBcdToUChar(DataRTC[3] &0x3f);
  pt->wd = (DataRTC[4] &0x07);
  pt->Mois=UCharBcdToUChar(DataRTC[5] &0x1f);
  pt->An=UCharBcdToUChar(DataRTC[6]);
  
  if(DataRTC[0]>=128) 
    {
    // Horloge non définie
    Retour=2;
    }
  else
    {
    // Horloge définie
    Retour=1;
    }
  }
    else
  {
  Retour=0;
  }
return(Retour);
}

unsigned char EcritureRegistres_8563(unsigned char Adresse, unsigned char *DataRTC,unsigned char NbOctet) //Permet d'écrire dans un registre de la RTC
{
unsigned char EESlaveAddr;

EESlaveAddr=ADDR_I2C_RTC;

WriteI2c(EESlaveAddr,Adresse,NbOctet,DataRTC,&CodeErreurI2c,&TimeOutI2c);
ReturnError(&CodeErreurI2c);
return 1;

}

unsigned char EcritureDateHeure(struct STR_DATE *pt) //Fonction permetant d'écrire la date et l'heure dans la RTC
{
unsigned char DataRTC[7];

// Lit l'heure courante pour recuperer le week day ou l'heure si necessaire
//if((Wd>6)||(pt==0))
 // LectureRegistres_8563(ADR_8563_BASE_HEURE_DATE, DataRTC,7);
// Si week day passé en parametre, on met a jour
//if(Wd<7)
  //DataRTC[4]=Wd;

if(pt!=0)
  {
  DataRTC[2]=UCharToUCharBcd(pt->Heure);
  DataRTC[1]=UCharToUCharBcd(pt->Minute);
  DataRTC[0]=UCharToUCharBcd(pt->Seconde);
  DataRTC[3]=UCharToUCharBcd(pt->Jour);
  DataRTC[4]=UCharToUCharBcd(pt->wd);
  DataRTC[5]=UCharToUCharBcd(pt->Mois);
  DataRTC[6]=UCharToUCharBcd(pt->An);
  return(EcritureRegistres_8563(ADR_8563_BASE_HEURE_DATE, DataRTC,7));
  }
return 1;
//return(EcritureRegistres_8563(ADR_8563_BASE_HEURE_DATE, DataRTC,7));
}

unsigned char UCharToUCharBcd(unsigned char Uc) // convertie les octets de BCD à décimal
{
unsigned char Retour;

Retour=(Uc/10);
Retour=Retour<<4;
Uc   =(Uc % 10);
Retour=Retour+Uc;

return(Retour);
}

unsigned char ReturnError(unsigned char* CodeErreur) // Permet d'afficher l'erreur si elle a lieu sur la console pour le déboggage
{
    switch (*CodeErreur)
    {
        case 1:
        {
            StrToConsole("ERROR: BUS NOT READY");
            return 1;
            break;
        }
        case 2:
        {
            StrToConsole("ERROR: START NOT SEND");
            return 2;
            break;
        }
        case 3:
        {
            StrToConsole("ERROR: ADDRESS WRITE NOT SEND");
            return 3;
            break;
        }
        case 4:
        {
            StrToConsole("ERROR: REGISTER NOT SEND");
            return 4;
            break;
        }
        case 5:
        {
            StrToConsole("ERROR: DATA NOT SEND");
            return 5;
            break;
        }
        case 6:
        {
            StrToConsole("ERROR: STOP NOT SEND");
            return 6;
            break;
        }
        case 7:
        {
            StrToConsole("ERROR: RESTART NOT SEND");
            return 7;
            break;
        }
        case 8:
        {
            StrToConsole("ERROR: ADDRESS READ NOT SEND");
            return 8;
            break;
        }
        case 9:
        {
            StrToConsole("ERROR: DATA NOT RECEIVED");
            return 9;
            break;
        }
        default:
        {
            return 0;
            break;
        }
    }    
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
