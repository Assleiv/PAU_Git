// <editor-fold defaultstate="collapsed" desc="includes">
#include "xc.h"
#include "Noyau.h"
#include "1_Gsm.h"
#include"8_Com232.h"
#include"Recherche.h"
#include"Conversion.h"
#include"Copy_Compare.h"
#include"pau_typ.h"
#include"pau_cst.h"
#include"pau_glo.h"
#include"Sim900D.h"
#include <string.h>
#include"ApiGsm.h"
// </editor-fold>
// <editor-fold defaultstate="collapsed" desc="variables globales">
volatile unsigned char GsmRx[GSM_RX_SIZE];
volatile unsigned char DataUartDispo; // disponibilité de donnée dans le buffer uart (déjà récupérées en interruption)
volatile unsigned int TimeoutUart; // pour la gestion des temps inter-intra trame
volatile unsigned char BufferRxUartGsm[GSM_RX_SIZE]; // tableau servant à récupérer les data dans l'uart hardware (en interruption)
volatile unsigned char IndBuffRx; // indexant le remplissage du tableau ci-dessus (en interruption)
volatile unsigned char LgRxTmp;
unsigned char LgRx;
unsigned char IndReqEnCours;
GsmModeEnum GsmMode = NOTPWR;
unsigned char IndGsmReq = 0; //les requête gsm sont gérées dans un tableau indexé avec cette variable, 0 étant la requête la plus ancienne
INFGSM InfoGSM;
unsigned int GsmCodeErrRecu = 0;
unsigned char Tx[GSM_RX_SIZE];
GsmStateEnum GsmState;
// </editor-fold>
// <editor-fold defaultstate="collapsed" desc="variables locales à la tâche">

unsigned char c1_1;
unsigned char c2_1;
unsigned long l1_1;
unsigned char RxUartTmp[GSM_RX_SIZE]; // tableau récupérant les données du buffer RX d'interruption (mais en décallage avec le temps partagé du noyau)
unsigned char RxUart[GSM_RX_SIZE]; // tableau contenant une trame complète selon le protocole
unsigned char PtUart; // permettant le remplissage du tableau ci-dessus
unsigned char FlagTrameUart; // flag de trame complète reçue
unsigned char IndLectRx; // indice pour le dépilage rotatif du buffer de récpetion
unsigned char DataRxOverflow; // pour signaler que la trame reçue est tronquée

unsigned char ReceptionSMS;
unsigned char LgnLigneGSM;      // Nb de car depuis CR ou LF
unsigned char LigneGSMRecue[256];
unsigned char MemLigneGSM[256];
unsigned char NbRepCLCC;        // Nombre de lignes de reponses CLCC pour detecter les fins de comm
unsigned char IdxCurRxGSM;
unsigned char CurRxGSM[SZ_TAB_ESP_GSM+1];
unsigned char CurRxGSM_IT[256];
unsigned char Gsm_MODELE[SZ_MODELE+1];
unsigned char Gsm_IMEI[SZ_IMEI+1];
unsigned char Gsm_ATI9[SZ_ATI9+1];
unsigned char Gsm_CCID[SZ_CCID+1];
unsigned char ChCmdModemTst[SZ_TRAME_MODEM_TST+1];
unsigned char NomOperateur[SZ_NOM_OPERATEUR+1];
unsigned int TimeOutGSM_T1;
unsigned int  TimeRSSI=0;
unsigned char ValReg;
unsigned char DefautInscription;          // Flag defaut inscription GSM géré par tache modem
unsigned char NbDefautInscription;        // Nombre de retour en defaut d'inscription successifs

unsigned char ModifAEModem;
unsigned char ModifSVRModem;
unsigned char NbInitGsm; 
unsigned int  TimeChkcall;
unsigned int  TimeChkcpin;
unsigned char TesterPIN;
unsigned int  TimeSMSList;
unsigned char _OLD_GSM_State; // Vérifier le comportement
unsigned char NbRebootTOModem;    // Nombre de retour en defaut d'inscription successifs
unsigned char NbToModemSucc;    // Nb de TO modem successif pour reboot
unsigned char CurDurCarDTMF;              // Duree caractere DTMF en 1/10 sec
unsigned char CurDurInterTrainDTMF_20ms;  // Duree entre 2 trains DTMF (* 20 ms)


struct STR_CAR_ATTENDU CtrlCarAttendu;
struct STR_CTRL_OK CtrlOk;
struct STR_SMS_ENTRANT SMSEntrant;
struct STR_STATUS StatusPau;
struct CALLin AppelEntrant;
struct CALLout AppelSortant;
struct Channel AudioChannel;
struct STR_SMS_SORTANT SMSSortant;
struct STR_DTMF_OUT DTMFOut;
struct GsmMod GsmModeFred;


// </editor-fold>



void AjouteTraceCharRxGSM(unsigned char cm)
{
//if(OptionsSoft&MSQ_OPTIONS_TRACE_MODEM)
  {
  if((cm==CR)||(cm==LF))
    {
    if(IdxCurRxGSM>0)
      {
      CurRxGSM[IdxCurRxGSM]=0;
      //StrPgmToConsole("\r\nR RXMOD ");
      //StrToConsole(CurRxGSM);
//      StrPgmToConsole("\n");
      IdxCurRxGSM=0;
      }
    // rajouter les lignes suivantes pour voir les CR LF
    CurRxGSM[IdxCurRxGSM]='<';
    IdxCurRxGSM++;
    if(cm==CR)
      {
      CurRxGSM[IdxCurRxGSM]='C';
      IdxCurRxGSM++;
      CurRxGSM[IdxCurRxGSM]='R';
      IdxCurRxGSM++;
      }
    else
      {
      CurRxGSM[IdxCurRxGSM]='L';
      IdxCurRxGSM++;
      CurRxGSM[IdxCurRxGSM]='F';
      IdxCurRxGSM++;
      }
    CurRxGSM[IdxCurRxGSM]='>';
    IdxCurRxGSM++;
    CurRxGSM[IdxCurRxGSM]=0;
    //StrToConsole(CurRxGSM);
    //StrPgmToConsole("\n");
    IdxCurRxGSM=0;
    }
  else
    {
    if(IdxCurRxGSM<SZ_TAB_ESP_GSM)
      {
      CurRxGSM[IdxCurRxGSM]=cm;
      IdxCurRxGSM+=1;
      }
    }
  }
}

void ReceptionCaractereGSM(unsigned char Car)
{
unsigned char EnteteSMS;  // 1 si ligne reçue commence par +CMGL:, 0 sinon -> 2 si entete corrcte
unsigned char IdxLigne;
unsigned char Lg;

AjouteTraceCharRxGSM(Car);

// Met a jour le flag du caractere special attendu
if(CtrlCarAttendu.Car==Car)
  {
  CtrlCarAttendu.Recu=1;
AjouteTraceCharRxGSM('\r');
  }

// interception de la reponse ok CR LF 0 K CR LF
//                               0  1  2 3 4  5 

if(CtrlOk.Attendu)
  {
  switch(Car)
    {
    case 'O':
      if(CtrlOk.Idx==2) 
        CtrlOk.Idx=3;
      else 
        CtrlOk.Idx=0;
      break;

    case 'K':
      if(CtrlOk.Idx==3) 
        CtrlOk.Idx=4;
      else 
        CtrlOk.Idx=0;
      break;

    case CR:
      if((CtrlOk.Idx==0)||(CtrlOk.Idx==4)) 
        {
        CtrlOk.Idx++;
        }
      else
        {
        CtrlOk.Idx=1;
        }
      break;

    case LF:
      if((CtrlOk.Idx==1)||(CtrlOk.Idx==5)) 
        {
        CtrlOk.Idx++;
        if(CtrlOk.Idx==6) 
          {
          CtrlOk.Recu=1;
          CtrlOk.Attendu=0;
          //GestionTimeOutModem(0);  // Ok attendu recu, on raz la detection blocage modem 
          ///!\ gestion des TO à gérer plus tard
          }
        }
      else
        {
        CtrlOk.Idx=0;
        }
      break;
    }
  }


if(Car==LF)
  {
  if((ReceptionSMS==1)&& (CtrlOk.Recu==1))  // Fin du SMS sur reception OK
    {
    SMSEntrant.ATraiter=1;
    SMSEntrant.Cpt+=1;
    // Gestion des compteurs journalier
    if(SMSEntrant.CptJour<255) SMSEntrant.CptJour+=1;

    ReceptionSMS=0;
    SMSEntrant.Plusieurs=0;
    }


  if(LgnLigneGSM>0)   // Le modem commence les reponses par cr lr
    {
    LigneGSMRecue[LgnLigneGSM]=0;

    // Ne recopie pas la ligne si c'est un OK
    if((CtrlOk.Idx!=5)&&(CtrlOk.Idx!=6)) 
      StrCopyToChar(MemLigneGSM,LigneGSMRecue);

    // Interception des SMS
    // Entete SMS +CMGL: 1,"REC READ","0899030019","","13/07/02,11:43:35+08"
    // Fin par autre entete ou OK     
   if((SMSEntrant.ATraiter==0)&&(SMSEntrant.AEffacer==0))    // Ajout && SMSEntrant.AEffacer==0 V140613, sur arrivee de rafales de SMS, effacement impossible car prise en compte suivant avant effacement 
     {
     if((LgnLigneGSM>7) && StrnCompare(LigneGSMRecue,"+CMGL: ",7))
       {
       EnteteSMS=1;
       PositionneAttendOk();
       if(ReceptionSMS==0)
         {
         // Debut d'un SMS que l'on va recuperer
         ReceptionSMS=1;
         // Demande de tout effacer si erreur
         SMSEntrant.ToutEffacer=1;

         // Recuperation des donnees de l'entete

         // Recupere index du SMS
         IdxLigne=7;
         SMSEntrant.ModuleIndex=0;
         while((IdxLigne<LgnLigneGSM) && LigneGSMRecue[IdxLigne] && (LigneGSMRecue[IdxLigne]!=','))
           {
           if((LigneGSMRecue[IdxLigne]>='0')&&(LigneGSMRecue[IdxLigne]<='9'))
             {
             SMSEntrant.ModuleIndex=SMSEntrant.ModuleIndex*10;
             SMSEntrant.ModuleIndex+=AscToHex(LigneGSMRecue[IdxLigne]);
             }
           IdxLigne+=1;
           }
         if(LigneGSMRecue[IdxLigne]==',')
           {
           // saute jusqu'a la prochaine ,
           do{
             IdxLigne+=1;
             }while(LigneGSMRecue[IdxLigne] && (LigneGSMRecue[IdxLigne]!=','));  
           if(LigneGSMRecue[IdxLigne]==',')
             {
             IdxLigne+=1;
             if(LigneGSMRecue[IdxLigne]=='"')
               {
               // Recupere le numero 
               Lg=0;
               IdxLigne+=1;
               while((IdxLigne<LgnLigneGSM) && LigneGSMRecue[IdxLigne] && (LigneGSMRecue[IdxLigne]!='"') && (Lg<SZ_NUM_TEL))
                 {
                 SMSEntrant.BuffNum[Lg]=LigneGSMRecue[IdxLigne];
                 IdxLigne+=1;
                 Lg+=1;
                 }
               SMSEntrant.BuffNum[Lg]=0;

               if((IdxLigne<LgnLigneGSM) && LigneGSMRecue[IdxLigne])
                 {
                 // saute jusqu'a la prochaine ,
                 do{
                   IdxLigne+=1;
                   }while(LigneGSMRecue[IdxLigne] && (LigneGSMRecue[IdxLigne]!=','));  

                 if(LigneGSMRecue[IdxLigne]==',')
                   {
                   // saute jusqu'a la prochaine ,
                   do{
                     IdxLigne+=1;
                     }while(LigneGSMRecue[IdxLigne] && (LigneGSMRecue[IdxLigne]!=','));  

                   // Recupere la date
                   if(LigneGSMRecue[IdxLigne]==',')
                     {
                     IdxLigne+=1;

                     if(LigneGSMRecue[IdxLigne]=='"')
                       {
                       IdxLigne+=1;
                       SMSEntrant.TxtLengt=0;   //  TxtLengt sert d'index sur buffdate et buffheure

                       while((IdxLigne<LgnLigneGSM) && LigneGSMRecue[IdxLigne] && (LigneGSMRecue[IdxLigne]!=',') && (SMSEntrant.TxtLengt<SZ_DATE_SMS))
                         {
                         SMSEntrant.BuffDate[SMSEntrant.TxtLengt]=LigneGSMRecue[IdxLigne];
                         IdxLigne+=1;
                         SMSEntrant.TxtLengt+=1;
                         }
                       SMSEntrant.BuffDate[SMSEntrant.TxtLengt]=0;

                       // saute jusqu'a la prochaine , (normalement on est dessus)
                       while(LigneGSMRecue[IdxLigne] && (LigneGSMRecue[IdxLigne]!=','))
                         {
                         IdxLigne+=1;
                         };  
 
                       if(LigneGSMRecue[IdxLigne]==',')
                         {
                         IdxLigne+=1;
                         SMSEntrant.TxtLengt=0;

                         while((IdxLigne<LgnLigneGSM) && LigneGSMRecue[IdxLigne] && (LigneGSMRecue[IdxLigne]!='+') && (SMSEntrant.TxtLengt<SZ_HEURE_SMS))
                           {
                           SMSEntrant.BuffHeure[SMSEntrant.TxtLengt]=LigneGSMRecue[IdxLigne];
                           IdxLigne+=1;
                           SMSEntrant.TxtLengt+=1;
                           }
                         SMSEntrant.BuffHeure[SMSEntrant.TxtLengt]=0;
                         // Entete ok, on n'efface pas
                         SMSEntrant.ToutEffacer=0;
                         }
                       }
                     }
                   }
                 } 
               }
             }
           }
         SMSEntrant.TxtLengt=0;
         SMSEntrant.BuffTxt[0]=0;
         EnteteSMS=2;
         }  // Fin reception et analyses entete SMS
       }
     else
       {
       EnteteSMS=0;
       if((CtrlOk.Recu==0)&&(ReceptionSMS==1))
         {
         // Données du SMS
         IdxLigne=0;
         while((IdxLigne<LgnLigneGSM)&&(SMSEntrant.TxtLengt<SZ_DATA_SMS))
           {
           SMSEntrant.BuffTxt[SMSEntrant.TxtLengt]= LigneGSMRecue[IdxLigne];
           IdxLigne+=1;
           SMSEntrant.TxtLengt+=1;
           }
         SMSEntrant.BuffTxt[SMSEntrant.TxtLengt]=0;
         }
       }

     // Fin du SMS sur entete suivante ou reception OK
     if((ReceptionSMS==1)&& ((EnteteSMS==1)||(CtrlOk.Recu==1)))  
       {
       SMSEntrant.ATraiter=1;
       SMSEntrant.Cpt+=1;
       ReceptionSMS=0;
       // Gestion des compteurs journalier
       if(SMSEntrant.CptJour<255) SMSEntrant.CptJour+=1;
       // Plusieurs SMS en attente, on accellerera la lecture
       if(EnteteSMS)
         SMSEntrant.Plusieurs=1;
       else
         SMSEntrant.Plusieurs=0;
       }
     }

    // FIN Interception des SMS
    if(ReceptionSMS==0)
      {
      // Interception des appels entrants
      if(StrnCompare(LigneGSMRecue,"+CLCC: ",7))
        {
        NbRepCLCC+=1;
        //+CLCC: 1,1,4,0,0,"0662175825",129,""    (appel entrant , ring)
        //+CLCC: 1,1,0,0,0,"0662175825",129,""    (pendant phonie
        //+CLCC: 1,0,2,0,0,"0662175825",129,""    (Appel sortant
        //           3                             sonnerie
        //           6                             Disque numero non attribue
        //+CLCC: 2,1,5,0,0,"+33494599591",145,""   // Appel entrant (1) en attente 5, trace recuperee pendant appel sortant
        //+clcc  1,0,2,0,0, nnnnnnnn
        }
    }
  }
  LgnLigneGSM=0;
}
else
  {
  if(Car!=CR)
    {
    LigneGSMRecue[LgnLigneGSM]=Car;
    LgnLigneGSM++;
    }
  }
}


void TASK1(void) 
{
    switch (TASK[TASK1_ID].STEP) 
    {
        case 0: // <editor-fold defaultstate="collapsed" desc="initialisation Hard + Variables">
        {
            GSM_PWR_KEY_ON;
            GsmMode = INITIALAZING;
            Init_Uart1();
            InitVarGsm();
            TASK[TASK1_ID].STEP = 1;
            SET_WAIT(TASK1_ID, 500);
            break;
        }
        
        // On commence à communiquer avec le modem-----------------------------------------------------------------
        case 1:
        {
            if (UART_GSM_NOT_READY)
                break;
            PositionneAttendOk();
            StrPgmToGsm("AT\r");
            TimeOutGSM_T1 = 5000;
            TASK[TASK1_ID].STEP = 2;
            SET_WAIT(TASK1_ID, 500);
            break;
        }
        
        case 2:
        {
            if(CtrlOk.Recu)
                {
                    PositionneAttendOk();
                    TimeOutGSM_T1=1000;	//TimeOut reponse gsm
                    StrPgmToGsm("AT&K0\r"); // On désactive le contrôle de flux DTE
                    TASK[TASK1_ID].STEP=3;
                }
            //Si TimeOut reprend en 18
            if(TimeOutGSM_T1==0)
                {
                    TASK[TASK1_ID].STEP=1;
                    //GestionTimeOutModem(1);  // Non reponse modem, on incremente le compteur de timeout
                }
            break;
        }
        
        case 3:
        {
            if(CtrlOk.Recu)
                {
                    PositionneAttendOk();
                    StrPgmToGsm("ATE0\r"); // On retire l'écho
                    TimeOutGSM_T1 = 5000;
                    TASK[TASK1_ID].STEP = 4;
                }            
            if (TimeOutGSM_T1==0)
                {
                    TASK[TASK1_ID].STEP = 2;
                    GsmState=GSM_ERROR;
                }
            SET_WAIT(TASK1_ID, 500);
            break;
        }
            
        case 4:
        {
            if(CtrlOk.Recu)
                {
                    PositionneAttendOk();
                    StrPgmToGsm("AT+IPR=115200\r");
                    TimeOutGSM_T1 = 5000;
                    TASK[TASK1_ID].STEP = 5;
                }            
            if (TimeOutGSM_T1==0)
                {
                    TASK[TASK1_ID].STEP = 3;
                    GsmState=GSM_ERROR;
                }
            SET_WAIT(TASK1_ID, 500);
            break;
        }
        //Connexion au reseau--------------------------------------------------------------------------------------------
        case 5:
        {
            if(CtrlOk.Recu)
                {                  
                    PositionneAttendOk();
                    MemLigneGSM[0]=0;
                    StrPgmToGsm("AT+CCID\r");
                    TimeOutGSM_T1 = 5000;
                    TASK[TASK1_ID].STEP = 6;
                }            
            if (TimeOutGSM_T1==0)
                {
                    TASK[TASK1_ID].STEP = 4;
                    GsmState=GSM_ERROR;
                }
            SET_WAIT(TASK1_ID, 500);
            break;
        }

        case 6:
        {
            if(MemLigneGSM[0])
                {
                    MemLigneGSM[SZ_CCID]=0;
                    StrCopyToChar(Gsm_CCID,MemLigneGSM);
                    if(StrnCompare(Gsm_CCID,"ERROR",5))
                        {
                            TASK[TASK1_ID].STEP = 205;
                            GsmState=GSM_ERROR;
                        }
                    else
                        {
                            StrCopyToChar(InfoGSM.CCID,Gsm_CCID);
                            // StrToConsole(InfoGSM.CCID); test ok
                            TASK[TASK1_ID].STEP=7;
                            TimeOutGSM_T1 = 5000;
                        } 
                }
            if (TimeOutGSM_T1 ==0)
                {
                    TASK[TASK1_ID].STEP = 5;
                    GsmState=GSM_ERROR;
                }
            SET_WAIT(TASK1_ID, 500);
            break;
        }
        // On récupère les infos concerant le modem et la sim
        case 7:
        {
            if(CtrlOk.Recu)
                {
                    PositionneAttendOk();
                    MemLigneGSM[0]=0;
                    StrPgmToGsm("AT+CGSN\r");
                    TimeOutGSM_T1 = 5000;
                    TASK[TASK1_ID].STEP = 8;
                }
            if (TimeOutGSM_T1==0)
                {
                    TASK[TASK1_ID].STEP = 5;
                    GsmState=GSM_ERROR;
                }
            SET_WAIT(TASK1_ID, 500);
            break;
        }
        
        case 8:
        {
            if(MemLigneGSM[0])
                {
                    MemLigneGSM[SZ_IMEI]=0;
                    StrCopyToChar(Gsm_IMEI,MemLigneGSM);
                    if(StrnCompare(Gsm_IMEI,"ERROR",5))
                        {
                            TASK[TASK1_ID].STEP = 210;
                            GsmState=GSM_ERROR;
                        }
                    else
                        {
                            StrCopyToChar(InfoGSM.IMEI,Gsm_IMEI);
                            //StrToConsole(InfoGSM.IMEI); test ok
                            TASK[TASK1_ID].STEP=9;
                            TimeOutGSM_T1 = 5000;
                        } 
                }
            if (TimeOutGSM_T1 ==0)
                {
                    TASK[TASK1_ID].STEP = 7;
                    GsmState=GSM_ERROR;
                }
            SET_WAIT(TASK1_ID, 500);
            break;
        }
        
        case 9:
        {
            if(CtrlOk.Recu)
                {
                    PositionneAttendOk();
                    MemLigneGSM[0]=0;
                    StrPgmToGsm("ATI9\r");
                    TimeOutGSM_T1 = 5000;
                    TASK[TASK1_ID].STEP = 10;
                }
            if (TimeOutGSM_T1 ==0)
                {
                    TASK[TASK1_ID].STEP = 7;
                    GsmState=GSM_ERROR;
                }
            SET_WAIT(TASK1_ID, 500);
            break;   
        }
        
        case 10:
        {
            if(MemLigneGSM[0])
                {
                    MemLigneGSM[SZ_ATI9]=0;
                    StrCopyToChar(Gsm_ATI9,MemLigneGSM);
                    if(StrnCompare(Gsm_ATI9,"ERROR",5))
                        {
                            TASK[TASK1_ID].STEP = 212;
                            GsmState=GSM_ERROR;
                        }
                    else
                        {
                            StrCopyToChar(InfoGSM.FirmwareVersion,Gsm_ATI9);
                            //StrToConsole(InfoGSM.FirmwareVersion); test ok
                            TASK[TASK1_ID].STEP=11;
                            TimeOutGSM_T1 = 5000;
                        } 
                 }
            if (TimeOutGSM_T1 ==0)
                {
                    TASK[TASK1_ID].STEP = 9;
                    GsmState=GSM_ERROR;
                }
            SET_WAIT(TASK1_ID, 500);
            break;
        }

        case 11:
        {
             if(CtrlOk.Recu)
            {
                PositionneAttendOk();
                MemLigneGSM[0]=0;
                StrPgmToGsm("AT+CGMM\r");
                TimeOutGSM_T1 = 5000;
                TASK[TASK1_ID].STEP = 12;
            }
            if (TimeOutGSM_T1 ==0)
                {
                    TASK[TASK1_ID].STEP =9;
                    GsmState=GSM_ERROR;
                }
            SET_WAIT(TASK1_ID, 500);
            break;
        }
        
        case 12:
        {
            if(MemLigneGSM[0])
                {
                    MemLigneGSM[SZ_MODELE]=0;
                    StrCopyToChar(Gsm_MODELE,MemLigneGSM);
                    if(StrnCompare(Gsm_MODELE,"ERROR",5))
                        {
                            TASK[TASK1_ID].STEP = 208;
                            GsmState=GSM_ERROR;
                        }
                    else
                    {
                        StrCopyToChar(InfoGSM.Modele,Gsm_MODELE);
                        //StrToConsole(InfoGSM.Modele); test ok
                        TimeOutGSM_T1 = 5000;
                        TASK[TASK1_ID].STEP = 13;
                    }
                } 
            if (TimeOutGSM_T1 ==0)
                {
                    TASK[TASK1_ID].STEP = 11;
                    GsmState=GSM_ERROR;
                }
            SET_WAIT(TASK1_ID, 500);
            break;
        }
        
        // on s'occupe de la sim-----------------------------------------------------------------------------------------
        case 13:
        {
            if(CtrlOk.Recu)
            {
                PositionneAttendOk();
                MemLigneGSM[0]=0;
                StrPgmToGsm("AT+CPIN?\r");
                TimeOutGSM_T1 = 5000;
                TASK[TASK1_ID].STEP = 14;
            }
            if (TimeOutGSM_T1==0)
                {
                    TASK[TASK1_ID].STEP = 11;
                    GsmState=GSM_ERROR;
                }
            SET_WAIT(TASK1_ID, 500);
            break;     
        }
        
        case 14:
        {
            if(MemLigneGSM[0])
                {
                    if(StrnCompare(MemLigneGSM,"+CPIN: READY",12))
                        {
                            TASK[TASK1_ID].STEP=18;  // Code ok
                            TimeOutGSM_T1=5000;
                        }
                    else
                        {
                            if(StrnCompare(MemLigneGSM,"+CPIN: SIM PIN",14))
                                {
                                    TASK[TASK1_ID].STEP=15;  // On doit renvoyer le code
                                    TimeOutGSM_T1=5000;
                                }
                            else
                                {
                                    if(StrnCompare(MemLigneGSM,"ERROR",5))
                                        {
                                            //if(StrnCompare(MemLigneGSM,"+CPIN: NOT INSERTED",19))   // absence de carte sim
                                            TASK[TASK1_ID].STEP=216;
                                            GsmState=GSM_ERROR;
            
                                        }
                                }
                        }
                }
            SET_WAIT(TASK1_ID, 500);
            break;
        }
        
        case 15:
        {
            PositionneAttendOk();
            StrPgmToGsm("AT+CPIN=\"");
            StrToGsm(&InfoGSM.CodePin[0]);
            StrPgmToGsm("\"\r");
            TimeOutGSM_T1 = 5000;
            TASK[TASK1_ID].STEP = 18;
            SET_WAIT(TASK1_ID, 500);
            break;
        }
        
        // On configure le modem------------------------------------------------------------------------------------------------
        case 18:
        {
            if(CtrlOk.Recu)
            {
                PositionneAttendOk();
                StrPgmToGsm("AT+CMEE=0\r");
                TASK[TASK1_ID].STEP = 19;
            }
            if (TimeOutGSM_T1 ==0)
                {
                    TASK[TASK1_ID].STEP = 16;
                    GsmState=GSM_ERROR;
                }
                
            SET_WAIT(TASK1_ID, 500);
            break;
        }
        
        case 19 ://"AT+IFC" Set TE-TA Local Data Flow Control - "AT+IFC=2,2" Hardware Flow control is ON
        {
            PositionneAttendOk();
            TimeOutGSM_T1=1000;	//TimeOut reponse gsm
            StrPgmToGsm("AT+IFC=2,2\r"); //Flow control hardware
            TASK[TASK1_ID].STEP=21;
            SET_WAIT(TASK1_ID,500);
            break;
        }
        
        case 21 ://"AT+CMGF" Select SMS Message Format - "AT+CMGF=1" Text Mode //"AT+CNMI" New SMS Message Indications
        {                       
            if(CtrlOk.Recu)
                {
                    PositionneAttendOk();
                    TimeOutGSM_T1=1000;	//TimeOut reponse gsm
                    StrPgmToGsm("AT+CMGF=1;+CNMI=0,0,0,0,0\r"); //SMS Message Format in text mode
                    TASK[TASK1_ID].STEP=22;
                }
                //Si TimeOut reprend en 18
            if(TimeOutGSM_T1==0)
                {
                    TASK[TASK1_ID].STEP=18;
                    //GestionTimeOutModem(1);  // Non reponse modem, on incremente le compteur de timeout
                }
            break;
        }
  
        case 22 ://"ATS0" Set Number of Rings before Automatically Answering the Call - "ATS0=0" Disable auto answering
        {
            if(CtrlOk.Recu)
                {
                    PositionneAttendOk();
                    TimeOutGSM_T1=1000;	//TimeOut reponse gsm
                    StrPgmToGsm("ATS0=0;S7=255\r"); //Automatic answering is disable ATS0=0
                                      // Tempo max attente decroche appel sortant= 255 sec
                    TASK[TASK1_ID].STEP=24;
                }
            //Si TimeOut reprend en 18
            if(TimeOutGSM_T1==0)
                {
                    TASK[TASK1_ID].STEP=18;
                    //GestionTimeOutModem(1);  // Non reponse modem, on incremente le compteur de timeout
                }
            break;
        }
  
        case 23 ://"AT+CR" Service Reporting Control  //"AT+CLTS" Get Local Timestamp
        {
            if(CtrlOk.Recu)
                {
                    PositionneAttendOk();
                    TimeOutGSM_T1=1000;	//TimeOut reponse gsm
                    
                    //StrPgmToGsm("AT+CR=0\r"); valeur par défaut dc inutile//Service Reporting Control disabled intermediate result code
                    TASK[TASK1_ID].STEP=24; 
                }
            //Si TimeOut reprend en 18
            if(TimeOutGSM_T1==0)
                {
                    TASK[TASK1_ID].STEP=18;
                    //GestionTimeOutModem(1);  // Non reponse modem, on incremente le compteur de timeout
                }
            break;
        }
  
        case 24 ://"AT+CRC" Service Reporting Control
        {
            if(CtrlOk.Recu)
                {
                    PositionneAttendOk();
                    TimeOutGSM_T1=1000;	//TimeOut reponse gsm
                    StrPgmToGsm("AT+CRC=0\r"); //Disable extended format for Incoming Call Indication
                    TASK[TASK1_ID].STEP=25;
                }
            //Si TimeOut reprend en 18
            if(TimeOutGSM_T1==0)
                {
                    TASK[TASK1_ID].STEP=18;
                    //GestionTimeOutModem(1);  // Non reponse modem, on incremente le compteur de timeout
                }
            break;
        }

        case 25 ://"AT+CRSL" Ringer Sound Level
        {
            if(CtrlOk.Recu)
                {
                    PositionneAttendOk();
                    TimeOutGSM_T1=1000;	//TimeOut reponse gsm
                    StrPgmToGsm("AT+CRSL=0\r");
                    TASK[TASK1_ID].STEP=26;
                }
            //Si TimeOut reprend en 18
            if(TimeOutGSM_T1==0)
                {
                    TASK[TASK1_ID].STEP=18;
                    //GestionTimeOutModem(1);  // Non reponse modem, on incremente le compteur de timeout
                }
            break;
        }

        case 26 ://"AT+CFUN" Set Phone Functionality
        {
            if(CtrlOk.Recu)
                {
                    PositionneAttendOk();
                    TimeOutGSM_T1=1000;	//TimeOut reponse gsm
                    StrPgmToGsm("AT+CFUN=1\r"); //Full functionality of phone
                    TASK[TASK1_ID].STEP=27;
                }
            //Si TimeOut reprend en 18
            if(TimeOutGSM_T1==0)
                {
                    TASK[TASK1_ID].STEP=18;
                    //GestionTimeOutModem(1);  // Non reponse modem, on incremente le compteur de timeout
                }
            break;
        }

        case 27 ://"AT+CLIP" Calling Line Identification Presentation  //"AT+CLIR" Calling Line Identification Restriction //"AT+COLP" Connected Line Identification Presentation
        {	 
            if(CtrlOk.Recu)
                {
                    PositionneAttendOk();
                    TimeOutGSM_T1=1000;	//TimeOut reponse gsm
                    StrPgmToGsm("AT+CLIP=0;+CLIR=0;+COLP=0\r");
                    TASK[TASK1_ID].STEP=70;
                }
            //Si TimeOut reprend en 18
            if(TimeOutGSM_T1==0)
                {
                    TASK[TASK1_ID].STEP=18;
                    //GestionTimeOutModem(1);  // Non reponse modem, on incremente le compteur de timeout
                }
            break;
        }
        
        
 case 200:   // Gsm initialise, etape d'orientation des commandes
    {

        if(ChCmdModemTst[0])
            {
                TASK[TASK1_ID].STEP=240;//commande de test
                break;
            }

        if(GsmModeFred.RequestOff==1)//Si demande de force off
            {
                if(GsmState!=GSM_POWER_OFF)
                    {
                        TASK[TASK1_ID].STEP=235;//aller en normal power down
                        break;
                    }
            }
        else//On a le droit de démarrer le gsm
            {
                if((GsmState==GSM_PRET)||(GsmState==GSM_NO_SIGNAL))//Si le status du gsm = PRET
                    {
                        if(ModifAEModem)  // Modif anti echo demandee, on force grafcet en 29
                            {
                                CtrlOk.Recu=1;
                                TASK[TASK1_ID].STEP=29;
                                break;
                            }

                        if(ModifSVRModem)  // Modif type de codage demandee, on force grafcet en 130
                            {
                                TASK[TASK1_ID].STEP=130;
                                break;
                            }

                        //Controle des appels sortants et entrants -----------------------------------------------------------
                        if(AppelEntrant.Status==APPEL_ACTIF)
                            {	
                                AppelEntrant.ReponseAutor=0;//Gestion de la retombée du flag de reponse des appels entrants
                            }

                        // Decroche appel entrant si auto ou demandé
                        if((AppelEntrant.Status==APPEL_ALERT)&&((AppelEntrant.ModeReponse==REPONSE_AUTO)||(AppelEntrant.ReponseAutor==1)))
                            {
                                AppelEntrant.ReponseAutor=0;
                                TASK[TASK1_ID].STEP=72;//On doit décrocher
                                break;
                            }

                        //Traite le controle contrôle de connection et le RSSI toutes les TIME_INTER_RSSI_TEST milisecondes
                        if(TimeRSSI>=TIME_INTER_RSSI_TEST)
                            {
                                TASK[TASK1_ID].STEP=50;          
                                break;
                            }

                        if((TimeChkcall>=TIME_CHECK_CALL_ACTIF)&&((AppelEntrant.Status!=APPEL_AUCUN)||(AppelSortant.Status!=APPEL_AUCUN)))
                            {
                                TASK[TASK1_ID].STEP=70;
                                break;
                            }

                        if(AudioChannel.ATraiter==1)//Si demande de changement de canal audio
                            {
                                TASK[TASK1_ID].STEP=80;
                                break;
                            }


                        //Traite le controle du code PIN apres un reveil par horloge TIME_INTER_CPIN milisecondes
                        if((TesterPIN)&&(TimeChkcpin>TIME_INTER_CPIN)&&(AppelEntrant.Status==APPEL_AUCUN)&&(AppelSortant.Status==APPEL_AUCUN)&&(SMSEntrant.ATraiter==0)&&(SMSSortant.ATraiter==0))
                            {
                                TASK[TASK1_ID].STEP=120;
                                break;
                            }

                        // Demande nom operateur si inconnu  
                        if((NomOperateur[0]=='?')&&(NomOperateur[1]==0)&&(GsmState==GSM_PRET) && (StatusPau.DerGsmRSSI>5))
                        {
                            TASK[TASK1_ID].STEP=122;//Lecture nom operateur
                            break;
                        }
                    }   


                if(GsmState==GSM_PRET)//Si le status du gsm = PRET
                    {
                        //GESTION DES APPELS SORTANTS ---------------------------------------------------------
                        if(AppelEntrant.Status==APPEL_AUCUN)//Si pas d'appel entrant en cours
                            {
                                AppelEntrant.ReponseAutor=0;//Gestion de la retombée du flag de reponse des appels entrants
                                if(AppelSortant.Status==APPEL_TRAITEMENT)
                                    {
                                        TASK[TASK1_ID].STEP=90;
                                        break;
                                    }
                            }

                        //GESTION EMISSION DTMF -----------------------------------------------------------
                        if(DTMFOut.ATraiter==1)//Si demande d'emission DTMF
                            {
                                TASK[TASK1_ID].STEP=110;
                                //En step 110, si l'appel n'est pas actif, l'emission DTMF sera annulée et ressortira une erreur
                                break;
                            }


                        //GESTION DES SMS -----------------------------------------------------------
                        //Efface le sms entrant sur demande
                        if(SMSEntrant.ToutEffacer)
                            {
                                TASK[TASK1_ID].STEP=45;
                                break;
                            }

                        //Efface le sms entrant sur demande
                        if(SMSEntrant.AEffacer)
                            {
                                TASK[TASK1_ID].STEP=43;
                                break;
                            }

                        //Traite les SMS sortants
                        if(SMSSortant.ATraiter==1)
                            {
                                if(SMSSortant.Tempo==0)   // tempo depuis dernier SMS envoyé
                                    {
                                        TASK[TASK1_ID].STEP=60;
                                        TimeSMSList=0;
                                        break;
                                    }
                            }

                        //Traite les sms entrants si pas de sms deja à traiter et mini toutes les TIME_INTER_SMS_LIST milisecondes
                        if(SMSEntrant.ATraiter==0 && SMSEntrant.AEffacer==0 && TimeSMSList>=TIME_INTER_SMS_LIST)
                            {
                                TASK[TASK1_ID].STEP=40;
                                TimeSMSList=0;
                                break;
                            }


                        //SECURITE SUR LES APPELS -----------------------------------------------------------
                        if(AppelEntrant.Status!=APPEL_AUCUN && AppelSortant.Status!=APPEL_AUCUN)
                            {
                                TASK[TASK1_ID].STEP=100;
                                break;
                            }

                        //GESTION DE LA DECONNECTION APPEL VOIX -----------------------------------------------------------
                        if((AppelEntrant.HangUp==1) || (AppelSortant.HangUp==1))
                            {
                                TASK[TASK1_ID].STEP=100;
                                break;
                            }
                    } // if(GSM_State==PRET)//Si le status du gsm = PRET


                if((GsmState!=GSM_PRET)&&(GsmState!=GSM_NO_SIGNAL))  //SSecurite, doit jamais arriver ici
                  {
                      //DebutTraceDebugConsole();
                      //StrPgmToConsole("Reboot modem etape 200, etat incorrect\n");
                      TASK[TASK1_ID].STEP=235;
                  }

          //GESTION DU SLEEP MODE -----------------------------------------------------------
          //SI DEMANDE DE SLEEP MODE
                if(GsmModeFred.RequestSleep==1 && AppelEntrant.Status==APPEL_AUCUN && AppelSortant.Status==APPEL_AUCUN && SMSEntrant.ATraiter==0 && SMSEntrant.AEffacer==0)
                  {
                      _OLD_GSM_State=GsmState;
                      GsmState=GSM_SLEEP;
                      //FixeStatus_GsmState(GsmState);
                      TASK[TASK1_ID].STEP=201;
                      break;
                  }
            } 
        break;
    }
        
        
        

    
        case 98 :
        {
            PositionneAttendOk();
            MemLigneGSM[0]=0;
            StrPgmToGsm("AT+CREG?\r");//Attente de la connexion au reseau
            TimeOutGSM_T1=5000;	//TimeOut reponse gsm
            TimeRSSI=0;
            TASK[TASK1_ID].STEP=32;
            break;
        }
        
        case 99:
        {
            if(CtrlOk.Recu)
                {
                    if(StrnCompare(MemLigneGSM,"+CREG: 0,1",10) || StrnCompare(MemLigneGSM,"+CREG: 0,5",10) )// Inscrit HOME ou ROAMING
                        {
                            //Si connecté au réseau (home ou roaming) on doit controler le RSSI
                            GsmState=CONNECTED;
                            //FixeStatus_GsmState(GsmState); à décommenter quand prêt
                            // Memorise l'heure de l'acces au reseau ou du retour du réseau    
//                            if((DateReseauOK.Heure==HEURE_NON_POSITIONNEE)||DefautInscription)
                                //LectureDateHeure(&DateReseauOK); à décommenter
                            
                            // PAU inscrit sur reseau
                            DefautInscription=0;
                            NbDefautInscription=0;
                            ValReg=MemLigneGSM[9];
                            //if(ValReg=='5')
                                //SetBitMotErreur(MSQEPAU_GSM_MT_ROAMING,0); à décommenter quand prêt
                            //else
                                //ResetBitMotErreur(MSQEPAU_GSM_MT_ROAMING,0); à décommenter quand prêt
                        }
                    else
                        {
                            if(StrnCompare(MemLigneGSM,"+CREG: 0,3",10))                     // Echec enregistrement
                                {
                                    // Registration denied on recommence 
                                    //DebutTraceDebugConsole(); à décommenter
                                    //StrPgmToConsole("Registration denied on recommence\n"); à décommenter
                                    TASK[TASK1_ID].STEP=235;
                                    ValReg=MemLigneGSM[9];
                                    break;
                                }
                            
                            // +CREG: 0,2 Recherche operateur, on ne fait rien
                            if(StrnCompare(MemLigneGSM,"+CREG: 0,0",10) || StrnCompare(MemLigneGSM,"+CREG: 0,4",10) )// 0,0 Pas d'enregistrement, pas de recherche en cours   0,4 etat Inconnu
                                {
                                    // Memorise l'heure de perte du réseau          
                                    NbDefautInscription++;
                                    
                                     // Relance la tache GSM si plus de 10 lectures successives (env 3 reveils)
                                    if(NbDefautInscription>7)
                                        {
                                            NbDefautInscription=0;
                                            if(DefautInscription==0)
                                                {
                                                    DefautInscription=1;
                                                    //LectureDateHeure(&DateErreurReseau); à décommenter
                                                    //DebutTraceDebugConsole();
                                                    //SetBitMotErreur(MSQEPAU_GSM_MT_RESEAU_GSM,1);
                                                }
                                            
                                            // Relance la tache GSM
                                            //StrPgmToConsole("Relance tache modem car defaut inscription\n"); à décommenter
                                            TASK[TASK1_ID].STEP=235;
                                            GsmState=GSM_NO_SIGNAL;
                                            //FixeStatus_GsmState(GsmState);
                                            break;
                                        }
                                    ValReg=MemLigneGSM[9];
                                }      
                            GsmState=GSM_NO_SIGNAL;
                            //FixeStatus_GsmState(GsmState);
                        }
                    //FixeStatus_GsmState(GsmState);
                    TASK[TASK1_ID].STEP=31;
                }
            if(TimeOutGSM_T1==0)
                {
                    TASK[TASK1_ID].STEP=200;
                    //GestionTimeOutModem(1); à décommenter quand prêt  // Non reponse modem, on incremente le compteur de timeout
                }
            break;
        }
        
        case 70:
        {
            if(CtrlOk.Recu)
                {
                    PositionneAttendOk();
                    StrPgmToGsm("AT+UFTP=0,\"54.38.189.80\"\r"); // changer l'IP en dur par une variable: IP
                    TimeOutGSM_T1=5000;
                    TASK[TASK1_ID].STEP = 71;
                }
            if (TimeOutGSM_T1==0)
                {
                    TASK[TASK1_ID].STEP = 18;
                    GsmState=GSM_ERROR;
                }
            SET_WAIT(TASK1_ID, 500);
            break;     
        }
        
        case 71:
        {
             if(CtrlOk.Recu)
                {
                    PositionneAttendOk();
                    StrPgmToGsm("AT+UFTP=2,\"clement\"\r"); // changer clement par une variable: user
                    TimeOutGSM_T1=5000;
                    TASK[TASK1_ID].STEP = 72;
                }
            if (TimeOutGSM_T1 ==0)
                {
                    TASK[TASK1_ID].STEP = 70;
                    GsmState=GSM_ERROR;
                }
            SET_WAIT(TASK1_ID, 500);
            break;
        }
        
        case 72:
        {
            if(CtrlOk.Recu)
                {
                    PositionneAttendOk();
                    StrPgmToGsm("AT+UFTP=3,\"%Clem$\"\r"); // changer le mdp en dur par une variable: mdp
                    TimeOutGSM_T1=5000;
                    TASK[TASK1_ID].STEP = 73;
                }
            if (TimeOutGSM_T1 ==0)
                {
                    TASK[TASK1_ID].STEP = 71;
                    GsmState=GSM_ERROR;
                }
            SET_WAIT(TASK1_ID, 500);
        }
        
        case 73:
        {
            if(CtrlOk.Recu)
                {
                    PositionneAttendOk();
                    StrPgmToGsm("AT+UFTP=6,1\r"); // changer 1 par une variable: connexion active/passive 
                    TimeOutGSM_T1=5000;
                    TASK[TASK1_ID].STEP = 74;
                }
            if (TimeOutGSM_T1 ==0)
                {
                    TASK[TASK1_ID].STEP = 72;
                    GsmState=GSM_ERROR;
                }
            SET_WAIT(TASK1_ID, 500);
        }
        
        case 74:
        {
            if(CtrlOk.Recu)
                {
                    PositionneAttendOk();
                    StrPgmToGsm("AT+UFTP=7,21\r"); // changer 21 par une variable: port
                    TimeOutGSM_T1=5000;
                    TASK[TASK1_ID].STEP = 75;
                }
            if (TimeOutGSM_T1 ==0)
                {
                    TASK[TASK1_ID].STEP = 73;
                    GsmState=GSM_ERROR;
                }
            SET_WAIT(TASK1_ID, 500);
        }
        
        case 75:
        {
            if(CtrlOk.Recu)
                {
                    PositionneAttendOk();
                    StrPgmToGsm("AT+UPSD=0,1,\"mmsbouygtel.com.mnc020.mcc208.gprs\"\r"); // changer le nom par une variable
                    TimeOutGSM_T1=5000;
                    TASK[TASK1_ID].STEP = 76;
                }
            if (TimeOutGSM_T1 ==0)
                {
                    TASK[TASK1_ID].STEP = 74;
                    GsmState=GSM_ERROR;
                }
            SET_WAIT(TASK1_ID, 500);
        }
        
        case 76:
        {
            if(CtrlOk.Recu)
                {
                    PositionneAttendOk();
                    StrPgmToGsm("AT+UPSDA=0,3\r"); // changer le nom par une variable
                    TimeOutGSM_T1=5000;
                    TASK[TASK1_ID].STEP = 77;
                }
            if (TimeOutGSM_T1 ==0)
                {
                    TASK[TASK1_ID].STEP = 75;
                    GsmState=GSM_ERROR;
                }
            SET_WAIT(TASK1_ID, 500);
        }
        
        case 77:
        {
            if(CtrlOk.Recu)
                {
                    PositionneAttendOk();
                    MemLigneGSM[0]=0;
                    StrPgmToGsm("AT+UFTPC=1\r"); // changer le nom par une variable
                    TimeOutGSM_T1=5000;
                    TASK[TASK1_ID].STEP = 78;
                }
            if (TimeOutGSM_T1 ==0)
                {
                    TASK[TASK1_ID].STEP = 76;
                    GsmState=GSM_ERROR;
                }
            SET_WAIT(TASK1_ID, 500);
        }
        
        case 78:
        {
            if(MemLigneGSM[0])
                {
                    if(StrnCompare(MemLigneGSM,"+UUFTPCR: 1,1",13)) //Connexion au serveur FTP réussie
                        {
                            TASK[TASK1_ID].STEP = 79;
                            TimeOutGSM_T1=5000;
                        }
                    else
                        {
                            if(StrnCompare(MemLigneGSM,"+UUFTPCR: 1,0",13)) //Connexion au serveur FTP échouée
                                {
                                    TASK[TASK1_ID].STEP = 77;
                                    GsmState=GSM_ERROR;
                                }
                        }
                }
            if (TimeOutGSM_T1 ==0)
                {
                    TASK[TASK1_ID].STEP = 229;
                    GsmState=GSM_ERROR;
                }
            SET_WAIT(TASK1_ID, 1000);
        }
        
        case 79:// case de test à retirer
        {
            if(CtrlOk.Recu)
                {
                    PositionneAttendOk();
                    StrPgmToGsm("AT+UFTPC=10,\"Work!\"\r"); // changer le nom par une variable
                    TimeOutGSM_T1=5000;
                    TASK[TASK1_ID].STEP = 81;
                }
            if (TimeOutGSM_T1 ==0)
                {
                    TASK[TASK1_ID].STEP = 79;
                    GsmMode=GSM_ERROR;
                }
            SET_WAIT(TASK1_ID, 500);
        }
        
        
    }
}

    // <editor-fold defaultstate="collapsed" desc="Fonctions">

    /*******************************************************************************
     *                               FONCTIONS                                      *
     *******************************************************************************/
    void Init_Uart1(void) 
        {
            // STSEL 1; IREN disabled; PDSEL 8N; UARTEN enabled; RTSMD disabled; USIDL disabled; WAKE disabled; ABAUD disabled; LPBACK disabled; BRGH enabled; URXINV disabled; UEN TX_RX; Data Bits = 8; Parity = None; Stop Bits = 1;
            U1MODE = (0x8008 & ~(1 << 15)); // disabling UARTEN bit
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
            U1STAbits.URXEN = 1;
            for (c1_1 = 0; c1_1 < GSM_RX_SIZE; c1_1++) {
                GsmRx[c1_1] = 0;
            }
            IndBuffRx = 0;
            DataUartDispo = 0;
            IndLectRx = 0;
            DataRxOverflow = 0;
            TimeoutUart = 0;
            //CTS_PULLUP_ON;    //désactivé car le translateur ne fournit pas assez de courant !
        }

    void ReceiveUart(void) //fonction appelée en interruption qui récupère la data dans le registre de réception pour la mettre dans un buffer de réception
        {
            //    if(RC1STAbits.OERR == 1)    //cas erreur de réception
            //    { 
            //        RC1STAbits.SPEN = 0;    //restart
            //        RC1STAbits.SPEN = 1;
            //    }
            //BufferRxUartGsm[IndBuffRx] = U1RXREG;
            CurRxGSM_IT[IdxCurRxGSM_IT]=U1RXREG;
            IdxCurRxGSM_IT++;
            //U2TXREG=CurRxGSM_IT[IdxCurRxGSM_IT];            // Reponse modem vers console
            //IndBuffRx++;
            //if (sizeof (BufferRxUartGsm) <= IndBuffRx) //buffer rotatif
            //{
            //    IndBuffRx = 0;
            //}
            //DataUartDispo++; //incrémente le nombre de data à récupérer
        }

    void GsmToUc(volatile unsigned char* DataRx) 
        {
            while (DataUartDispo >= 1) 
                {
                    if (FlagTrameUart == 0) 
                        {
                            if (TimeoutUart > MAX_INTER_DATA_MS) // au delà d'un temps entre 2 octets, on repart au 1er octet, il s'agit d'une sécurité au cas où on n'ait pas reçu le RC attendu d'une fin de trame
                                {
                                    PtUart = 0;
                                }
                            DataRx[PtUart] = ReadUart();
                            TimeoutUart = 0;
                            if ((PtUart > 0)&&(DataRx[PtUart] == 0x0A)&&(DataRx[PtUart - 1] == 0x0D)) 
                                {
                                    FlagTrameUart = 1; // à la réception de l'octet RC on signale la réception d'une trame à analyser
                                    LgRxTmp = PtUart - 1;
                                    PtUart = 0;
                                    break; //on sort de la fonction, s'il y a un reste dans le buffer de réception il sera récupérer lors du prochain appel par la tache COM.
                                }
                            PtUart++;
                            if (PtUart > GSM_RX_SIZE) //tableau trop petit
                                {
                                    PtUart--; // en cas de débordement on écrase la dernière case (la réception du prochain RC lèvera le flag de trame à analyser)
                                    DataRxOverflow = 1; // pour signaler que la trame reçue est tronquée
                                }
                        } 
                    else // dans le cas où la trame précédente n'a pas été traitée, on sort
                    {
                        TimeoutUart = 0;
                        break;
                    }
                }
        }

    unsigned char ReadUart(void) //fonction qui, à chaque fois appelée, retourne la valeur du buffer de réception et pointe sur la case suivante
        {
            unsigned char readValue = 0;

            //    IEC0bits.U1RXIE = 0;
            if (DataUartDispo) 
                {
                    readValue = BufferRxUartGsm[IndLectRx];
                    IndLectRx++;
                    if (sizeof (BufferRxUartGsm) <= IndLectRx) //buffer rotatif
                        {
                            IndLectRx = 0;
                        }
                    DataUartDispo--; //décrémente le nombre de data à récupérer
                }
            //    IEC0bits.U1RXIE = 1;
            return readValue;
        }

    void StrPgmToGsm(const char *data) //Send a string in flash to GSM module
        {
            unsigned char x = 0;
            do {
                while (U1STAbits.TRMT == 0); //Transmit Shift Register is not empty, a transmission is in progress or queued --> wait
                while (UART_GSM_NOT_READY);
                U1TXREG = *data; // Transmit a byte
                Tx[x] = *data;
                x++;
            } while (*++data);
            while (U1STAbits.TRMT == 0);
        }

    void StrToGsm(unsigned char *data) //Send a string in ram (buff) to GSM module
        {
            unsigned char x = 30;
            do {
                while (U1STAbits.TRMT == 0); //Transmit Shift Register is not empty, a transmission is in progress or queued --> wait
                while (UART_GSM_NOT_READY);
                U1TXREG = *data; // Transmit a byte
                Tx[x] = *data;
                x++;
            } while (*++data);
            while (U1STAbits.TRMT == 0);
        }

    void InitVarGsm(void) 
        {
            InfoGSM.CodePin[0] = CODE_PIN_D1;
            InfoGSM.CodePin[1] = CODE_PIN_D2;
            InfoGSM.CodePin[2] = CODE_PIN_D3;
            InfoGSM.CodePin[3] = CODE_PIN_D4;
            InfoGSM.CodePin[4] = 0;
            InfoGSM.Rssi = 99;
            GsmCodeErrRecu = 0;
        }

    void CpTab(unsigned char* tabToCopy, unsigned char* tab, unsigned char lght) 
        {
            unsigned char ind;
            for (ind = 0; ind < lght; ind++) 
                {
                    tab[ind] = tabToCopy[ind];
                }
        }
    
    void Disable_Uart1(void) 
        {
            U1MODEbits.UARTEN = 0;
            U1STAbits.UTXEN = 0;
        }
    
    void PositionneAttendOk(void)
        {
            CtrlOk.Idx=0;
            CtrlOk.Recu=0;
            CtrlOk.Attendu=1;
        }

    
// </editor-fold>