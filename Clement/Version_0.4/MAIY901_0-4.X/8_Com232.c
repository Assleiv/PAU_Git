// <editor-fold defaultstate="collapsed" desc="includes">
#include "xc.h"
#include "Noyau.h"
#include "0_Main.h"
#include "8_Com232.h"
#include"1_Gsm.h"
// </editor-fold>
//variables globales
// <editor-fold defaultstate="collapsed" desc="">
//Console port série RS232
#define RS232PWD	SensRS485
#define RS232_ON	1
#define RS232_OFF	0
volatile unsigned char IndBuffRx2;
volatile unsigned char RS232Rx[RS232_RX_SIZE];
volatile unsigned char DataUart2Dispo;
volatile unsigned int TimeoutUart2;
volatile unsigned char BufferRxUart2[RS232_RX_SIZE];
volatile unsigned char LgRx2Tmp;
unsigned char LgRx2;
unsigned int CodeErrRecu;
// </editor-fold>
//variables locales à la tâche
    unsigned char c1_8;
    unsigned char c2_8;
    unsigned long l1_8;
    unsigned char RxUart2Tmp[RS232_RX_SIZE];  // tableau récupérant les données du buffer RX d'interruption (mais en décallage avec le temps partagé du noyau)
    unsigned char RxUart2[RS232_RX_SIZE]; // tableau contenant une trame complète selon le protocole
    unsigned char PtUart2;   // permettant le remplissage du tableau ci-dessus
    unsigned char FlagTrameUart2;    // flag de trame complète reçue
    unsigned char IndLectRx2;    // indice pour le dépilage rotatif du buffer de récpetion
    unsigned char DataRx2Overflow;   // pour signaler que la trame reçue est tronquée
    char c;
    
void TASK8 (void)
{
    switch (TASK[TASK8_ID].STEP)
    {
//------------------------------------INIT--------------------------------------
        case 0: // <editor-fold defaultstate="collapsed" desc="">
        {
            Init_Uart2();
            SET_WAIT(TASK8_ID,10);
            TASK[TASK8_ID].STEP++;
            break;
        }
        case 1: 
        {
            StrToConsole("Coucou \r");
            StrToConsole("blabla");
            SET_WAIT(TASK8_ID,10);
            TASK[TASK8_ID].STEP++;
            break;
        }
        case 2: //
        {
            U2TXREG=13;
            SET_WAIT(TASK8_ID,10);
            TASK[TASK8_ID].STEP++;
            break;
        }
        case 3:
        {
            c=0;
            //if(U2STAbits.URXDA==0)
            {
                c=U2RXREG;
                SET_WAIT(TASK8_ID,10);
                if (c!=0)
                    U2TXREG=c;
            }
            //TASK[TASK8_ID].STEP=1;
            break;
        }// </editor-fold>
        default:
        {
            TASK[TASK8_ID].STEP=0;
        }
    }
}

/*******************************************************************************
*                               FONCTIONS                                      *
*******************************************************************************/
void Init_Uart2(void)
{
    // STSEL 1; IREN disabled; PDSEL 8N; UARTEN enabled; RTSMD disabled; USIDL disabled; WAKE disabled; ABAUD disabled; LPBACK disabled; BRGH enabled; URXINV disabled; UEN TX_RX; Data Bits = 8; Parity = None; Stop Bits = 1;
    U2MODE = (0x8008 & ~(1<<15));  // disabling UARTEN bit
    // UTXISEL0 TX_ONE_CHAR; UTXINV disabled; URXEN disabled; OERR NO_ERROR_cleared; URXISEL RX_ONE_CHAR; UTXBRK COMPLETED; UTXEN disabled; ADDEN disabled; 
    U2STA = 0x00;
	// BaudRate = 115200; Frequency = 16000000 Hz; U2BRG 34; 
    U2BRG = 0x22;
    // ADMADDR 0; ADMMASK 0; 
    U2ADMD = 0x00;
    IEC1bits.U2RXIE = 1;
    //Make sure to set LAT bit corresponding to TxPin as high before UART initialization
    U2MODEbits.UARTEN = 1;
    U2STAbits.UTXEN = 1;
    U2STAbits.URXEN = 1;
    /*IPC16bits.U2ERIP = 1;   //UART2 Error PRIORITY 1
    IPC7bits.U2TXIP = 1;    //UART2 Transmitter PRIORITY 1
    IPC7bits.U2RXIP = 1;//UART2 Receiver PRIORITY 1*/
    _VCCSW_EN=1;
    _EXT1_EN=1; 
    for(c1_8=0;c1_8<RS232_RX_SIZE;c1_8++)
    {
        RS232Rx[c1_8]=0;
    }
    IndBuffRx2=0;
    DataUart2Dispo=0;
    IndLectRx2=0;
    DataRx2Overflow=0;
    TimeoutUart2=0;
    //CTS_PULLUP_ON;    //désactivé car le translateur ne fournit pas assez de courant !
}
void ReceiveUart2(void)  //fonction appelée en interruption qui récupère la data dans le registre de réception pour la mettre dans un buffer de réception
{
//    if(RC1STAbits.OERR == 1)    //cas erreur de réception
//    { 
//        RC1STAbits.SPEN = 0;    //restart
//        RC1STAbits.SPEN = 1;
//    }
    BufferRxUart2[IndBuffRx2] = U2RXREG;
    IndBuffRx2++;
    if(sizeof(BufferRxUart2) <= IndBuffRx2)   //buffer rotatif
    {
        IndBuffRx2 = 0;
    }
    DataUart2Dispo++;    //incrémente le nombre de data à récupérer
}
unsigned char ReadUart2(void)    //fonction qui, à chaque fois appelée, retourne la valeur du buffer de réception et pointe sur la case suivante
{
    unsigned char readValue=0;
    IEC1bits.U2RXIE = 0;
    readValue = BufferRxUart2[IndLectRx2];
    IndLectRx2++;
    if(sizeof(BufferRxUart2) <= IndLectRx2)   //buffer rotatif
    {
        IndLectRx2 = 0;
    }
    DataUart2Dispo--;    //décrémente le nombre de data à récupérer
    IEC1bits.U2RXIE = 1;
    return readValue;
}
void GsmToUc2(volatile unsigned char* DataRx)
{
    while(DataUart2Dispo>=1)
    {
        if(FlagTrameUart2==0)
        {
            if (TimeoutUart2>MAX_INTER_DATA_MS)	// au delà d'un temps entre 2 octets, on repart au 1er octet, il s'agit d'une sécurité au cas où on n'ait pas reçu le RC attendu d'une fin de trame
            {
                PtUart2=0;
            }
            DataRx[PtUart2]=ReadUart2();
            TimeoutUart2=0;
            if ((PtUart2>0)&&(DataRx[PtUart2]==0x0A)&&(DataRx[PtUart2-1]==0x0D))
            {
                FlagTrameUart2=1;	// à la réception de l'octet RC on signale la réception d'une trame à analyser
                LgRx2Tmp=PtUart2-1;
                PtUart2=0;
                break;  //on sort de la fonction, s'il y a un reste dans le buffer de réception il sera récupérer lors du prochain appel par la tache COM.
            }
            PtUart2++;
            if (PtUart2>RS232_RX_SIZE)  //tableau trop petit
            {
                PtUart2--;			// en cas de débordement on écrase la dernière case (la réception du prochain RC lèvera le flag de trame à analyser)
                DataRx2Overflow=1;   // pour signaler que la trame reçue est tronquée
            }
        }
        else	// dans le cas où la trame précédente n'a pas été traitée, on sort
        {
            TimeoutUart2=0;
            break;
        }
    }
}

void StrToConsole(unsigned char *data)
    {
        unsigned char x = 30;
        do 
        {
            while (U2STAbits.TRMT == 0); //Transmit Shift Register is not empty, a transmission is in progress or queued --> wait
            U2TXREG = *data; // Transmit a byte
            Tx[x] = *data;
            x++;
        } while (*++data);
        while (U2STAbits.TRMT == 0);
    }

    /*
     * void DebutTraceDebugConsole(void)
     * Envoi sur la console la chaine de début de debug
     * ENTREE Aucune
     * SORTIE Aucune
     * RETOUR Aucun
     */
    
void DebutTraceDebugConsole(void)
        {
            StrToConsole((unsigned char*)DEB_TRACE_DEBUG_CONS);
        }