// <editor-fold defaultstate="collapsed" desc="includes">
#include "xc.h"
#include "Noyau.h"
#include "1_Gsm.h"
#include <string.h>
// </editor-fold>
// <editor-fold defaultstate="collapsed" desc="variables globales">
volatile unsigned char GsmRx[GSM_RX_SIZE];
volatile unsigned char DataUartDispo;   // disponibilité de donnée dans le buffer uart (déjà récupérées en interruption)
volatile unsigned int TimeoutUart;     // pour la gestion des temps inter-intra trame
volatile unsigned char BufferRxUart[GSM_RX_SIZE];  // tableau servant à récupérer les data dans l'uart hardware (en interruption)
volatile unsigned char IndBuffRx;   // indexant le remplissage du tableau ci-dessus (en interruption)
volatile unsigned char LgRxTmp;
unsigned char LgRx;
unsigned char IndReqEnCours;
GsmRepEnum GsmRepTypeRecu;
GsmModeEnum GsmMode=NOTPWR;
StructGsmReq GsmReq[NB_MAX_REQ_GSM];
unsigned char IndGsmReq=0;  //les requête gsm sont gérées dans un tableau indexé avec cette variable, 0 étant la requête la plus ancienne
INFGSM InfoGSM;
unsigned int GsmCodeErrRecu=0;
unsigned char Tx[GSM_RX_SIZE];
GsmStateEnum GsmState;
// </editor-fold>
// <editor-fold defaultstate="collapsed" desc="variables locales à la tâche">
    unsigned char c1_1;
    unsigned char c2_1;
    unsigned long l1_1;
    unsigned char RxUartTmp[GSM_RX_SIZE];  // tableau récupérant les données du buffer RX d'interruption (mais en décallage avec le temps partagé du noyau)
    unsigned char RxUart[GSM_RX_SIZE]; // tableau contenant une trame complète selon le protocole
    unsigned char PtUart;   // permettant le remplissage du tableau ci-dessus
    unsigned char FlagTrameUart;    // flag de trame complète reçue
    unsigned char IndLectRx;    // indice pour le dépilage rotatif du buffer de récpetion
    unsigned char DataRxOverflow;   // pour signaler que la trame reçue est tronquée
// </editor-fold>
    
void TASK1 (void)
{
    switch (TASK[TASK1_ID].STEP)
    {
//--------------------------------GSM_INIT--------------------------------------
        case 0: // <editor-fold defaultstate="collapsed" desc="initialisation Hard + Variables">
        {
            TASK[TASK1_ID].PHASE=AMBRION;
            GSM_PWR_KEY_ON;
            GsmMode=INITIALAZING;
            Init_Uart1();
            InitVarGsm();
            SET_WAIT(TASK1_ID,500);
            TASK[TASK1_ID].STEP++;
            break;
        }// </editor-fold>
        case 1: // <editor-fold defaultstate="collapsed" desc="tentative connexion Uart">
        {
            if(UART_GSM_NOT_READY)
                break;
            StrPgmToGsm("AT\r");GsmReq[IndGsmReq].Req=PRESENCE_MODEM;GsmReq[IndGsmReq].RepTypeAttendu=OK;GsmReq[IndGsmReq].Timeout=500;IndGsmReq++;//test Présence modem --> attendu "OK"
            GsmState=GSM_WAITING;
            GSM_ECOUTE;
            break;
        }// </editor-fold>
//--------------------------------GSM_ECOUTE------------------------------------
        case 2: // <editor-fold defaultstate="collapsed" desc="routine d'écoute">
        {
            if(DataUartDispo>=1)    //data à récupérer
            {
                GsmToUc(RxUartTmp);
                if(FlagTrameUart==1)	// Trame complète reçue
                {
                    c2_1=0; //initialisation de la détection d'une éventuelle erreur
                    for(c1_1=0;c1_1<LgRxTmp;c1_1++) // on récupère le buffer et on profite de la boucle for pour rechercher une éventuelle trame d'erreur
                    {
                        RxUart[c1_1]=RxUartTmp[c1_1];
                        if((c1_1>3)&&(c2_1==0))   // trame suffisament longue pour contenir une éventuelle erreur (non détectée pour le moment)
                        {
                            if((RxUart[c1_1-4]=='E')&&(RxUart[c1_1-3]=='R')&&(RxUart[c1_1-2]=='R')&&(RxUart[c1_1-1]=='O')&&(RxUart[c1_1]=='R'))
                            {
                                c2_1=c1_1+3; //détection de la trame d'erreur (positionnant le curseur sur le début du code erreur, car trame de type ...ERROR: ...)
                            }
                        }
                    }
                    LgRx=LgRxTmp;
                    if(LgRx==0)
                        GsmRepTypeRecu=NULL;
                    else if(c2_1>0) //si ERROR : on récupère le code erreur dans un 1er temps
                    {
                        GsmRepTypeRecu=ERROR;
                        GsmCodeErrRecu=(unsigned int)ConvertChaineEntier (&RxUart[c2_1], LgRx-c2_1);
                        GSM_TRAITEMENT_ERREUR;
                        break;
                    }
                    else if((RxUart[0]=='O')&&(RxUart[1]=='K'))
                        GsmRepTypeRecu=OK;
                    else
                        GsmRepTypeRecu=AUTRE;
                    GSM_PRETRAITEMENT_RECEPTION;
                }
            }
            else if(GsmState==GSM_NO_RESPONSE)
            {
                if(GsmReq[0].Req==PRESENCE_MODEM)
                    TASK[TASK1_ID].STEP--;
                else
                {
                    GSM_RESTART;
                }
            }
            break;
        }// </editor-fold>
//---------------------GSM_PRETRAITEMENT_RECEPTION------------------------------
        case 3: // <editor-fold defaultstate="collapsed" desc="permet d'aiguiller le traitement selon si la réponse reçue était attendue ou non">
        {
            IndReqEnCours=NOREQENCOURS; //indice nous permettant de pointer sur la requête associée à une des réponses attendues
            for(c1_1=0;c1_1<IndGsmReq;c1_1++)
            {
                if(GsmReq[c1_1].RepTypeAttendu==GsmRepTypeRecu)
                {
                    GsmReq[c1_1].Timeout=0; //on reset le timeout qui n'a plus lieu d'être, afin d'empêcher la génération d'une erreur de non réponse
                    IndReqEnCours=c1_1;
                    break;
                }
            }
            if(IndReqEnCours==NOREQENCOURS)
            {
                GSM_TRAITEMENT_REP_NON_ATTENDUE;    //si la réponse reçue ne correspond à aucune réponse attendue, on part au traitement associé
            }
            else
            {
                GSM_TRAITEMENT_REP_ATTENDUE;        //sinon on part au traitement associé à la requête
            }
            break;
        }// </editor-fold>
//---------------------GSM_TRAITEMENT_REP_ATTENDUE------------------------------
        case 4: // <editor-fold defaultstate="collapsed" desc="permet d'avancer les étapes d'une des requêtes en cours">
        {
            GSM_ECOUTE; //sauf cas contraire, on repartira en écoute
            switch(GsmReq[IndReqEnCours].Req)
            {
                case PRESENCE_MODEM:
                {
                    StrPgmToGsm("ATE0\r");GsmReq[IndGsmReq].Req=ECHO_OFF;GsmReq[IndGsmReq].RepTypeAttendu=OK;GsmReq[IndGsmReq].Timeout=2000;IndGsmReq++;   //désactive l'echo
                    break;
                }
                case ECHO_OFF:
                {
                    StrPgmToGsm("AT+CMEE=1\r");GsmReq[IndGsmReq].Req=CFGERR;GsmReq[IndGsmReq].RepTypeAttendu=OK;GsmReq[IndGsmReq].Timeout=2000;IndGsmReq++; // configure la gestion des erreurs pour retourner un code erreur
                    break;
                }
                case CFGERR:
                {
                    StrPgmToGsm("AT+CGMM\r");GsmReq[IndGsmReq].Req=MODELE;GsmReq[IndGsmReq].RepTypeAttendu=AUTRE;GsmReq[IndGsmReq].Timeout=2000;IndGsmReq++; // demande du modèle du modem
                    break;
                }
                case MODELE:
                {
                    CpTab(&RxUart[0],&InfoGSM.Modele[0],LgRx);  //récupération du modèle du module
                    StrPgmToGsm("AT+CGSN\r");GsmReq[IndGsmReq].Req=IMEI;GsmReq[IndGsmReq].RepTypeAttendu=AUTRE;GsmReq[IndGsmReq].Timeout=2000;IndGsmReq++; // demande de l'IMEI
                    break;
                }
                case IMEI:
                {
                    CpTab(&RxUart[0],&InfoGSM.IMEI[0],LgRx);  //récupération de l'IMEI (par défaut le numéro de série)
                    StrPgmToGsm("ATI9\r");GsmReq[IndGsmReq].Req=VSOFT;GsmReq[IndGsmReq].RepTypeAttendu=AUTRE;GsmReq[IndGsmReq].Timeout=2000;IndGsmReq++; //demande de la version de soft
                    break;
                }
                case VSOFT:
                {
                    CpTab(&RxUart[0],&InfoGSM.FirmwareVersion[0],LgRx);  //récupération de la version du modem, version application du module
                    StrPgmToGsm("AT+CCID\r");GsmReq[IndGsmReq].Req=CCID;GsmReq[IndGsmReq].RepTypeAttendu=AUTRE;GsmReq[IndGsmReq].Timeout=2000;IndGsmReq++; //demande du numéro de SIM
                    break;
                }
                case CCID:
                {
                    CpTab(&RxUart[7],&InfoGSM.CCID[0],LgRx);  //récupération du num de la SIM
                    StrPgmToGsm("AT+CLCK=\"SC\",2\r");GsmReq[IndGsmReq].Req=SIM_STAT;GsmReq[IndGsmReq].RepTypeAttendu=AUTRE;GsmReq[IndGsmReq].Timeout=2000;IndGsmReq++; // Get SIM security statut
                    break;
                }
                case SIM_STAT:  // si réponse c'est qu'il n'y a pas de code SIM, on peut donc considérer directement le case suivant
                {
                    //todo
                    //break;
                }
                case CODE_PIN:
                {
                    GsmMode=ACTIVE;
                    StrPgmToGsm("AT+CREG=1\r");GsmReq[IndGsmReq].Req=NETREG;GsmReq[IndGsmReq].RepTypeAttendu=OK;GsmReq[IndGsmReq].Timeout=2000;IndGsmReq++;  //enable network registration
                    break;
                }
                case NETREG:
                {
                    StrPgmToGsm("AT+COPS?\r");GsmReq[IndGsmReq].Req=COPS;GsmReq[IndGsmReq].RepTypeAttendu=AUTRE;GsmReq[IndGsmReq].Timeout=2000;IndGsmReq++;  //operator selection automatique ?
                    break;
                }
                case COPS:
                {
                    if( StrCmpInBuff(&RxUart[0],"+COPS: ")==1)
                    {
                        if(RxUart[7]!='0')  //la sélection de l'opérateur n'est pas automatique, alors on le set en auto
                        {
                            StrPgmToGsm("AT+COPS=0\r");GsmReq[IndGsmReq].Req=SETCOPS;GsmReq[IndGsmReq].RepTypeAttendu=OK;GsmReq[IndGsmReq].Timeout=2000;IndGsmReq++;  //operator selection en automatique
                            break;
                        }
                        else
                        {
                            //sans le break cela nous fais passer directement au case ci-dessous
                        }
                    }
                    else
                    {
                        //todo
                        break;
                    }
                }
                case SETCOPS:
                {
                    if(GsmMode!=CONNECTED)
                        GsmMode=ACTIVE;
                    TASK[TASK1_ID].PHASE=INITIALIZED;
                    GsmState=GSM_INITIALIZED;
                    break;
                }
//---------------------------FIN PARAMETRAGE GSM--------------------------------
                case RSSI:
                {
                    if( StrCmpInBuff(&RxUart[0],"+CSQ: ")==1)
                    {
                        for(c1_1=6; c1_1<LgRx; c1_1++)
                        {
                            if(RxUart[c1_1]==',')
                                break;
                        }
                        l1_1=ConvertChaineEntier (&RxUart[6], c1_1-6);
                        InfoGSM.Rssi=(unsigned char)l1_1;
                    }
                    else
                    {
                        //todo
                    }
                    break;
                }
                case REBOOT:    //coupure PWR_ON, 5s puis reinitialisation
                {
                    Disable_Uart1();
                    GSM_PWR_KEY_OFF;
                    GsmMode=NOTPWR;
                    GSM_INIT;
                    SET_WAIT(TASK1_ID,5000);
                    break;
                }
                default:    //ne devrait pas arriver selon condition du case précédent
                {
                    //todo
                    break;
                }
            }
            for(c1_1=IndReqEnCours;c1_1<IndGsmReq;c1_1++)   //on remonte la liste des requêtes suivantes
            {
                GsmReq[c1_1].Req=GsmReq[c1_1+1].Req;
                GsmReq[c1_1].RepTypeAttendu=GsmReq[c1_1+1].RepTypeAttendu;
                GsmReq[c1_1].Timeout=GsmReq[c1_1+1].Timeout;
            }
            IndGsmReq--;
            break;
        }// </editor-fold>
//---------------------GSM_TRAITEMENT_REP_NON_ATTENDUE--------------------------
        case 100: // <editor-fold defaultstate="collapsed" desc="permet le traitement d'une réponse non attendue">
        {
            if( StrCmpInBuff(&RxUart[0],"+CREG: ")==1)   //on vient de recevoir un changement d'état de la connexion réseau
            {
                if((RxUart[7]=='1')||(RxUart[7]=='5'))  //on est connecté -> on lance une requête du niveau de signal
                {
                    GsmMode=CONNECTED;
                    StrPgmToGsm("AT+CSQ\r");GsmReq[IndGsmReq].Req=RSSI;GsmReq[IndGsmReq].RepTypeAttendu=AUTRE;GsmReq[IndGsmReq].Timeout=2000;IndGsmReq++;  //niveau du signal ?
                } 
                else    //on est déconnecté -> Rssi à 0
                {
                    GsmMode=ACTIVE;
                    InfoGSM.Rssi=0;
                }
            }
            GSM_ECOUTE;
            break;
        }// </editor-fold>
//-------------------------GSM_TRAITEMENT_ERREUR--------------------------------
        case 200: // <editor-fold defaultstate="collapsed" desc="permet le traitement d'un code erreur reçu">
        {
            switch(GsmCodeErrRecu)
            {
                case 4: //operation not supported
                {
                    GSM_RESTART;
                    break;
                }
                case 10:    //SIM not inserted
                {
                    
                    break;
                }
                case 11:    //SIM PIN required
                {
                    StrPgmToGsm("AT+CPIN=\"");
                    StrToGsm(&InfoGSM.CodePin[0]);
                    StrPgmToGsm("\"\r");
                    GsmReq[IndGsmReq].Req=CODE_PIN;GsmReq[IndGsmReq].RepTypeAttendu=OK;GsmReq[IndGsmReq].Timeout=5000;IndGsmReq++; //// Send Pin Code
                    for(c2_1=0;c2_1<IndGsmReq;c2_1++)   //on cherche la requête SIM_STAT pour l'effacer, réglée par le renseignement du code pin
                    {
                        if(GsmReq[c2_1].Req==SIM_STAT)
                            break;
                    }
                    if(c2_1<=IndGsmReq) //requête trouvée
                    {
                        for(c1_1=c2_1;c1_1<IndGsmReq;c1_1++)   //on remonte la liste des requêtes suivantes
                        {
                            GsmReq[c1_1].Req=GsmReq[c1_1+1].Req;
                            GsmReq[c1_1].RepTypeAttendu=GsmReq[c1_1+1].RepTypeAttendu;
                            GsmReq[c1_1].Timeout=GsmReq[c1_1+1].Timeout;
                        }
                        IndGsmReq--;
                    }
                    GSM_ECOUTE;
                    break;
                }
                case 12:    //SIM PUK required
                {
                    
                    break;
                }
                case 13:    //SIM failure
                {
                    
                    break;
                }
                case 14:    //SIM busy
                {
                    
                    break;
                }
                case 15:    //SIM wrong
                {
                    
                    break;
                }
                case 16:    //Incorrect password
                {
                    
                    break;
                }
                case 17:    //SIM PIN2 required
                {
                    
                    break;
                }
                case 18:    //SIM PUK2 required
                {
                    
                    break;
                }
                default:
                {
                    //todo
                    GSM_ECOUTE;
                }
            }
            break;
        }// </editor-fold>
//-----------------------GSM_RESTART--------------------------------------------
        case 250: // <editor-fold defaultstate="collapsed" desc="arrêt et rebouclage sur l'init du gsm">
        {
            StrPgmToGsm("AT+CPWROFF\r");
            GsmReq[IndGsmReq].Req=REBOOT;GsmReq[IndGsmReq].RepTypeAttendu=OK;GsmReq[IndGsmReq].Timeout=40000;IndGsmReq=1; // requête d'extinction
            GSM_ECOUTE;
            break;
        }// </editor-fold>
//------------------------------------------------------------------------------
        default: // <editor-fold defaultstate="collapsed" desc="case non traité !!!!">
        {
            //todo
            break;
        }// </editor-fold>
    }
}

// <editor-fold defaultstate="collapsed" desc="Fonctions">
/*******************************************************************************
*                               FONCTIONS                                      *
*******************************************************************************/
void Init_Uart1(void)
{
    // STSEL 1; IREN disabled; PDSEL 8N; UARTEN enabled; RTSMD disabled; USIDL disabled; WAKE disabled; ABAUD disabled; LPBACK disabled; BRGH enabled; URXINV disabled; UEN TX_RX; Data Bits = 8; Parity = None; Stop Bits = 1;
    U1MODE = (0x8008 & ~(1<<15));  // disabling UARTEN bit
    // UTXISEL0 TX_ONE_CHAR; UTXINV disabled; URXEN disabled; OERR NO_ERROR_cleared; URXISEL RX_ONE_CHAR; UTXBRK COMPLETED; UTXEN disabled; ADDEN disabled; 
    U1STA = 0x00;
	// BaudRate = 115200; Frequency = 16000000 Hz; U1BRG 34; 
    U1BRG = 0x22;
    // ADMADDR 0; ADMMASK 0; 
    U1ADMD = 0x00;
    IEC0bits.U1RXIE = 1;
    //Make sure to set LAT bit corresponding to TxPin as high before UART initialization
    U1MODEbits.UARTEN = 1;
    U1STAbits.UTXEN = 1;
    for(c1_1=0;c1_1<GSM_RX_SIZE;c1_1++)
    {
        GsmRx[c1_1]=0;
    }
    IndBuffRx=0;
    DataUartDispo=0;
    IndLectRx=0;
    DataRxOverflow=0;
    TimeoutUart=0;
    //CTS_PULLUP_ON;    //désactivé car le translateur ne fournit pas assez de courant !
}
void ReceiveUart(void)  //fonction appelée en interruption qui récupère la data dans le registre de réception pour la mettre dans un buffer de réception
{
//    if(RC1STAbits.OERR == 1)    //cas erreur de réception
//    { 
//        RC1STAbits.SPEN = 0;    //restart
//        RC1STAbits.SPEN = 1;
//    }
    BufferRxUart[IndBuffRx] = U1RXREG;
    IndBuffRx++;
    if(sizeof(BufferRxUart) <= IndBuffRx)   //buffer rotatif
    {
        IndBuffRx = 0;
    }
    DataUartDispo++;    //incrémente le nombre de data à récupérer
}
void GsmToUc(volatile unsigned char* DataRx)
{
    while(DataUartDispo>=1)
    {
        if(FlagTrameUart==0)
        {
            if (TimeoutUart>MAX_INTER_DATA_MS)	// au delà d'un temps entre 2 octets, on repart au 1er octet, il s'agit d'une sécurité au cas où on n'ait pas reçu le RC attendu d'une fin de trame
            {
                PtUart=0;
            }
            DataRx[PtUart]=ReadUart();
            TimeoutUart=0;
            if ((PtUart>0)&&(DataRx[PtUart]==0x0A)&&(DataRx[PtUart-1]==0x0D))
            {
                FlagTrameUart=1;	// à la réception de l'octet RC on signale la réception d'une trame à analyser
                LgRxTmp=PtUart-1;
                PtUart=0;
                break;  //on sort de la fonction, s'il y a un reste dans le buffer de réception il sera récupérer lors du prochain appel par la tache COM.
            }
            PtUart++;
            if (PtUart>GSM_RX_SIZE)  //tableau trop petit
            {
                PtUart--;			// en cas de débordement on écrase la dernière case (la réception du prochain RC lèvera le flag de trame à analyser)
                DataRxOverflow=1;   // pour signaler que la trame reçue est tronquée
            }
        }
        else	// dans le cas où la trame précédente n'a pas été traitée, on sort
        {
            TimeoutUart=0;
            break;
        }
    }
}
unsigned char ReadUart(void)    //fonction qui, à chaque fois appelée, retourne la valeur du buffer de réception et pointe sur la case suivante
{
    unsigned char readValue=0;
    IEC0bits.U1RXIE = 0;
    readValue = BufferRxUart[IndLectRx];
    IndLectRx++;
    if(sizeof(BufferRxUart) <= IndLectRx)   //buffer rotatif
    {
        IndLectRx = 0;
    }
    DataUartDispo--;    //décrémente le nombre de data à récupérer
    IEC0bits.U1RXIE = 1;
    return readValue;
}
void StrPgmToGsm(const char *data)  //Send a string in flash to GSM module
{
    unsigned char x=0;
    do
    {  
      while(U1STAbits.TRMT == 0);  //Transmit Shift Register is not empty, a transmission is in progress or queued --> wait
      while(UART_GSM_NOT_READY);
      U1TXREG=*data;// Transmit a byte
      Tx[x]=*data;
      x++;
    }
    while( *++data );
    while(U1STAbits.TRMT == 0);
}
void StrToGsm(unsigned char *data)  //Send a string in ram (buff) to GSM module
{
    unsigned char x=30;
    do
    {  
      while(U1STAbits.TRMT == 0);  //Transmit Shift Register is not empty, a transmission is in progress or queued --> wait
      while(UART_GSM_NOT_READY);
      U1TXREG=*data;// Transmit a byte
      Tx[x]=*data;
      x++;
    }
    while( *++data );
    while(U1STAbits.TRMT == 0);
}
void InitVarGsm(void)
{
    IndReqEnCours=0;
    for(c1_1=0;c1_1<NB_MAX_REQ_GSM;c1_1++)
    {
        GsmReq[c1_1].Req=NOREQ;
        GsmReq[c1_1].RepTypeAttendu=NULL;
        GsmReq[c1_1].Timeout=0;
    }
    InfoGSM.CodePin[0]=CODE_PIN_D1;
    InfoGSM.CodePin[1]=CODE_PIN_D2;
    InfoGSM.CodePin[2]=CODE_PIN_D3;
    InfoGSM.CodePin[3]=CODE_PIN_D4;
    InfoGSM.CodePin[4]=0;
    InfoGSM.Rssi=99;
    GsmCodeErrRecu=0;
}
void CpTab(unsigned char* tabToCopy, unsigned char* tab, unsigned char lght)
{
    unsigned char ind;
    for(ind=0; ind<lght; ind++)
    {
        tab[ind]=tabToCopy[ind];
    }
}
unsigned long ConvertChaineEntier (unsigned char* chaine, unsigned char size)
{
    unsigned char w;
    unsigned char x;
    unsigned char y;
    unsigned long z;
    unsigned long resultat=0;
    for(x=0; x<size; x++)
    {
        y=chaine[x]-48;
        z=1;
        for(w=1; w<(size-x);w++)    //calcul puissance
        {
            z*=10;
        }
        resultat+=(z*(unsigned long)y);
    }
    return resultat;
}
void Disable_Uart1(void)
{
    U1MODEbits.UARTEN = 0;
    U1STAbits.UTXEN = 0;
}
//Compare a string inflash with a buffer in ram
unsigned char StrCmpInBuff(unsigned char*Buff,const char * chain)
{
	unsigned char lengt=0;
	unsigned char i=0;
	lengt=strlen(chain);
	for (i=0;i<lengt;i++)
	{
		if(Buff[i]!=chain[i])return 0;
	}
	return 1;
}
// </editor-fold>