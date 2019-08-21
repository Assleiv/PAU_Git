#if 0
/************************************************************************************************
* cmd_traitement.c                                                                              *
************************************************************************************************/

#include"xc.h"  
#include"Recherche.h"
#include"Copy_Compare.h"
#include"Conversion.h"
#include"cmd_traitement.h"
#include"pau_glo.h"
#include"pau_cst.h"
#include"pau_typ.h"
#include"Plan_Memoire.h"
#include"Noyau.h"
#include"ApiGsm.h"
#include"pau_pro.h"
#include"1_Gsm.h"
#include"6_IcsI2c.h"
#include"8_Com232.h"
#include<stdio.h>
#include <string.h>  // definition de NULL
#include <stdlib.h>


// Numero de la borne distante cible
unsigned char NumSMSCible[SZ_NUM_TEL+1];
// Detection de la presence d'un PC sur port serie 
unsigned char CptPresPC;
// Si 1, cette commande ne renvoi rien, on n'emet pas fail ni rien d'autre
unsigned char Silence;

unsigned char T9_Gfe[NB_E_TOR];
// Gestion des reponses fail successive, sert a stopper ping/pong avec systeme emettant aussi des sms
struct STR_CTRL_FAIL_SMS RepFailSMS;

unsigned char NomPau[SZ_NOM_PAU+1];


/*
 * void DebutTraceDebugConsole(void)
 * Envoi sur la console la chaine de début de debug
 * ENTREE Aucune
 * SORTIE Aucune
 * RETOUR Aucun
 */
void DebutTraceDebugConsole(void)
{
    StrToConsole(DEB_TRACE_DEBUG_CONS);
}


/*
 * unsigned char PositionneBitOptionsSoft(unsigned char Masque,unsigned char Valeur)
 * Modification d'un ou plusieurs bits de la variable globale OptionsSoft
 * ENTREE Masque : Masque du ou des bits a modifier
 *        Valeur : Si 0 les bits sont mis à 0, sinon à 1
 * SORTIE Aucune
 * RETOUR <>0 si modification
 */
unsigned char PositionneBitOptionsSoft(unsigned char Masque,unsigned char Valeur)
{
unsigned char MemOptionsSoft;

MemOptionsSoft=OptionsSoft;
if(Valeur)
  {
  if((OptionsSoft&Masque)!=Masque )
    {
    OptionsSoft|=Masque;
    }
  }
else
  {
  if((OptionsSoft&Masque)==Masque)
    {
    OptionsSoft&=~Masque;
    }
  }

if(MemOptionsSoft!=OptionsSoft)
  {
    WriteI2c(EEPROM, ADR_OPTIONS,1,&OptionsSoft,&CodeErreurI2c,&TimeOutI2c);
    ReadI2c(EEPROM, ADR_OPTIONS,1,&OptionsSoft,&CodeErreurI2c,&TimeOutI2c);
  }
return(MemOptionsSoft^OptionsSoft);
}



/*
void RepondCFA(unsigned char Ligne)
Appellé pour répondre à une commande CFA
renvoie le paramétrage d'une ligne CFA
ENTREE : Ligne = Numero de la ligne
SORTIE : Variable globale Reponse
RETOUR : Aucun
*/
void RepondCFA(unsigned char Ligne)
{
unsigned char Lg;
unsigned char ii;
unsigned char Tmp[SZ_MAX_CH_CFA+1];
unsigned char Ch2[2];
unsigned int EEaddrBaseLigne;        // Adresse de base en EEPROM de la ligne CFA

if(Ligne<NB_LIGNES_TAB_CFA)
  {
  // EEaddrBaseLigne = ADRESSE_BASE_0_CFA + (Ligne *OFFSET_INTER_LIGNE_CFA) 
  // Detailler sinon erreur cause char / int
  EEaddrBaseLigne=Ligne;
  EEaddrBaseLigne=EEaddrBaseLigne*OFFSET_INTER_LIGNE_CFA;
  EEaddrBaseLigne=EEaddrBaseLigne+ADRESSE_BASE_0_CFA;

  Lg=StrCopyPgmToChar(Reponse,DEB_REP_LOCALE"CFA L=");
  Lg+=uchartoa(Ligne,&Reponse[Lg],0);
  
  // Lit le car ECDM dans Ch2[0]
  ReadI2c(EEPROM, EEaddrBaseLigne+OFF_CAR_ECDM,1,Ch2,&CodeErreurI2c,&TimeOutI2c);
  Lg+=StrCopyPgmToChar(&Reponse[Lg]," E=");
  for(ii=0;ii<NB_E_TOR;ii++)
    {
    if(Ch2[0] & MSQ_CAR_ECDM_E(ii))
      Tmp[ii]='1';
    else
      Tmp[ii]='0';
    }  
  Tmp[ii]=0;
  Lg+=StrCopyToChar(&Reponse[Lg],Tmp);

  Lg+=StrCopyPgmToChar(&Reponse[Lg]," D=");
  if(Ch2[0] & MSQ_CAR_ECDM_DTMF)
    Tmp[0]='1';
  else
    Tmp[0]='0';
  Tmp[1]=0;
  Lg+=StrCopyToChar(&Reponse[Lg],Tmp);

  Lg+=StrCopyPgmToChar(&Reponse[Lg]," C=");
  if(Ch2[0] & MSQ_CAR_ECDM_CYCLIQUE)
    Tmp[0]='1';
  else
    Tmp[0]='0';
  Tmp[1]=0;
  Lg+=StrCopyToChar(&Reponse[Lg],Tmp);

  // Parametre X
  Lg+=StrCopyPgmToChar(&Reponse[Lg]," X=");
  if(Ch2[0] & MSQ_CAR_ECDM_NUM_EXT)
    Tmp[0]='1';
  else
    Tmp[0]='0';
  Tmp[1]=0;
  Lg+=StrCopyToChar(&Reponse[Lg],Tmp);

  // Parametre V
  Lg+=StrCopyPgmToChar(&Reponse[Lg]," V=");
  memset(Tmp,0,SZ_MAX_CH_CFA);
  ReadI2c(EEPROM, EEaddrBaseLigne+OFF_NUM_TEL,SZ_NUM_TEL,Tmp,&CodeErreurI2c,&TimeOutI2c);
  Tmp[SZ_NUM_TEL]=0;
  if(Tmp[0]==0) StrCopyPgmToChar(Tmp,"Z");
  Lg+=StrCopyToChar(&Reponse[Lg],Tmp);

  // Parametre S
  Lg+=StrCopyPgmToChar(&Reponse[Lg]," S=");
  memset(Tmp,0,SZ_MAX_CH_CFA);
  ReadI2c(EEPROM, EEaddrBaseLigne+OFF_NUM_SMS,SZ_NUM_TEL,Tmp,&CodeErreurI2c,&TimeOutI2c);
  Tmp[SZ_NUM_TEL]=0;
  if(Tmp[0]==0) StrCopyPgmToChar(Tmp,"Z");
  Lg+=StrCopyToChar(&Reponse[Lg],Tmp);

  // Parametre I
  Lg+=StrCopyPgmToChar(&Reponse[Lg]," I=");
  memset(Ch2,0,2);
  ReadI2c(EEPROM, EEaddrBaseLigne+OFF_TYPE_SMS,1,Ch2,&CodeErreurI2c,&TimeOutI2c);
  uchartoa(Ch2[0],Tmp,0);
  Lg+=StrCopyToChar(&Reponse[Lg],Tmp);

  // Parametre M
  Lg+=StrCopyPgmToChar(&Reponse[Lg]," M=");
  memset(Tmp,0,SZ_MAX_CH_CFA);
  ReadI2c(EEPROM, EEaddrBaseLigne+OFF_TXT_SMS,SZ_TXT_SMS_APPEL,Tmp,&CodeErreurI2c,&TimeOutI2c);
  Tmp[SZ_TXT_SMS_APPEL]=0;
  if(Tmp[0]==0) StrCopyPgmToChar(Tmp,"Z");
  Lg+=StrCopyToChar(&Reponse[Lg],Tmp);

  // Parametre T
  Lg+=StrCopyPgmToChar(&Reponse[Lg]," T=");
  memset(Ch2,0,2);
  ReadI2c(EEPROM, EEaddrBaseLigne+OFF_TPA,1,Ch2,&CodeErreurI2c,&TimeOutI2c);
  uchartoa(Ch2[0],Tmp,0);
  Lg+=StrCopyToChar(&Reponse[Lg],Tmp);
  }
}


/*
void FabriqueReponseConfSMS(unsigned char *Rs)
Appellé pour répondre à une commande SMS ou sur echec de redirection
renvoie le paramétrage SMS sur le port serie
ENTREE : Rs  = Buffer de travail (! doit faire au moins SZ_CMD_SMS+1 caracteres
SORTIE : Rs
RETOUR : Aucun
SMS NUM=num tel pau distant ou Z pour effacer
*/
void FabriqueReponseConfSMS(unsigned char *Rs)
{
unsigned char Lg;


Lg=StrCopyPgmToChar(Rs,DEB_REP_LOCALE"SMS NUM=");
if(NumSMSCible[0])
  Lg+=StrCopyToChar(&Rs[Lg],NumSMSCible);
else
  Lg+=StrCopyPgmToChar(&Rs[Lg],"Z");

// Envoi SMS possible si precedent traité et effacé et GSM pret
Lg+=StrCopyPgmToChar(&Rs[Lg]," EP=");
if((SMSSortant.ATraiter==0)&&(StatusPau.GsmState==GSM_PRET))
  Lg+=StrCopyPgmToChar(&Rs[Lg],"1");
else
  Lg+=StrCopyPgmToChar(&Rs[Lg],"0");

Lg+=StrCopyPgmToChar(&Rs[Lg]," LNG=");
Lg+=uchartoa(SZ_DATA_SMS,&Reponse[Lg],0);

Lg+=StrCopyPgmToChar(&Rs[Lg]," CPE=");
Lg+=ulongtoa(SMSEntrant.Cpt,&Reponse[Lg],12);

Lg+=StrCopyPgmToChar(&Rs[Lg]," CPS=");
Lg+=ulongtoa(SMSSortant.Cpt,&Reponse[Lg],12);

/*
Lg+=StrCopyPgmToChar(&Rs[Lg]," FRED=");
Lg+=uchartoa(CligInfoTestCligBP,&Reponse[Lg],0);

Lg+=StrCopyPgmToChar(&Rs[Lg]," IGNB=");
Lg+=uchartoa(CptSecIgnoreAppel,&Reponse[Lg],0);
*/


}


/*
void EmetSMSPortSerie(const rom char *Entete,unsigned char *Texte,unsigned char *numSMS,unsigned char *DateSMS,unsigned char *HeureSMS)

Emet sur le port serie le contenu d'un SMS recu 

*/
void EmetSMSPortSerie(const char *Entete,unsigned char *Texte,unsigned char *numSMS,unsigned char *DateSMS,unsigned char *HeureSMS)
{
StrToConsole("\n");
StrToConsole(Entete);
StrToConsole(" NUM=");
StrToConsole(numSMS);

// Date SMS positionne sur SMS entrant uniquement
if(DateSMS!=NULL)
  {
  StrToConsole(" DTE=");
  StrToConsole(DateSMS);
  }
else
  {
  StrToConsole(" DTE=0");
  }
// Heure SMS positionne sur SMS entrant uniquement
if(HeureSMS!=NULL)
  {
  StrToConsole(" HRE=");
  StrToConsole(HeureSMS);
  }
else
  {
  StrToConsole(" HRE=0");
  }
StrToConsole(" TSMS=");
StrToConsole(Texte);
StrToConsole("\n");
}



void Traite_MAPPING(unsigned char *cmd,unsigned char *numSMS)
{
unsigned char Lg;
unsigned char iTask;
unsigned char Tmp[5];

if(ValeurVariableLCR(cmd,"BUG=",Tmp,3))
  {
  if(Tmp[0]=='1')
    TASK[4].STATE=5;
  else
    TASK[4].STATE=MSQ_READY;
  }

for (iTask=0;iTask<NB_TASK;iTask++)
  {
  Lg=StrCopyPgmToChar(Reponse,DEB_REP_LOCALE"MAPPING");
  Lg+=StrCopyPgmToChar(&Reponse[Lg]," G");
  Lg+=uchartoa(iTask,&Reponse[Lg],2);
  Lg+=StrCopyPgmToChar(&Reponse[Lg],"=");
  Lg+=uchartoa(TASK[iTask].STEP,&Reponse[Lg],3);

  Lg+=StrCopyPgmToChar(&Reponse[Lg]," S");
  Lg+=uchartoa(iTask,&Reponse[Lg],2);
  Lg+=StrCopyPgmToChar(&Reponse[Lg],"=");
  Lg+=uchartoa(TASK[iTask].STATE,&Reponse[Lg],0);

  if(TASK[iTask].STATE==MSQ_WAIT)
    {
    Lg+=StrCopyPgmToChar(&Reponse[Lg]," D");
    Lg+=uchartoa(iTask,&Reponse[Lg],0);
    Lg+=StrCopyPgmToChar(&Reponse[Lg],"=");
    Lg+=uinttoa(TASK[iTask].DELAY,&Reponse[Lg]);
    }
  Lg+=StrCopyPgmToChar(&Reponse[Lg],"\n");
  StrToConsole(Reponse);
  }
#if 0
Lg=StrCopyPgmToChar(Reponse,DEB_REP_LOCALE"MAPPING");
Lg+=StrCopyPgmToChar(&Reponse[Lg]," STKPTR=");
Lg+=uchartoa(STKPTR,&Reponse[Lg],2);             // Ne fonctionne pas car adapté au PIC précedent
Lg+=StrCopyPgmToChar(&Reponse[Lg],"\n");
#endif
}

 // Mit en commentaire car sert au debug 
void Traite_MODEM(unsigned char *cmd,unsigned char *numSMS)
{
unsigned char Tmp[10];
unsigned char Lg;
unsigned char Idx;
int ii;

if(FindFirstSTRInBuff (cmd,"CMD=", &Idx))
  {
  // Rajoute la longueur de la variable a l'index retourne
  Idx+=4;

  // Recopie la valeur de la variable
  ii=0;
  while((ii<SZ_TRAME_MODEM_TST)&&(cmd[Idx]!=LF)&&(cmd[Idx]!=CR)&&(cmd[Idx]))     // &&(cmd[Idx]!=' ')    // supprimé pour passer commande avec espace
    {
    ChCmdModemTst[ii]=cmd[Idx];
    Idx++;
    ii++;
    }
  ChCmdModemTst[ii]=0;
  if(ii)
    {
    DebutTraceDebugConsole();
    StrToConsole("Cmd modem :");
    StrToConsole(ChCmdModemTst);
    StrToConsole("\n");
    } 
  }

if(ValeurVariableLCR(cmd,"RAZ=",Tmp,3))
  {
  if(Tmp[0]=='1')
    {
    DebutTraceDebugConsole();
    StrToConsole("Reboot du modem\n");
    TASK[TASK1_ID].STEP=235;
    }
#ifdef TEST_ARRET_MODEM_DEF
  if((Tmp[0]=='2')&&(numSMS==NULL))
    {
    DebutTraceDebugConsole();
    StrPgmToConsole("ARRET def du modem\n");
    ArretModemDefinitif=1;
    TASK[TASK12_ID].STEP=235;
    }
#endif
  }

// test modif anti echo du modem
if(ValeurVariableLCR(cmd,"ECHO=",Tmp,3))
  {
  Lg=strtouc(Tmp);
  WriteI2c(EEPROM, ADR_ECHO_MODEM,1,&Lg,&CodeErreurI2c,&TimeOutI2c);
  FabriqueChaineValeurEcho(Tmp);
  ModifAEModem=1;
  }

// test modif anti echo du modem
if(ValeurVariableLCR(cmd,"SVR=",Tmp,3))
  {
  Lg=strtouc(Tmp);
  if(Lg<=MAX_SVR_MODEM)
    {
    WriteI2c(EEPROM, ADR_SVR_MODEM,1,&Lg,&CodeErreurI2c,&TimeOutI2c);
    ModifSVRModem=1;
    }  
  }


if(ValeurVariableLCR(cmd,"DCD=",Tmp,3))
  {
  CurDurCarDTMF=strtouc(Tmp);
  if((CurDurCarDTMF<DCD_MIN)||(CurDurCarDTMF>DCD_MAX))
    CurDurCarDTMF=DCD_DEFAUT;

  WriteI2c(EEPROM, ADR_DC_DTMF,1,&CurDurCarDTMF,&CodeErreurI2c,&TimeOutI2c);
  }

if(ValeurVariableLCR(cmd,"DIT=",Tmp,3))
  {
  CurDurInterTrainDTMF_20ms=strtouc(Tmp);
  if((CurDurInterTrainDTMF_20ms<DIT_MIN)||(CurDurInterTrainDTMF_20ms>DIT_MAX))
    CurDurInterTrainDTMF_20ms=DIT_DEFAUT;

  WriteI2c(EEPROM, ADR_INT_TRAIN,1,&CurDurInterTrainDTMF_20ms,&CodeErreurI2c,&TimeOutI2c);
  }

// Recupere duree d'un car DTMF
ReadI2c(EEPROM,ADR_DC_DTMF,1,&CurDurCarDTMF,&CodeErreurI2c,&TimeOutI2c);
// Recupere duree inter train DTMF
ReadI2c(EEPROM,ADR_INT_TRAIN,1,&CurDurInterTrainDTMF_20ms,&CodeErreurI2c,&TimeOutI2c);

if(ValeurVariableLCR(cmd,"DTMF=",Tmp,9))
  {
  SendDTMF(Tmp, StrLength(Tmp),CurDurCarDTMF);
  }

Reponse[0]=0;
if(ValeurVariableLCR(cmd,"INFO=",Tmp,3))
  {
  if(Tmp[0]=='1')
    {
    Lg=StrCopyPgmToChar(Reponse,DEB_REP_LOCALE"MODEM RequOff=");
    if(GsmModeFred.RequestOff)
      Lg+=StrCopyPgmToChar(&Reponse[Lg],"1");
    else
      Lg+=StrCopyPgmToChar(&Reponse[Lg],"0");
    Lg+=StrCopyPgmToChar(&Reponse[Lg]," RequSleep=");
    if(GsmModeFred.RequestSleep)
      Lg+=StrCopyPgmToChar(&Reponse[Lg],"1");
    else
      Lg+=StrCopyPgmToChar(&Reponse[Lg],"0");

    Lg+=StrCopyPgmToChar(&Reponse[Lg]," GSM_Ste=");
    switch(GSM_State)
      {
      case GSM_ST_POWER_OFF: //module eteint (pas d'alimentation)
        Lg+=StrCopyPgmToChar(&Reponse[Lg],"PWR_OFF");
        break;
      case GSM_ST_POWER_ON:	//phase d'alimentation du module
        Lg+=StrCopyPgmToChar(&Reponse[Lg],"PW_ON");
        break;
      case GSM_ST_CONFIG:		//phase d'init
        Lg+=StrCopyPgmToChar(&Reponse[Lg],"CONFIG");
        break;
      case GSM_ST_PRET:		//Connecté au réseau
        Lg+=StrCopyPgmToChar(&Reponse[Lg],"PRET");
        break;
      case GSM_ST_NO_SIGNAL:	//Deconnecté
        Lg+=StrCopyPgmToChar(&Reponse[Lg],"NO_SIGNAL");
        break;
      case GSM_ST_SLEEP:		//Mode veille actif
        Lg+=StrCopyPgmToChar(&Reponse[Lg],"SLEEP");
        break;
      case GSM_ST_ERROR:		//ERREUR - (pas d'alimentation)
        Lg+=StrCopyPgmToChar(&Reponse[Lg],"ERROR");
        break;
      default:
        Lg+=StrCopyPgmToChar(&Reponse[Lg],"???");
        break;
      }

    Lg+=StrCopyPgmToChar(&Reponse[Lg]," T( pin=");
    Lg+=uinttoa(TimeChkcpin,&Reponse[Lg]);

    Lg+=StrCopyPgmToChar(&Reponse[Lg],"  rssi=");
    Lg+=uinttoa(TimeRSSI,&Reponse[Lg]);
 
    Lg+=StrCopyPgmToChar(&Reponse[Lg],"  call=");
    Lg+=uinttoa(TimeChkcall,&Reponse[Lg]);

    Lg+=StrCopyPgmToChar(&Reponse[Lg],"  sms=");
    Lg+=uinttoa(TimeSMSList,&Reponse[Lg]);
    Lg+=StrCopyPgmToChar(&Reponse[Lg]," ) ");

    if(CmdAlimGsm)
      Lg+=StrCopyPgmToChar(&Reponse[Lg],"  CmdA=1");
    else
      Lg+=StrCopyPgmToChar(&Reponse[Lg],"  CmdA=0");

    if(PwrKeyGsm)
      Lg+=StrCopyPgmToChar(&Reponse[Lg],"  pwk=1");
    else
      Lg+=StrCopyPgmToChar(&Reponse[Lg],"  pwk=0");

    if(In_StatusGSM)
      Lg+=StrCopyPgmToChar(&Reponse[Lg],"  In_S=1");
    else
      Lg+=StrCopyPgmToChar(&Reponse[Lg],"  In_S=0");
    
    Lg+=StrCopyPgmToChar(&Reponse[Lg],"  Nbto=");
    Lg+=uchartoa(NbToModemSucc,&Reponse[Lg],0);
    }
  }

if(ValeurVariableLCR(cmd,"EFFACERSMS=",Tmp,3))
  {
  if(Tmp[0]=='1')
    {
    // Demande de tout effacer si erreur
    SMSEntrant.ToutEffacer=1;
    }
  }

if(ValeurVariableLCR(cmd,"TRACE=",Tmp,3))
  {
  if(Tmp[0]=='1')
    {
    PositionneBitOptionsSoft(MSQ_OPTIONS_TRACE_MODEM,1);
    // Raz des index de memorisation
    IdxCurCmdGSM=0;
    IdxCurRxGSM=0;
    }
  else
    {
    PositionneBitOptionsSoft(MSQ_OPTIONS_TRACE_MODEM,0);
    }
  }

if(Reponse[0]==0)
  {
  Lg=StrCopyPgmToChar(Reponse,DEB_REP_LOCALE"MODEM TRACE=");
  if((OptionsSoft&MSQ_OPTIONS_TRACE_MODEM)==MSQ_OPTIONS_TRACE_MODEM)
    Lg+=StrCopyPgmToChar(&Reponse[Lg],"1");   
  else
    Lg+=StrCopyPgmToChar(&Reponse[Lg],"0");

  Lg+=StrCopyPgmToChar(&Reponse[Lg]," DCD=");   
  Lg+=uchartoa(CurDurCarDTMF,&Reponse[Lg],1);

  Lg+=StrCopyPgmToChar(&Reponse[Lg]," DIT=");   
  Lg+=uchartoa(CurDurInterTrainDTMF_20ms,&Reponse[Lg],1);

  Lg+=StrCopyPgmToChar(&Reponse[Lg]," SVR=");  
  ReadI2c(EEPROM,ADR_SVR_MODEM,1,Tmp,&CodeErreurI2c,&TimeOutI2c);
  Lg+=uchartoa(Tmp[0],&Reponse[Lg],1);

  Lg+=StrCopyPgmToChar(&Reponse[Lg]," ECHO=");   
  Lg+=uchartoa(FabriqueChaineValeurEcho(&Reponse[Lg]),&Reponse[Lg],1);
  
  Lg+=StrCopyPgmToChar(&Reponse[Lg]," (");   
  FabriqueChaineValeurEcho(&Reponse[Lg]);
  }
}





void Traite_PC(unsigned char *cmd,unsigned char *numSMS)
{
unsigned char Lg;

Lg=StrCopyPgmToChar(Reponse,DEB_REP_LOCALE"PC PRE=");
Lg+=uchartoa(CptPresPC,&Reponse[Lg],1);
}


void Traite_OPERATEUR(unsigned char *cmd,unsigned char *numSMS)
{
unsigned char Lg;

Lg=StrCopyPgmToChar(Reponse,DEB_REP_LOCALE"OPERATEUR REG=");
Reponse[Lg]=ValReg;
Lg+=1;
Lg+=StrCopyPgmToChar(&Reponse[Lg]," NOM=\"");
Lg+=StrCopyToChar(&Reponse[Lg],NomOperateur);
Lg+=StrCopyPgmToChar(&Reponse[Lg],"\"");
}



/**********************************************************************************************************
***                          Fonctions appellees en réponse à une commande LCR                          ***
***********************************************************************************************************/

void Traite_APPEL(unsigned char *cmd,unsigned char *numSMS)
{
unsigned char NumTel[SZ_NUM_TEL+1];
unsigned char DureeAch;
unsigned char Urgent;
unsigned char Lg;
unsigned char Court;

if(FindFirstSTRInBuff (cmd,"COURT=1",  &Lg))
  Court=1;          
else
  Court=0;

if(ValeurVariableLCR(cmd,"TPA=",NumTel,3))
  {
  DureeAch=strtouc(NumTel);
  }
else
  {
  DureeAch=TPA_DEFAUT;
  }
if(ValeurVariableLCR(cmd,"URG=",NumTel,1))
  {
  Urgent=strtouc(NumTel);
  }
else
  {
  Urgent=0;
  }

if(ValeurVariableLCR(cmd,"NUM=",NumTel,SZ_NUM_TEL))
  {
  DemandeAppelVoix(NumTel,EDTA_AUCUNE,DureeAch,Urgent,0,0,2,Court);

  Lg=StrCopyPgmToChar(Reponse,DEB_REP_LOCALE"APPEL NUM=");
  Lg+=StrCopyToChar(&Reponse[Lg],NumTel);
  Lg+=StrCopyPgmToChar(&Reponse[Lg]," TPA=");
  Lg+=uchartoa(DureeAch,&Reponse[Lg],1);
  Lg+=StrCopyPgmToChar(&Reponse[Lg]," URG=");
  Lg+=uchartoa(Urgent,&Reponse[Lg],1);
  Lg+=StrCopyPgmToChar(&Reponse[Lg]," COURT=");
  Lg+=uchartoa(Court,&Reponse[Lg],1);
  Lg+=StrCopyPgmToChar(&Reponse[Lg]," DEM=1");
  }
}




#ifdef GAINDEPLACE
void Traite_ALARME(unsigned char *cmd,unsigned char *numSMS)
{
unsigned char Tmp[3+1];
unsigned char Lg;

unsigned char MinuteAlarme;
unsigned char HeureAlarme;
unsigned char AlarmePositionnee;

unsigned char Ctrl2;
unsigned char Ck;   // Clockout control
unsigned char Tc;
unsigned char Tv;

/*
if(ValeurVariableLCR(cmd,"CK=",Tmp,3))
  {
  MinuteAlarme=strtouc(Tmp);
  EcritureRegistre_8563(ADR_8563_CLKOUT_CTRL,MinuteAlarme);
  }

if(ValeurVariableLCR(cmd,"CR=",Tmp,3))
  {
  MinuteAlarme=strtouc(Tmp);
  EcritureControlRegister2_8563(MinuteAlarme);
  }

if(ValeurVariableLCR(cmd,"TC=",Tmp,3))
  {
  Tc=strtouc(Tmp);
  EcritureRegistre_8563(ADR_8563_TIMER_CTRL,Tc);
  }

if(ValeurVariableLCR(cmd,"TV=",Tmp,3))
  {
  Tv=strtouc(Tmp);
  EcritureRegistre_8563(ADR_8563_TIMER_VAL,Tv);
  }
*/

LectureAlarme_8563(&HeureAlarme,&MinuteAlarme,&AlarmePositionnee);

LectureControlRegister2_8563(&Ctrl2);

LectureRegistre_8563(ADR_8563_TIMER_CTRL,&Tc);
Tc&=0x83;
LectureRegistre_8563(ADR_8563_TIMER_VAL,&Tv);

LectureRegistre_8563(ADR_8563_CLKOUT_CTRL,&Ck);
Ck&=0x83;

Lg=StrCopyPgmToChar(Reponse,DEB_REP_LOCALE"ALARME H=");
Lg+=uchartoa(HeureAlarme,&Reponse[Lg],2);
Lg+=StrCopyPgmToChar(&Reponse[Lg]," M=");
Lg+=uchartoa(MinuteAlarme,&Reponse[Lg],2);
Lg+=StrCopyPgmToChar(&Reponse[Lg]," ACT=");
Lg+=uchartoa(AlarmePositionnee,&Reponse[Lg],1);

Lg+=StrCopyPgmToChar(&Reponse[Lg]," CR=0x");
Lg+=uchartoHexa(Ctrl2,&Reponse[Lg]);

Lg+=StrCopyPgmToChar(&Reponse[Lg]," TC=0x");
Lg+=uchartoHexa(Tc,&Reponse[Lg]);

Lg+=StrCopyPgmToChar(&Reponse[Lg]," TV=");
Lg+=uchartoa(Tv,&Reponse[Lg],2);

Lg+=StrCopyPgmToChar(&Reponse[Lg]," CK=0x");
Lg+=uchartoHexa(Ck,&Reponse[Lg]);

Lg+=StrCopyPgmToChar(&Reponse[Lg]," ERR=");
Lg+=uchartoHexa(ErrPCF8363,&Reponse[Lg]);
}

#endif
      

/*
void Traite_BIP(unsigned char *cmd,unsigned char *numSMS)
Appellé a la réception d'une commande BIP
BIP N=n 
ENTREE : cmd = Commande recue
         num = Numero du SMS si commande par SMS ou NULL si port serie 
SORTIE : Variable globale Reponse
*/
void Traite_BIP(unsigned char *cmd,unsigned char *numSMS)
{
unsigned char Lg;
unsigned char NbBip;
unsigned char Tmp[3+1];
unsigned char LongBip;

NbBip=0;
LongBip=BIP_LONG;
if(ValeurVariableLCR(cmd,"N=",Tmp,2))
  {
  NbBip=strtouc(Tmp);
  /*
  if(ValeurVariableLCR(cmd,"L=",Tmp,2))
    {
    if(Tmp[0]=='0')
      LongBip=BIP_COURT;
    }
  */
  NbBip=FaitBip(NbBip,LongBip);
  }
Lg=StrCopyPgmToChar(Reponse,DEB_REP_LOCALE"BIP N=");
Lg+=uchartoa(NbBip,&Reponse[Lg],0);
/*
Lg+=StrCopyPgmToChar(&Reponse[Lg]," L=");
Lg+=uchartoa(NbBip,&Reponse[Lg],LongBip);
*/
}


/*
void Traite_CFA(unsigned char *cmd,unsigned char *numSMS)
Appellé a la réception d'une commande CFA
ENTREE : cmd = Commande recue
         num = Numero du SMS si commande par SMS ou NULL si port serie 
SORTIE : Variable globale Reponse
*/
void Traite_CFA(unsigned char *cmd,unsigned char *numSMS)
{
unsigned char Ligne;
unsigned char Tmp[SZ_MAX_CH_CFA+1];
unsigned char Ch2[2];
unsigned int EEaddrBaseLigne;        // Adresse de base en EEPROM de la ligne CFA

unsigned char MsqSetEDC;
unsigned char MsqResetEDC;
unsigned char ii;

if(ValeurVariableLCR(cmd," L=",Tmp,2))
  {
  Ligne=strtouc(Tmp);
  if(Ligne<NB_LIGNES_TAB_CFA)
    {
    //EEaddrBaseLigne = ADRESSE_BASE_0_CFA + (Ligne *OFFSET_INTER_LIGNE_CFA) voir ligne 774
      
    // Detailler sinon erreur cause char / int
    EEaddrBaseLigne=Ligne;
    EEaddrBaseLigne=EEaddrBaseLigne*OFFSET_INTER_LIGNE_CFA;
    EEaddrBaseLigne=EEaddrBaseLigne+ADRESSE_BASE_0_CFA;
    if(ValeurVariableLCR(cmd,"EFFACE=",Tmp,1))
      {
      if(Tmp[0]=='1')
        {
        // Efface = 1 met à 0 tous les champs
        memset(Tmp,0,SZ_MAX_CH_CFA+1);
        WriteI2c(EEPROM, EEaddrBaseLigne+OFF_NUM_TEL,SZ_NUM_TEL,Tmp,&CodeErreurI2c,&TimeOutI2c);
        WriteI2c(EEPROM, EEaddrBaseLigne+OFF_NUM_SMS,SZ_NUM_TEL,Tmp,&CodeErreurI2c,&TimeOutI2c);
        WriteI2c(EEPROM, EEaddrBaseLigne+OFF_TPA,1,Tmp,&CodeErreurI2c,&TimeOutI2c);
        WriteI2c(EEPROM, EEaddrBaseLigne+OFF_TXT_SMS,SZ_TXT_SMS_APPEL,Tmp,&CodeErreurI2c,&TimeOutI2c);
        WriteI2c(EEPROM, EEaddrBaseLigne+OFF_TYPE_SMS,1,Tmp,&CodeErreurI2c,&TimeOutI2c);
        WriteI2c(EEPROM, EEaddrBaseLigne+OFF_CAR_ECDM,1,Tmp,&CodeErreurI2c,&TimeOutI2c);
        }
	  }

    if(ValeurVariableLCR(cmd," V=",Tmp,SZ_NUM_TEL))
      {
      if(Tmp[0]=='Z')
        Tmp[0]=0;
      WriteI2c(EEPROM, EEaddrBaseLigne+OFF_NUM_TEL,SZ_NUM_TEL,Tmp,&CodeErreurI2c,&TimeOutI2c);
      }
    if(ValeurVariableLCR(cmd," S=",Tmp,SZ_NUM_TEL))
      {
      if(Tmp[0]=='Z')
        Tmp[0]=0;
      WriteI2c(EEPROM, EEaddrBaseLigne+OFF_NUM_SMS,SZ_NUM_TEL,Tmp,&CodeErreurI2c,&TimeOutI2c);
      }
    if(ValeurVariableLCR(cmd," T=",Tmp,3))
      {
      Ch2[0]=strtouc(Tmp);
      WriteI2c(EEPROM, EEaddrBaseLigne+OFF_TPA,1,Ch2,&CodeErreurI2c,&TimeOutI2c);
      }
    if(ValeurVariableLCR(cmd," M=",Tmp,SZ_TXT_SMS_APPEL))
      {
      if(Tmp[0]=='Z')
        Tmp[0]=0;
      WriteI2c(EEPROM, EEaddrBaseLigne+OFF_TXT_SMS,SZ_TXT_SMS_APPEL,Tmp,&CodeErreurI2c,&TimeOutI2c);
      }

    if(ValeurVariableLCR(cmd," I=",Tmp,3))
      {
      Ch2[0]=strtouc(Tmp);
      WriteI2c(EEPROM, EEaddrBaseLigne+OFF_TYPE_SMS,1,Ch2,&CodeErreurI2c,&TimeOutI2c);
      }

    MsqSetEDC=0;
    MsqResetEDC=0;
    if(ValeurVariableLCR(cmd," E=",Tmp,4))  // !! laisser espace pour EFFACE=1 ok
      {
      ii=0;
      while(ii<NB_E_TOR)
        {
        switch(Tmp[ii])
          {
          case '0':
            MsqResetEDC|=MSQ_CAR_ECDM_E(ii);
            break;
          case '1':
            MsqSetEDC|=MSQ_CAR_ECDM_E(ii);
            break;
          case 0:
            ii=NB_E_TOR;
            break;
          }
        ii++;
		}
      }

    if(ValeurVariableLCR(cmd," D=",Tmp,1))
      {
      switch(Tmp[0])
        {
        case '0':
          MsqResetEDC|=MSQ_CAR_ECDM_DTMF;
          break;
        case '1':
          MsqSetEDC|=MSQ_CAR_ECDM_DTMF;
          break;
        }
      }

    if(ValeurVariableLCR(cmd," C=",Tmp,1))
      {
      switch(Tmp[0])
        {
        case '0':
          MsqResetEDC|=MSQ_CAR_ECDM_CYCLIQUE;
          break;
        case '1':
          MsqSetEDC|=MSQ_CAR_ECDM_CYCLIQUE;
          break;
        }
      }

    if(ValeurVariableLCR(cmd," X=",Tmp,1))
      {
      switch(Tmp[0])
        {
        case '0':
          MsqResetEDC|=MSQ_CAR_ECDM_NUM_EXT;
          break;
        case '1':
          MsqSetEDC|=MSQ_CAR_ECDM_NUM_EXT;
          break;
        }
      }

    if((MsqSetEDC)||(MsqResetEDC))
      {
      ReadI2c(EEPROM, EEaddrBaseLigne+OFF_CAR_ECDM,1,Tmp,&CodeErreurI2c,&TimeOutI2c);
      Tmp[1]=Tmp[0];
      Tmp[0]|=MsqSetEDC;
      Tmp[0]&=~MsqResetEDC;

      if(Tmp[1]!=Tmp[0])
        WriteI2c(EEPROM, EEaddrBaseLigne+OFF_CAR_ECDM,1,Tmp,&CodeErreurI2c,&TimeOutI2c);
      }

    RepondCFA(Ligne);
    }
  }
}




/*
void Traite_CFPAU(unsigned char *cmd,unsigned char *numSMS)
Appellé a la réception d'une commande CFPAU 
ENTREE : cmd = Commande recue
         num = Numero du SMS si commande par SMS ou NULL si port serie 
SORTIE : Variable globale Reponse
*/
void Traite_CFPAU(unsigned char *cmd,unsigned char *numSMS)
{
static unsigned char Tel[SZ_NUM_TEL+1];
static unsigned char TelMem[SZ_NUM_TEL+1];
unsigned char Dtmf[SZ_DTMF_PAU+1];
unsigned char Htj[SZ_HTJ+1];
unsigned char Vt[1+1];
unsigned char Lg;
unsigned char stop;
unsigned char Ds;
unsigned char *pt;
unsigned char EcrirePcf8563;
unsigned char CurDMCMin;
uint16 ValTmp;

EcrirePcf8563=0;

if(ValeurVariableLCR(cmd,"DMC=",NomPau,SZ_DMC))
  {
  CurDMCMin=strtouc(NomPau);
  WriteI2c(EEPROM, ADR_DMC_MIN,1,&CurDMCMin,&CodeErreurI2c,&TimeOutI2c);
  }
ReadI2c(EE_SLAVE_ADDR_2,ADR_DMC_MIN,1,&CurDMCMin,&CodeErreurI2c,&TimeOutI2c);    //   (tempo de comm maxi en minute dans eeprom )
// Met a jour la variable en secondes
CurDMCSec=(unsigned int)(CurDMCMin);
CurDMCSec=CurDMCSec*60;

if(ValeurVariableLCR(cmd,"RTR=",NomPau,3))
  {
  PositionneBitOptionsSoft(MSQ_REBOOT_TEST_ROAMING,(NomPau[0]=='1'));
  }
if(ValeurVariableLCR(cmd,"TAC=",NomPau,3))
  {
  PositionneBitOptionsSoft(MSQ_OPTIONS_TST_AUDIO_AAEC,(NomPau[0]=='1'));
  }

if(ValeurVariableLCR(cmd,"MDSC=",NomPau,5))
  {
  ValTmp=strtoui(NomPau);
  if(ValTmp!=MsqDefautsSmsChgt)
    {
    WriteI2c(EEPROM,ADR_MSQ_CHGT_DEF_SMS,2,(unsigned char*)ValTmp,&CodeErreurI2c,&TimeOutI2c); 
    ReadI2c(EEPROM,ADR_MSQ_CHGT_DEF_SMS,2,(unsigned char*)MsqDefautsSmsChgt,&CodeErreurI2c,&TimeOutI2c); //&MsqDefautsSmsChgt
    }
  }

if(ValeurVariableLCR(cmd,"WDSV=",NomPau,3))
  {
  MsqWdSmsVerifNum=strtouc(NomPau);
  WriteI2c(EEPROM, ADR_MWD_SMS_VERIFNUM,1,&MsqWdSmsVerifNum,&CodeErreurI2c,&TimeOutI2c);
  ReadI2c(EEPROM, ADR_MWD_SMS_VERIFNUM,1,&MsqWdSmsVerifNum,&CodeErreurI2c,&TimeOutI2c); //&MsqWdSmsVerifNum
  }

if(ValeurVariableLCR(cmd,"WDST=",NomPau,3))
  {
  MsqWdSmsTest=strtouc(NomPau);
  WriteI2c(EEPROM, ADR_MWD_SMS_TEST,1,&MsqWdSmsTest,&CodeErreurI2c,&TimeOutI2c);
  ReadI2c(EEPROM, ADR_MWD_SMS_TEST,1,&MsqWdSmsTest,&CodeErreurI2c,&TimeOutI2c); //&MsqWdSmsTest
  }

if(ValeurVariableLCR(cmd,"WDSG=",NomPau,3))
  {
  MsqWdSmsGps=strtouc(NomPau);
  WriteI2c(EEPROM, ADR_MWD_SMS_GPS,1,&MsqWdSmsGps,&CodeErreurI2c,&TimeOutI2c);
  ReadI2c(EEPROM, ADR_MWD_SMS_GPS,1,&MsqWdSmsGps,&CodeErreurI2c,&TimeOutI2c); //&MsqWdSmsGps
  }

if(ValeurVariableLCR(cmd,"WDVA=",NomPau,3))
  {
    MsqWdSmsVana=strtouc(NomPau);
    WriteI2c(EEPROM, ADR_MWD_SMS_VANA,1,&MsqWdSmsVana,&CodeErreurI2c,&TimeOutI2c);
    ReadI2c(EEPROM, ADR_MWD_SMS_VANA,1,&MsqWdSmsVana,&CodeErreurI2c,&TimeOutI2c); //&MsqWdSmsVana
  }

if(ValeurVariableLCR(cmd,"DS=",NomPau,3))
  {
  Ds=strtouc(NomPau);
  if(Ds<1) 
    Ds=CFPAU_DS_DEFAUT;
  WriteI2c(EEPROM, ADR_DS_PAU,1,&Ds,&CodeErreurI2c,&TimeOutI2c);
  EcrirePcf8563=1;
  }

if(ValeurVariableLCR(cmd,"HTJ=",Htj,SZ_HTJ))
  {
  pt=Htj;
  stop=0;
  if(DecodeChaine(pt, ':', &HeureMinuteTest[0], &stop))
    {
    pt+=stop+1;
    if(DecodeChaine(pt, ' ', &HeureMinuteTest[1], &stop))
      {
      WriteI2c(EEPROM, ADR_2O_HTJ,2,HeureMinuteTest,&CodeErreurI2c,&TimeOutI2c);
      EcrirePcf8563=1;
      }
    }
  }

ReadI2c(EEPROM, ADR_2O_HTJ,2,HeureMinuteTest,&CodeErreurI2c,&TimeOutI2c);

if(ValeurVariableLCR(cmd,"NOM=",NomPau,SZ_NOM_PAU))
  {
  WriteI2c(EEPROM, ADR_NOM_PAU,SZ_NOM_PAU,NomPau,&CodeErreurI2c,&TimeOutI2c); 
  }
memset(NomPau,0,SZ_NOM_PAU);
//ReadI2c(EEPROM, ADR_NOM_PAU,SZ_NOM_PAU,NomPau,&CodeErreurI2c,&TimeOutI2c); ne prend pas les tableaux de char
NomPau[SZ_NOM_PAU]=0;

if(ValeurVariableLCR(cmd,"DTMF=",Dtmf,SZ_DTMF_PAU))
  {
  WriteI2c(EEPROM, ADR_DTMF_PAU,SZ_DTMF_PAU,Dtmf,&CodeErreurI2c,&TimeOutI2c);
  }
memset(Dtmf,0,SZ_DTMF_PAU);
ReadI2c(EEPROM, ADR_DTMF_PAU,SZ_DTMF_PAU,Dtmf,&CodeErreurI2c,&TimeOutI2c); 
Dtmf[SZ_DTMF_PAU]=0;

if(ValeurVariableLCR(cmd,"TEL=",Tel,SZ_NUM_TEL))
  {
  memset(TelMem,0,SZ_NUM_TEL);
  ReadI2c(EEPROM, ADR_TEL_PAU,SZ_NUM_TEL,TelMem,&CodeErreurI2c,&TimeOutI2c); 
  TelMem[SZ_NUM_TEL]=0;

  if(strcmp((char*)TelMem,(char*)Tel))
    {
    // Numero different du parametré, on demande sa verification
    WriteI2c(EEPROM, ADR_TEL_PAU,SZ_NUM_TEL,Tel,&CodeErreurI2c,&TimeOutI2c); 
    VerifNumPau=EVNP_NUM_DEMANDEE;
    // Positionne a zero l'indicateur de verification
    StrCopyPgmToChar(Vt,"0");
    WriteI2c(EEPROM, ADR_VT_PAU,1,Vt,&CodeErreurI2c,&TimeOutI2c); 
    }
  }
memset(Tel,0,SZ_NUM_TEL);
ReadI2c(EEPROM, ADR_TEL_PAU,SZ_NUM_TEL,Tel,&CodeErreurI2c,&TimeOutI2c); 
Tel[SZ_NUM_TEL]=0;

memset(Vt,0,2);
ReadI2c(EEPROM, ADR_VT_PAU,1,Vt,&CodeErreurI2c,&TimeOutI2c);
Vt[1]=0;

ReadI2c(EEPROM, ADR_DS_PAU,1,&Ds,&CodeErreurI2c,&TimeOutI2c); // &Ds 

if(EcrirePcf8563)
  //PositionneDonneesIT_8563(HeureMinuteTest[0],HeureMinuteTest[1],Ds); Nathan

Lg=StrCopyPgmToChar(Reponse,DEB_REP_LOCALE"CFPAU NOM=");
Lg+=StrCopyToChar(&Reponse[Lg],NomPau);
Lg+=StrCopyPgmToChar(&Reponse[Lg]," DTMF=");
Lg+=StrCopyToChar(&Reponse[Lg],Dtmf);
Lg+=StrCopyPgmToChar(&Reponse[Lg]," HTJ=");
Lg+=uchartoa(HeureMinuteTest[0],&Reponse[Lg],2);
Lg+=StrCopyPgmToChar(&Reponse[Lg],":");
Lg+=uchartoa(HeureMinuteTest[1],&Reponse[Lg],2);
Lg+=StrCopyPgmToChar(&Reponse[Lg]," DMC=");
Lg+=uchartoa(CurDMCMin,&Reponse[Lg],2);
Lg+=StrCopyPgmToChar(&Reponse[Lg]," TEL=");
Lg+=StrCopyToChar(&Reponse[Lg],Tel);
Lg+=StrCopyPgmToChar(&Reponse[Lg]," DS=");
Lg+=uchartoa(Ds,&Reponse[Lg],0);
Lg+=StrCopyPgmToChar(&Reponse[Lg]," VT=");
Lg+=StrCopyToChar(&Reponse[Lg],Vt);
Lg+=StrCopyPgmToChar(&Reponse[Lg]," MDSC=");
Lg+=uinttoa(MsqDefautsSmsChgt,&Reponse[Lg]);

/*
Lg+=StrCopyPgmToChar(&Reponse[Lg]," MVSC=");
if((OptionsSoft&MSQ_OPTIONS_VANA_CHGT)==MSQ_OPTIONS_VANA_CHGT)
  Lg+=StrCopyPgmToChar(&Reponse[Lg],"1");   
else
  Lg+=StrCopyPgmToChar(&Reponse[Lg],"0");
Lg+=StrCopyPgmToChar(&Reponse[Lg]," MVST=");
if((OptionsSoft&MSQ_OPTIONS_VANA_TEST)==MSQ_OPTIONS_VANA_TEST)
  Lg+=StrCopyPgmToChar(&Reponse[Lg],"1");   
else
  Lg+=StrCopyPgmToChar(&Reponse[Lg],"0");
*/

Lg+=StrCopyPgmToChar(&Reponse[Lg]," RTR=");
if((OptionsSoft&MSQ_REBOOT_TEST_ROAMING)==MSQ_REBOOT_TEST_ROAMING)
  Lg+=StrCopyPgmToChar(&Reponse[Lg],"1");   
else
  Lg+=StrCopyPgmToChar(&Reponse[Lg],"0");

Lg+=StrCopyPgmToChar(&Reponse[Lg]," WDSV=");
Lg+=uchartoa(MsqWdSmsVerifNum,&Reponse[Lg],0);

Lg+=StrCopyPgmToChar(&Reponse[Lg]," WDST=");
Lg+=uchartoa(MsqWdSmsTest,&Reponse[Lg],0);

Lg+=StrCopyPgmToChar(&Reponse[Lg]," WDSG=");
Lg+=uchartoa(MsqWdSmsGps,&Reponse[Lg],0);

Lg+=StrCopyPgmToChar(&Reponse[Lg]," WDVA=");
Lg+=uchartoa(MsqWdSmsVana,&Reponse[Lg],0);

Lg+=StrCopyPgmToChar(&Reponse[Lg]," TAC=");
if((OptionsSoft&MSQ_OPTIONS_TST_AUDIO_AAEC)==MSQ_OPTIONS_TST_AUDIO_AAEC)
  Lg+=StrCopyPgmToChar(&Reponse[Lg],"1");   
else
  Lg+=StrCopyPgmToChar(&Reponse[Lg],"0");

}


/*
void Traite_CFPIN(unsigned char *cmd,unsigned char *numSMS)
Appellé a la réception d'une commande CFPAU 
ENTREE : cmd = Commande recue
         num = Numero du SMS si commande par SMS ou NULL si port serie 
SORTIE : Variable globale Reponse
*/
void Traite_CFPIN(unsigned char *cmd,unsigned char *numSMS)
{
unsigned char CodePin[SZ_CODE_PIN+1];
unsigned char Lg;
unsigned char Valide;

if((numSMS==NULL)&&ValeurVariableLCR(cmd,"PIN=",CodePin,SZ_CODE_PIN))
  {
  CodePin[SZ_CODE_PIN]=0;
  WriteI2c(EEPROM, ADR_CODE_PIN,SZ_CODE_PIN,CodePin,&CodeErreurI2c,&TimeOutI2c); 
  }

memset(CodePin,0,SZ_CODE_PIN);
ReadI2c(EEPROM, ADR_CODE_PIN,SZ_CODE_PIN,CodePin,&CodeErreurI2c,&TimeOutI2c);
CodePin[SZ_CODE_PIN]=0;

Valide=CodePinValide(CodePin);

Lg=StrCopyPgmToChar(Reponse,DEB_REP_LOCALE"CFPIN PIN=");
Lg+=StrCopyToChar(&Reponse[Lg],CodePin);
Lg+=StrCopyPgmToChar(&Reponse[Lg]," CV=");
Lg+=uchartoa(Valide,&Reponse[Lg],0);
}



/*
void LectureCFS(unsigned char Ligne)
Lecture en flah des parametres d'une ligne CFS (Appellée au demarrage et avant réponse)
ENTREE : Ligne = Numero de la ligne ( 0 à NB_LIGNES_TAB_CFS-1)
SORTIE : Variable globale EquaCFS[Ligne]
RETOUR : Aucun
*/
void LectureCFS(unsigned char Ligne)
{
unsigned char DataEE[OFFSET_INTER_LIGNE_CFS+1];
unsigned int EEaddrBaseLigne;        // Adresse de base en EEPROM de la ligne CFS

if(Ligne<NB_LIGNES_TAB_CFS)
  {
  // EEaddrBaseLigne = ADRESSE_BASE_0_CFS + (Ligne *OFFSET_INTER_LIGNE_CFS) 
  // Detailler sinon erreur cause char / int
  EEaddrBaseLigne=Ligne;
  EEaddrBaseLigne=EEaddrBaseLigne*OFFSET_INTER_LIGNE_CFS;
  EEaddrBaseLigne=EEaddrBaseLigne+ADR_BASE_CFS;
 
 // Lit les donnéees d'un bloc
  ReadI2c(EE_SLAVE_ADDR_2, EEaddrBaseLigne,OFFSET_INTER_LIGNE_CFS,DataEE,&CodeErreurI2c,&TimeOutI2c);

  EquaCFS[Ligne].Fixe=DataEE[OFF_CFS_FIXE];
  EquaCFS[Ligne].Clig=DataEE[OFF_CFS_CLIG];
  EquaCFS[Ligne].Inv=DataEE[OFF_CFS_INV];
  }
}


/*
void RepondCFS(unsigned char Ligne)
Appellé pour répondre à une commande CFS
renvoie le paramétrage d'une ligne CFS
ENTREE : Ligne = Numero de la ligne ( 0 à NB_LIGNES_TAB_CFS-1)
SORTIE : Variable globale Reponse
RETOUR : Aucun
*/
void RepondCFS(unsigned char Ligne)
{
unsigned char Lg;

if(Ligne<NB_LIGNES_TAB_CFS)
  {
  // EEaddrBaseLigne = ADRESSE_BASE_0_CFS + (Ligne *OFFSET_INTER_LIGNE_CFS) 
  // Detailler sinon erreur cause char / int
  LectureCFS(Ligne);

  Lg=StrCopyPgmToChar(Reponse,DEB_REP_LOCALE"CFS S=");
  Lg+=uchartoa(Ligne+1,&Reponse[Lg],0);
  
  Lg+=StrCopyPgmToChar(&Reponse[Lg]," F=");
  Lg+=uchartoa(EquaCFS[Ligne].Fixe,&Reponse[Lg],0);

  Lg+=StrCopyPgmToChar(&Reponse[Lg]," C=");
  Lg+=uchartoa(EquaCFS[Ligne].Clig,&Reponse[Lg],0);

  Lg+=StrCopyPgmToChar(&Reponse[Lg]," I=");
  Lg+=uchartoa(EquaCFS[Ligne].Inv,&Reponse[Lg],0);
  }
}




/*
void Traite_CFS(unsigned char *cmd,unsigned char *numSMS)
Appellé a la réception d'une commande CFS
ENTREE : cmd = Commande recue
         num = Numero du SMS si commande par SMS ou NULL si port serie 
SORTIE : Variable globale Reponse
*/
void Traite_CFS(unsigned char *cmd,unsigned char *numSMS)
{
unsigned char Ligne;
unsigned char Tmp[3+1];
unsigned char ValChar[2];

unsigned int EEaddrBaseLigne;        // Adresse de base en EEPROM de la ligne CFA


if(ValeurVariableLCR(cmd," S=",Tmp,2))
  {
  Ligne=strtouc(Tmp);
  if((Ligne>0)&&(Ligne<=NB_LIGNES_TAB_CFS))    // Sorties reperees 1 à 4, dans tableau 0 - 3
    {
    Ligne=Ligne-1;   
    // EEaddrBaseLigne = ADRESSE_BASE_0_CFS + (Ligne *OFFSET_INTER_LIGNE_CFS) 
    // Detailler sinon erreur cause char / int
    EEaddrBaseLigne=Ligne;
    EEaddrBaseLigne=EEaddrBaseLigne*OFFSET_INTER_LIGNE_CFS;
    EEaddrBaseLigne=EEaddrBaseLigne+ADR_BASE_CFS;

    // Pas de relecture car les valeurs modifiees sont relues par la fonction LectureCFS appellée dans RepondCFS.
    if(ValeurVariableLCR(cmd," F=",Tmp,3))
      {
      ValChar[0]=strtouc(Tmp);
      WriteI2c(EEPROM, EEaddrBaseLigne+OFF_CFS_FIXE,1,ValChar,&CodeErreurI2c,&TimeOutI2c);
      }
    if(ValeurVariableLCR(cmd," C=",Tmp,3))
      {
      ValChar[0]=strtouc(Tmp);
      WriteI2c(EEPROM, EEaddrBaseLigne+OFF_CFS_CLIG,1,ValChar,&CodeErreurI2c,&TimeOutI2c); 
      }
    if(ValeurVariableLCR(cmd," I=",Tmp,3))
      {
      ValChar[0]=strtouc(Tmp);
      WriteI2c(EEPROM, EEaddrBaseLigne+OFF_CFS_INV,1,ValChar,&CodeErreurI2c,&TimeOutI2c);
      }
    RepondCFS(Ligne);
    }
  }
}



/*
void Traite_CPT(unsigned char *cmd,unsigned char *numSMS)
Appellé a la réception d'une commande CPT
ENTREE : cmd = Commande recue
         num = Numero du SMS si commande par SMS ou NULL si port serie 
SORTIE : Variable globale Reponse
*/
void Traite_CPT(unsigned char *cmd,unsigned char *numSMS)
{
unsigned char Lg;
unsigned char Tmp[3+1];

if(ValeurVariableLCR(cmd,"RAZ=",Tmp,1))
  {
  Lg=strtouc(Tmp);
  if(Lg&1)
    SMSEntrant.Cpt=0;
  if(Lg&2)
    SMSSortant.Cpt=0;
  }

if(ValeurVariableLCR(cmd,"MJA=",Tmp,3))
  {
  Lg=strtouc(Tmp);
  if(Lg!=MaxSmsJourAutoAutorise)
    {
    WriteI2c(EEPROM, ADR_MAX_SMS_AUTO_JOUR,1,&Lg,&CodeErreurI2c,&TimeOutI2c);
    ReadI2c(EEPROM, ADR_MAX_SMS_AUTO_JOUR,1,&MaxSmsJourAutoAutorise,&CodeErreurI2c,&TimeOutI2c); // &MaxSmsJourAutoAutorise
    }
  }


Lg=StrCopyPgmToChar(Reponse,DEB_REP_LOCALE"CPT CPE=");   // Total entrant
Lg+=ulongtoa(SMSEntrant.Cpt,&Reponse[Lg],12);

Lg+=StrCopyPgmToChar(&Reponse[Lg]," CPS=");              // Total sortant
Lg+=ulongtoa(SMSSortant.Cpt,&Reponse[Lg],12);

Lg+=StrCopyPgmToChar(&Reponse[Lg]," EJ=");              // Entrant jour
Lg+=uchartoa(SMSEntrant.CptJour ,&Reponse[Lg],0);

Lg+=StrCopyPgmToChar(&Reponse[Lg]," EH=");              // Entrant hier
Lg+=uchartoa(SMSEntrant.CptHier ,&Reponse[Lg],0);

Lg+=StrCopyPgmToChar(&Reponse[Lg]," SJ=");              // Sortant jour
Lg+=uchartoa(SMSSortant.CptJour ,&Reponse[Lg],0);

Lg+=StrCopyPgmToChar(&Reponse[Lg]," SH=");             // Sortant hier
Lg+=uchartoa(SMSSortant.CptHier ,&Reponse[Lg],0);

Lg+=StrCopyPgmToChar(&Reponse[Lg]," SJA=");              // Sortant jour a l'initiative de la borne
Lg+=uchartoa(SMSSortant.CptJourAuto ,&Reponse[Lg],0);

Lg+=StrCopyPgmToChar(&Reponse[Lg]," SHA=");             // Sortant hier a l'initiative de la borne
Lg+=uchartoa(SMSSortant.CptHierAuto ,&Reponse[Lg],0);

Lg+=StrCopyPgmToChar(&Reponse[Lg]," MJA=");             // Max jour auto aotorisé
Lg+=uchartoa(MaxSmsJourAutoAutorise ,&Reponse[Lg],0);

Tmp[0]=EnvoiSmsAutoAutorise(0);
Lg+=StrCopyPgmToChar(&Reponse[Lg]," EA=");             // Envoi autorise
Lg+=uchartoa(Tmp[0] ,&Reponse[Lg],0);

}



#if 0
unsigned char EcritHeureDansBuffer(struct STR_DATE *pt,unsigned char *Buffer)
{
unsigned char Retour;

Retour=uchartoa(pt->Jour,Buffer,2);
Retour+=StrCopyPgmToChar(&Buffer[Retour],"/");
Retour+=uchartoa(pt->Mois,&Buffer[Retour],2);
Retour+=StrCopyPgmToChar(&Buffer[Retour],"/");
Retour+=uchartoa(pt->An,&Buffer[Retour],2);        // Attendre Nathan
Retour+=StrCopyPgmToChar(&Buffer[Retour],"_");
Retour+=uchartoa(pt->Heure,&Buffer[Retour],2);
Retour+=StrCopyPgmToChar(&Buffer[Retour],":");
Retour+=uchartoa(pt->Minute,&Buffer[Retour],2);
Retour+=StrCopyPgmToChar(&Buffer[Retour],":");
Retour+=uchartoa(pt->Seconde,&Buffer[Retour],2);

return(Retour);
}
#endif

/*
void Traite_DT(unsigned char *cmd,unsigned char *numSMS,char FtAMJ_VT)
Appellé a la réception d'une commande DT
ENTREE : cmd      = Commande recue
         num      = Numero du SMS si commande par SMS ou NULL si port serie 
         FtAMJ_VT = Si 1, la fonction est appellé par la verification de numero pour synchro, 
                    le jour et l'annee sont inversé. Il ne faut pas repondre
SORTIE : Variable globale Reponse
*/
#if 0
void Traite_DT(unsigned char *cmd,unsigned char *numSMS,char FtAMJ_VT)
{
//struct STR_DATE pt;
unsigned char Lg;
unsigned char stop;
unsigned char Tmp[2];


// Saute le DT
while((*cmd)&&(*cmd!=' ')) cmd++;
// Saute les espaces
while(*cmd==' ') cmd++;

if(*cmd)
  {
  //memset(&pt,0,sizeof(pt)); attendre Nathan
  stop=0;
  if(DecodeChaine(cmd, '/', &pt.Jour, &stop))
    {
    cmd+=stop+1;
    //if(DecodeChaine(cmd, '/', &pt.Mois, &stop))
    if(DecodeChaine(cmd, '/', &Lg, &stop))
      {
      pt.Mois = Lg;
      cmd+=stop+1;
      if(DecodeChaine(cmd, ' ', &pt.An, &stop))
        {
        cmd+=stop;
        // Saute les espaces
        while(*cmd==' ') cmd++;
        if(DecodeChaine(cmd, ':', &pt.Heure, &stop))
          {
          cmd+=stop+1;
          if(DecodeChaine(cmd, ':', &pt.Minute, &stop))
            {
            cmd+=stop+1;
            if(DecodeChaine(cmd, ' ', &pt.Seconde, &stop))
              {
              if(FtAMJ_VT)
                {
                Lg=pt.Jour;
                pt.Jour=pt.An;
                pt.An=Lg;
                }
              // Ecriture de l'heure dans le circuit horloge    // attendre Nathan
              if(ValeurVariableLCR(cmd," WD=",Tmp,1))
                {
                Lg=Tmp[0]-'0';
                }
              else
                {
                Lg=0xff;
                }
              //EcritureDateHeure(&pt,Lg);attendre Nathan
              //LectureDateHeure(&DateCourante);attendre Nathan
              }
            }
          } 
        }
      }
    } 
  }

// Lecture de l'heure
if((FtAMJ_VT==0)&&(Silence==0))
  {
  Lg=StrCopyPgmToChar(Reponse,DEB_REP_LOCALE"DT ");
//  Lg+=EcritHeureDansBuffer(&DateCourante,&Reponse[Lg]); attendre Nathan
  Lg+=StrCopyPgmToChar(&Reponse[Lg]," WD=");
 // Lg+=uchartoa(DateCourante.wd,&Reponse[Lg],1);
  }
}

#endif
/*
void Traite_FREQ(unsigned char *cmd,unsigned char *numSMS)
Appellé a la réception d'une commande FREQ
FREQ ACT=1
ENTREE : cmd = Commande recue
         num = Numero du SMS si commande par SMS ou NULL si port serie 
SORTIE : Variable globale Reponse
*/
void Traite_FREQ(unsigned char *cmd,unsigned char *numSMS)
{
unsigned char Tmp[6+1];
unsigned char Act;
unsigned char Lg;

if(ValeurVariableLCR(cmd,"ACT=",Tmp,1))
  {
  Act=strtouc(Tmp);
  }
else
  {
  Act=0;
  }

if(ValeurVariableLCR(cmd,"PT1=",Tmp,5))
  {
  PresetT1_bip=atoi(Tmp);
  if(PresetT1_bip==0)
    PresetT1_bip=PRESET_DEF_TP1;
  }

Lg=StrCopyPgmToChar(Reponse,DEB_REP_LOCALE"FREQ ACT=");
Lg+=uchartoa(Act,&Reponse[Lg],0);

Lg+=StrCopyPgmToChar(&Reponse[Lg]," PT1=");
Lg+=uinttoa(PresetT1_bip,&Reponse[Lg]);



if(Act)
  {  
  PowerAudioAmp(AUDIO_ON);//Allume l'ampli de puissance
  SetAudioChannel(1,0, 0); 
  StartAudioSignal();
  }  
else
  {  
  StopAudioSignal();
  PowerAudioAmp(AUDIO_OFF);//eteint l'ampli de puissance
  }  
}


/*
void Traite_GAIN(unsigned char *cmd,unsigned char *numSMS)
Appellé a la réception d'une commande GAIN
GAIN HPN=hpn MIN=min HPME=hpme MIME=mime
ENTREE : cmd = Commande recue
         num = Numero du SMS si commande par SMS ou NULL si port serie 
SORTIE : Variable globale Reponse
*/
void Traite_GAIN(unsigned char *cmd,unsigned char *numSMS)
{
unsigned char Lg;
unsigned char Tmp[3+1];

if(ValeurVariableLCR(cmd,"HPN=",Tmp,3))
  {
  CurGainHpN=strtouc(Tmp);
  WriteI2c(EEPROM,ADR_G_HPN,1,&CurGainHpN,&CodeErreurI2c,&TimeOutI2c);   //&CurGainHpN 
  ReadI2c(EEPROM,ADR_G_HPN,1,&CurGainHpN,&CodeErreurI2c,&TimeOutI2c);    //   (Hp Normal) // &CurGainHpN
  }
if(ValeurVariableLCR(cmd,"MIN=",Tmp,3))
  {
  CurGainMiN=strtouc(Tmp);
  WriteI2c(EEPROM,ADR_G_MIN,1,&CurGainMiN,&CodeErreurI2c,&TimeOutI2c); // &CurGainMiN
  ReadI2c(EEPROM,ADR_G_MIN,1,&CurGainMiN,&CodeErreurI2c,&TimeOutI2c);    //   (Micro Normal) // &CurGainMiN
  }
if(ValeurVariableLCR(cmd,"HPME=",Tmp,3))
  {
  CurGainHpMe=strtouc(Tmp);
  WriteI2c(EEPROM,ADR_G_HPME,1,&CurGainHpMe,&CodeErreurI2c,&TimeOutI2c);    //&CurGainHpMe
  ReadI2c(EEPROM,ADR_G_HPME,1,&CurGainHpMe,&CodeErreurI2c,&TimeOutI2c);    //   (Hp mal entendant) //&CurGainHpMe
  }
if(ValeurVariableLCR(cmd,"MIME=",Tmp,3))
  {
  CurGainMiMe=strtouc(Tmp);
  WriteI2c(EEPROM,ADR_G_MIME,1,&CurGainMiMe,&CodeErreurI2c,&TimeOutI2c);    // &CurGainMiMe
  ReadI2c(EEPROM,ADR_G_MIME,1,&CurGainMiMe,&CodeErreurI2c,&TimeOutI2c);    //   (Micro mal entendant)// &CurGainMiMe
  }

Lg=StrCopyPgmToChar(Reponse,DEB_REP_LOCALE"GAIN HPN=");
Lg+=uchartoa(CurGainHpN,&Reponse[Lg],0);
Lg+=StrCopyPgmToChar(&Reponse[Lg]," MIN=");
Lg+=uchartoa(CurGainMiN,&Reponse[Lg],0);
Lg+=StrCopyPgmToChar(&Reponse[Lg]," HPME=");
Lg+=uchartoa(CurGainHpMe,&Reponse[Lg],0);
Lg+=StrCopyPgmToChar(&Reponse[Lg]," MIME=");
Lg+=uchartoa(CurGainMiMe,&Reponse[Lg],0);
}




/*
void Traite_GRAF(unsigned char *cmd,unsigned char *numSMS)
Appellé a la réception d'une commande GRAF (Valeur des etapes grafcets)
GRAF G0=gr0 G1=gr1 G2=gr2 G3=gr3....
ENTREE : cmd = Commande recue
         num = Numero du SMS si commande par SMS ou NULL si port serie 
SORTIE : Variable globale Reponse
*/
void Traite_GRAF(unsigned char *cmd,unsigned char *numSMS)
{
unsigned char Lg;
unsigned char iTask;

Lg=StrCopyPgmToChar(Reponse,DEB_REP_LOCALE"GRAF");

for (iTask=1;iTask<NB_TASK;iTask++)
  {
  Lg+=StrCopyPgmToChar(&Reponse[Lg]," G");
  Lg+=uchartoa(iTask,&Reponse[Lg],0);
  Lg+=StrCopyPgmToChar(&Reponse[Lg],"=");
  Lg+=uchartoa(TASK[iTask].STEP,&Reponse[Lg],0);
  }

Lg+=StrCopyPgmToChar(&Reponse[Lg]," GC=");
Lg+=uchartoa(GrafChargeur,&Reponse[Lg],0);

Lg+=StrCopyPgmToChar(&Reponse[Lg]," DecC=");
Lg+=uchartoa(DeconnecterChargeur,&Reponse[Lg],0);

for (iTask=0;iTask<NB_E_TOR;iTask++)
  {
  Lg+=StrCopyPgmToChar(&Reponse[Lg]," 9G");
  Lg+=uchartoa(iTask,&Reponse[Lg],0);
  Lg+=StrCopyPgmToChar(&Reponse[Lg],"=");
  Lg+=uchartoa(T9_Gfe[iTask],&Reponse[Lg],0);
  }

Lg+=StrCopyPgmToChar(&Reponse[Lg]," AES=");
Lg+=uchartoa(AppelEntrant.Status,&Reponse[Lg],0);

Lg+=StrCopyPgmToChar(&Reponse[Lg]," ASS=");
Lg+=uchartoa(AppelSortant.Status,&Reponse[Lg],0);

#if 0
// a virer, trace longueur
DebutTraceDebugConsole();
StrPgmToConsole("grLG=");
uchartoa(Lg,Trace,3);
StrToConsole(Trace);
StrPgmToConsole("\n");
#endif

}



/*
void Traite_GUSINE(unsigned char *cmd,unsigned char *numSMS)
Appellé a la réception d'une commande GUSINE
ENTREE : cmd = Commande recue
         num = Numero du SMS si commande par SMS ou NULL si port serie 
SORTIE : Variable globale Reponse
*/
void Traite_GUSINE(unsigned char *cmd,unsigned char *numSMS)
{
unsigned char Lg;
unsigned char ii;
unsigned char nsms[2];
unsigned char Cmd[60];

nsms[0]=0;

Lg=StrCopyPgmToChar(Cmd,"CFPAU NOM=USINE DTMF=123 HTJ=24:00 DMC=10 DS=");
Lg+=uchartoa(CFPAU_DS_DEFAUT,&Cmd[Lg],0);

Traite_CFPAU(Cmd,nsms);
ClrWdt();

// V161216, Masque de changement de defaut pour envoi SMS, tous sauf niveau reception
//Lg=StrCopyPgmToChar(Cmd,"CFPAU MDSC=1023 WDSV=127 WDST=127 WDSG=127 WDVA=127"); // Avant V161216

Lg=StrCopyPgmToChar(Cmd,"CFPAU WDSV=127 WDST=127 WDSG=127 WDVA=127 MDSC=");
Lg+=uinttoa(MSQ_GUSINE_MDSC,&Cmd[Lg]);

Traite_CFPAU(Cmd,nsms);
ClrWdt();

for(ii=0;ii<NB_LIGNES_TAB_CFA;ii++)
  {
  Lg=StrCopyPgmToChar(Cmd,"CFA L=");
  Lg+=uchartoa(ii,&Cmd[Lg],0);
  Lg+=StrCopyPgmToChar(&Cmd[Lg]," EFFACE=1");
  Traite_CFA(Cmd,nsms);
  ClrWdt();
  }

Lg=StrCopyPgmToChar(Cmd,"GAIN HPN=30 MIN=9 HPME=45 MIME=5");
Traite_GAIN(Cmd,nsms);
ClrWdt();


// Sortie 1 voyant, clignote en appel, fixe en phonie
Lg=StrCopyPgmToChar(Cmd,"CFS S=1 F=2 C=1 I=0");
Traite_CFS(Cmd,nsms);

for(ii=2;ii<=NB_SORTIE;ii++)
  {
  Lg=StrCopyPgmToChar(Cmd,"CFS S=");
  Lg+=uchartoa(ii,&Cmd[Lg],0);
  Lg+=StrCopyPgmToChar(&Cmd[Lg]," F=0 C=0 I=0");
  Traite_CFS(Cmd,nsms);
  ClrWdt();
  }

// Options par defaut
// Reponse FAIL aux commandes erronnees par SMS, VANA sur chgt chargeur et apres tests
//OptionsSoft=(MSQ_OPTIONS_REPOND_FAIL|MSQ_REBOOT_TEST_ROAMING);
// Reponse fail supprime V170817
OptionsSoft=(MSQ_REBOOT_TEST_ROAMING);
WriteI2c(EEPROM, ADR_OPTIONS,1,&OptionsSoft,&CodeErreurI2c,&TimeOutI2c); // &OptionsSoft

// Duree d'un caractere DTMF
CurDurCarDTMF=DCD_DEFAUT;
WriteI2c(EEPROM, ADR_DC_DTMF,1,&CurDurCarDTMF,&CodeErreurI2c,&TimeOutI2c);

CurDurInterTrainDTMF_20ms=DIT_DEFAUT;
WriteI2c(EEPROM, ADR_INT_TRAIN,1,&CurDurInterTrainDTMF_20ms,&CodeErreurI2c,&TimeOutI2c);

// Parametres de l'anti echo
ii=DEFAUT_ECHO_MODEM;
WriteI2c(EEPROM, ADR_ECHO_MODEM,1,&ii,&CodeErreurI2c,&TimeOutI2c);

// Ecriture tolerance GPS
DeltaDeci=1500;
WriteI2c(EEPROM,ADR_GPS_TLA,4,(unsigned char*)DeltaDeci,&CodeErreurI2c,&TimeOutI2c); 
WriteI2c(EEPROM,ADR_GPS_TLO,4,(unsigned char*)DeltaDeci,&CodeErreurI2c,&TimeOutI2c); 
ReadI2c(EEPROM,ADR_GPS_TLO,4,(unsigned char*)TolLongitude,&CodeErreurI2c,&TimeOutI2c); // &TolLongitude 

WriteI2c(EEPROM, ADR_GPS_TP_ATTENTE_2S,1,(unsigned char *)PRESET_TEMPO_2S_MAINTIENT_GPS_DEFAUT,&CodeErreurI2c,&TimeOutI2c);
ReadI2c(EEPROM, ADR_GPS_TP_ATTENTE_2S,1,&ValTempoGPS_2Sec,&CodeErreurI2c,&TimeOutI2c); // &ValTempoGPS_2Sec
// Nombre de messages ignores
WriteI2c(EEPROM, ADR_GPS_NB_MS_IGNO,1,(unsigned char *)NB_MES_GPS_SUCC_PAU_STABLE_DEFAUT,&CodeErreurI2c,&TimeOutI2c);
ReadI2c(EEPROM, ADR_GPS_NB_MS_IGNO,1,&NbMesIgnoreesGPS,&CodeErreurI2c,&TimeOutI2c); // &NbMesIgnoreesGPS

// Positionne et relit l'etat service (!!! Pau hors service si (ValService==VAL_HORS_SERVICE))
WriteI2c(EEPROM, ADR_SERVICE,1,(unsigned char *)~VAL_HORS_SERVICE,&CodeErreurI2c,&TimeOutI2c);
ReadI2c(EEPROM, ADR_SERVICE,1,&ValService,&CodeErreurI2c,&TimeOutI2c); // &ValService


// Positionne et relit les seuils de charge batterie
WriteI2c(EEPROM,ADR_SEUIL_BAS_CHARGE,2,(unsigned char *)SEUIL_BAS_CHARGE_DEFAUT,&CodeErreurI2c,&TimeOutI2c);
ReadI2c(EEPROM,ADR_SEUIL_BAS_CHARGE,2,(unsigned char*)SeuilBasCharge,&CodeErreurI2c,&TimeOutI2c); // &SeuilBasCharge

WriteI2c(EEPROM,ADR_SEUIL_HAUT_CHARGE,2,(unsigned char *)SEUIL_HAUT_CHARGE_DEFAUT,&CodeErreurI2c,&TimeOutI2c);
ReadI2c(EEPROM,ADR_SEUIL_HAUT_CHARGE,2,(unsigned char*)SeuilHautCharge,&CodeErreurI2c,&TimeOutI2c); // &SeuilHautCharge

//WriteI2c(EEPROM,ADR_DELTA_PAN_BAT_CHARGE,2,DELTA_PAN_BAT_CHARGE_DEFAUT,&CodeErreurI2c,&TimeOutI2c);4:pas bon type de variable
ReadI2c(EEPROM,ADR_DELTA_PAN_BAT_CHARGE,2,(unsigned char*)DeltaPanBatCharge,&CodeErreurI2c,&TimeOutI2c);// &DeltaPanBatCharge4:pas bon type de variable

// Positionne maxi SMS auto autorise jour
WriteI2c(EEPROM, ADR_MAX_SMS_AUTO_JOUR,1, (unsigned char *)MAX_SMS_AUTO_JOUR_DEFAUT,&CodeErreurI2c,&TimeOutI2c);
ReadI2c(EEPROM, ADR_MAX_SMS_AUTO_JOUR,1,&MaxSmsJourAutoAutorise,&CodeErreurI2c,&TimeOutI2c); // &MaxSmsJourAutoAutorise

Lg=StrCopyPgmToChar(Reponse,DEB_REP_LOCALE"GUSINE");
}

#if 0
/*
void Traite_INFO(unsigned char *cmd,unsigned char *numSMS)
Appellé a la réception d'une commande INFO
ENTREE : cmd      = Commande recue
         num      = Numero du SMS si commande par SMS ou NULL si port serie 
SORTIE : Variable globale Reponse
*/
void Traite_INFO(unsigned char *cmd,unsigned char *numSMS)
{
unsigned char Lg;
unsigned char Tmp[1+1];

if(ValeurVariableLCR(cmd,"RDR=",Tmp,1))
  {
  if(Tmp[0]=='1')
    {
    ResetBitMotErreur(MSQEPAU_GSM_MT_RESEAU_GSM,1);
    }
  if(Tmp[0]=='2')
    {
    NbRebootTOModem=0;
    }
  }

LectureDateHeure(&DateCourante);
Lg=StrCopyPgmToChar(Reponse,DEB_REP_LOCALE"INFO HB=");
Lg+=EcritHeureDansBuffer(&DateBoot,&Reponse[Lg]);

Lg+=StrCopyPgmToChar(&Reponse[Lg]," HC=");
Lg+=EcritHeureDansBuffer(&DateCourante,&Reponse[Lg]);
                                                                // Attendre Nathan
Lg+=StrCopyPgmToChar(&Reponse[Lg]," HROK=");
Lg+=EcritHeureDansBuffer(&DateReseauOK,&Reponse[Lg]);

Lg+=StrCopyPgmToChar(&Reponse[Lg]," HRKO=");
Lg+=EcritHeureDansBuffer(&DateErreurReseau,&Reponse[Lg]);

Lg+=StrCopyPgmToChar(&Reponse[Lg]," ME=");
Lg+=uinttoa(StatusPau.MotErreurPAU,&Reponse[Lg]);

Lg+=StrCopyPgmToChar(&Reponse[Lg]," NTO=");
Lg+=uchartoa(NbRebootTOModem,&Reponse[Lg],0);

}
#endif 
/*
void Traite_LCONF(unsigned char *cmd,unsigned char *numSMS)
Appellé a la réception d'une commande LCONF ACT=a
LCONF CONF=c ST=s
ENTREE : cmd = Commande recue
         num = Numero du SMS si commande par SMS ou NULL si port serie 
SORTIE : Variable globale Reponse
*/
void Traite_LCONF(unsigned char *cmd,unsigned char *numSMS)
{
unsigned char Lg;
unsigned char Tmp[1+1];

if(ValeurVariableLCR(cmd,"ACT=",Tmp,1))
  {
  if((Tmp[0]=='1')||(Tmp[0]=='2'))
    {
    if(numSMS==NULL)
      {
      CtrlLConf.Console=(Tmp[0]-'0');
      }
    else
      {
      if(NumTelValide(numSMS))
        {
        if(CtrlLConf.NumSMS1[0]==0)
          {
          StrCopyToChar(CtrlLConf.NumSMS1,numSMS);
          CtrlLConf.ActSMS1=(Tmp[0]-'0');
          Silence=1; // On va renvoyer la config, inutile de repondre
          }
        else
          {
          if(CtrlLConf.NumSMS2[0]==0)
            {
            StrCopyToChar(CtrlLConf.NumSMS2,numSMS);
            CtrlLConf.ActSMS2=(Tmp[0]-'0');
            Silence=1; // On va renvoyer la config, inutile de repondre
            }
          }
        }
      }
    }
  }

// Mise en forme de la reponse
if(Silence==0)
  {
  Lg=StrCopyPgmToChar(Reponse,DEB_REP_LOCALE"LCONF CONS=");
  Lg+=uchartoa(CtrlLConf.Console,&Reponse[Lg],0);
  Lg+=StrCopyPgmToChar(&Reponse[Lg]," SMS1=");
  if(CtrlLConf.NumSMS1[0])
    Lg+=StrCopyToChar(&Reponse[Lg],CtrlLConf.NumSMS1);
  else
    Lg+=StrCopyPgmToChar(&Reponse[Lg],"Z");

  Lg+=StrCopyPgmToChar(&Reponse[Lg]," SMS2=");
  if(CtrlLConf.NumSMS2[0])
    Lg+=StrCopyToChar(&Reponse[Lg],CtrlLConf.NumSMS2);
  else
    Lg+=StrCopyPgmToChar(&Reponse[Lg],"Z");

  Lg+=StrCopyPgmToChar(&Reponse[Lg]," IDX=");
  Lg+=uchartoa(CtrlLConf.CurIdx,&Reponse[Lg],0);
  }
}



void Traite_ISDSTATUS(unsigned char *cmd,unsigned char *numSMS)
{
unsigned char Lg;
unsigned char Ret;


Lg=StrCopyPgmToChar(Reponse,DEB_REP_LOCALE"ISDSTATUS SRO=");
Ret=ISD_RD_STATUS();
Lg+=uchartoa(SR0_CC,&Reponse[Lg],0);
Lg+=StrCopyPgmToChar(&Reponse[Lg]," SR1=");
Lg+=uchartoa(SR1_CC,&Reponse[Lg],0);
}


/*
void Traite_MSGEFFACE(unsigned char *cmd,unsigned char *numSMS)
Appellé a la réception d'une commande MSGEFFACE
MSGEFFACE N=n N2=n
ENTREE : cmd = Commande recue
         num = Numero du SMS si commande par SMS ou NULL si port serie 
SORTIE : Variable globale Reponse
*/
void Traite_MSGEFFACE(unsigned char *cmd,unsigned char *numSMS)
{
unsigned char Lg;
unsigned char Ret;
unsigned char Tmp[3+1];


Lg=StrCopyPgmToChar(Reponse,DEB_REP_LOCALE"MSGEFFACE ");
if(ValeurVariableLCR(cmd,"TOUS=",Tmp,1))
  {
  if(Tmp[0]=='1')
    {
    if(SynthVoc_GetPowerState()==0)
      SynthVoc_PowerON (8);

    Ret=SynthVoc_GlobalErase();
    Lg+=StrCopyPgmToChar(&Reponse[Lg]," TOUS=1 RET=");
    Lg+=uchartoa(Ret,&Reponse[Lg],0);
    }
  }
}


/*
void Traite_MSGENREG(unsigned char *cmd,unsigned char *numSMS)
Appellé a la réception d'une commande MSGENREG
MSGENREG N=n ACT=a
ENTREE : cmd = Commande recue
         num = Numero du SMS si commande par SMS ou NULL si port serie 
SORTIE : Variable globale Reponse
*/
void Traite_MSGENREG(unsigned char *cmd,unsigned char *numSMS)
{
unsigned char Lg;
unsigned char Tmp[3+1];

if(SynthVoc_GetPowerState()==0)
  SynthVoc_PowerON (8);
PowerAudioAmp(AUDIO_ON);//allume l'ampli de puissance

Lg=StrCopyPgmToChar(Reponse,DEB_REP_LOCALE"MSGENREG ");
if(ValeurVariableLCR(cmd,"ACT=",Tmp,1))
  {
  if(Tmp[0]=='1')
    {
    SynthVoc_Rec();
    Lg+=StrCopyPgmToChar(&Reponse[Lg]," ACT=1");
    }
  else
    {
    SynthVoc_Stop();
    Lg+=StrCopyPgmToChar(&Reponse[Lg]," ACT=0");
    }
  }
}


/*
void Traite_MSGTEST(unsigned char *cmd,unsigned char *numSMS)
Appellé a la réception d'une commande MSGTEST
MSGTEST N=n ACT=a
ENTREE : cmd = Commande recue
         num = Numero du SMS si commande par SMS ou NULL si port serie 
SORTIE : Variable globale Reponse
*/
void Traite_MSGTEST(unsigned char *cmd,unsigned char *numSMS)
{
unsigned char Tmp[3+1];
unsigned char Act;
unsigned char Num;
unsigned char Lg;
unsigned char DevId;

if(SynthVoc_GetPowerState()==0)
  SynthVoc_PowerON (8);

Num=0xff;
if(ValeurVariableLCR(cmd,"ACT=",Tmp,1))
  {
  Act=strtouc(Tmp);
  if(ValeurVariableLCR(cmd,"NUM=",Tmp,3))
    {
    Num=strtouc(Tmp);
    }
  }
else
  {
  Act=0;
  }

DevId=SynthVoc_GetDeviceID();

Lg=StrCopyPgmToChar(Reponse,DEB_REP_LOCALE"MSGTEST DEV=");
Lg+=uchartoa(DevId,&Reponse[Lg],0);
Lg+=StrCopyPgmToChar(&Reponse[Lg]," ACT=");  

if((Act)&&(Num!=0xff))
  {
  Lg+=StrCopyPgmToChar(&Reponse[Lg],"1 NUM=");  
  Lg+=uchartoa(Num,&Reponse[Lg],0);
  PowerAudioAmp(AUDIO_ON);//allume l'ampli de puissance
  SetAudioChannel(1,00,80);//passe sur le canal audio 1
  SynthVoc_PlayMsg(Num);
  }  
else
  {  
  Lg+=StrCopyPgmToChar(&Reponse[Lg],"0 ");  
  PowerAudioAmp(AUDIO_OFF);//eteint l'ampli de puissance
  SynthVoc_PowerOff(); 
  }  
}


/*
void Traite_RACCROCHE(unsigned char *cmd,unsigned char *numSMS)
Appellé a la réception d'une commande RACCROCHE
ENTREE : cmd      = Commande recue
         num      = Numero du SMS si commande par SMS ou NULL si port serie 
SORTIE : Variable globale Reponse
*/
void Traite_RACCROCHE(unsigned char *cmd,unsigned char *numSMS)
{
unsigned char Tmp[1+1];
unsigned char Lg;


Lg=StrCopyPgmToChar(Reponse,DEB_REP_LOCALE"RACCROCHE ACT=");

if(ValeurVariableLCR(cmd,"ACT=",Tmp,1))
  {
  if(Tmp[0]=='1')
    {
    HangUp();
    }
  else
    {
    Tmp[0]='0';
    }
  }
else
  {
  Tmp[0]='0';
  }
Reponse[Lg]=Tmp[0];
Lg+=1;
Reponse[Lg]=0;
}


/*
void void Traite_REBOOT(unsigned char *cmd,unsigned char *numSMS)
Appellé a la réception d'une commande REBOOT
ENTREE : cmd      = Commande recue
         num      = Numero du SMS si commande par SMS ou NULL si port serie 
SORTIE : Variable globale Reponse
*/
void Traite_REBOOT(unsigned char *cmd,unsigned char *numSMS)
{
unsigned char Tmp[1+1];
unsigned char Lg;

if(ValeurVariableLCR(cmd,"ACT=",Tmp,1))
  {
  if(Tmp[0]=='M')
    RebootDemande=REBOOT_DEMANDE_MODEM; // reboot du modem
  if(Tmp[0]=='P')
    RebootDemande=REBOOT_DEMANDE_PAU; // reboot du PAU
  }
if(RebootDemande!=REBOOT_DEMANDE_AUCUN)
  StrCopyPgmToChar(Reponse,DEB_REP_LOCALE"REBOOT");
}

/*
void Traite_REPONDRE(unsigned char *cmd,unsigned char *numSMS)
Appellé a la réception d'une commande REPONDRE
ENTREE : cmd      = Commande recue
         num      = Numero du SMS si commande par SMS ou NULL si port serie 
SORTIE : Variable globale Reponse
*/
void Traite_REPONDRE(unsigned char *cmd,unsigned char *numSMS)
{
unsigned char Tmp[1+1];
unsigned char Lg;

if(ValeurVariableLCR(cmd,"ACT=",Tmp,1))
  {
  if((Tmp[0]=='1')&&(TASK[TASK5_ID].STEP==2))
    DecrocheLCR=1;
  else
    DecrocheLCR=0;

  Lg=StrCopyPgmToChar(Reponse,DEB_REP_LOCALE"REPONDRE ACT=");
  Lg+=uchartoa(DecrocheLCR,&Reponse[Lg],0);
  }
}


/*
void Traite_S(unsigned char *cmd)
Appellé a la réception d'une commande S d'emission d'un SMS
La commande S n'est autorisé que sur le port série
Le SMS est envoye au numero parametre par la commande SMS.
Si pas de numero parametre, on renvoi la config
*/
void Traite_S(unsigned char *cmd)
{
if(NumSMSCible[0])
  {
  // Saute la commande
  while((*cmd!=' ')&&(*cmd!=0)) cmd++;
  // Saute les espaces
  while( *cmd==' ') cmd++;
  if(*cmd)
    {
    // Envoi le SMS
    SMSEnvoi(NumSMSCible,cmd,0);
    }
  }
FabriqueReponseConfSMS(Reponse);
}


/*
void Traite_SMS(unsigned char *cmd)
Appellé a la réception d'une commande SMS 
La commande SMS n'est autorisé que sur le port série
SMS NUM=num tel pau distant ou Z pour effacer
*/
void Traite_SMS(unsigned char *cmd)
{
unsigned char Num[SZ_NUM_TEL+1];

if(ValeurVariableLCR(cmd,"NUM=",Num,SZ_NUM_TEL))
  {
  if(Num[0]=='Z')
    NumSMSCible[0]=0;
  else
    StrCopyToChar(NumSMSCible,Num);
  }
FabriqueReponseConfSMS(Reponse);
}


/*
void Traite_SIMULE(unsigned char *cmd,unsigned char *numSMS)
Appellé a la réception d'une commande SIMULE
La commande SMS n'est autorisé que sur le port série
SMS NUM=num tel pau distant ou Z pour effacer
*/
void Traite_SIMULE(unsigned char *cmd,unsigned char *numSMS)
{
unsigned char Tmp[2+1];
unsigned char Lg;

if(ValeurVariableLCR(cmd,"NE=",Tmp,2))
  {
  EntreeSimulee=Tmp[0]-'0';
  if((EntreeSimulee>0)&&(EntreeSimulee<=NB_E_TOR))
    {
    Lg=StrCopyPgmToChar(Reponse,DEB_REP_LOCALE"SIMULE NE=");
    Lg+=uchartoa(EntreeSimulee,&Reponse[Lg],0);
    }
  else
    {
    EntreeSimulee=0;
    }
  }
}


/*
void Traite_SLEEP(unsigned char *cmd,unsigned char *numSMS)
Appellé a la réception d'une commande SLEEP
ENTREE : cmd      = Commande recue
         num      = Numero du SMS si commande par SMS ou NULL si port serie 
SORTIE : Variable globale Reponse
*/
void Traite_SLEEP(unsigned char *cmd,unsigned char *numSMS)
{
unsigned char Tmp[10+1];
unsigned char Lg;

if(ValeurVariableLCR(cmd,"GSM=",Tmp,1))
  {
  if(Tmp[0]=='S')
    {
    //SetGsmMode(GSM_MODE_SLEEP);
    Lg=StrCopyPgmToChar(Reponse,DEB_REP_LOCALE"SLEEP GSM=S");
    }
  if(Tmp[0]=='N')
    {
    Lg=StrCopyPgmToChar(Reponse,DEB_REP_LOCALE"SLEEP GSM=N");
   SetGsmMode(GSM_MODE_NORMAL);
    }
  }

if(ValeurVariableLCR(cmd,"UC=",Tmp,1))
  {
  if(Tmp[0]=='S')
    {
    Sleep();
    }
  }
}


/*
void Traite_ST_PAUSTA(unsigned char *cmd,unsigned char *numSMS,char FrtReduit)
Appellé a la réception d'une commande ST ou PAUSTA
ENTREE : cmd       = Commande recue
         num       = Numero du SMS si commande par SMS ou NULL si port serie 
         FrtReduit = FormatReduit si appel depuis PAUSTA sinon appel depuis ST
SORTIE : Variable globale Reponse
*/
void Traite_ST_PAUSTA(unsigned char *cmd,unsigned char *numSMS,char FrtReduit)
{
unsigned char Tmp[5];

// Depuis V151217, possibilite depuis ST de changer le jour de la semaine
if(ValeurVariableLCR(cmd,"WD=",Tmp,1))
  {
  Tmp[0]-='0';
  if(Tmp[0]<7)
    {
    // Positionne WD et relit heure pour transmission correcte
    //EcritureDateHeure(NULL,Tmp[0]);                                 Nathan
    //LectureDateHeure(&DateCourante);                                Nathan
    }  
  }

if((numSMS==NULL)&&(ValeurVariableLCR(cmd,"SPC=",Tmp,3)))
  {
  StatusPau.SPC=strtouc(Tmp);
  if(StatusPau.SPC)
    {
    // SPC activé, on repasse en decroche manuel
    if(GetAnsweringMode()!=REPONSE_MANUELLE)
      SetAnsweringMode(REPONSE_MANUELLE);
    }
  else
    {
    // Perte du PC, on repasse en decroche automatique
    if(GetAnsweringMode()!=REPONSE_AUTO)
      SetAnsweringMode(REPONSE_AUTO);
    }
  }

if(ValeurVariableLCR(cmd,"RDP=",Tmp,3))
  {
  // Commande pour fair tomber le defaut phonie
  if(Tmp[0]=='1')
    {
    ResetBitMotErreur(MSQEPAU_GSM_MT_PHONIE,1);
    }
  }

if(ValeurVariableLCR(cmd,"RDA=",Tmp,3))
  {
  // Commande pour fair tomber le defaut appel non abouti
  if(Tmp[0]=='1')
    {
    ResetBitMotErreur(MSQEPAU_GSM_MT_APPEL_NON_ABOUTI,1);
    }
  }

if(ValeurVariableLCR(cmd,"SRV=",Tmp,3))
  {
  // Modifie memoire uniquement si changement
  if((Tmp[0]=='1')&&(ValService==VAL_HORS_SERVICE))
    {
//    WriteI2c(EEPROM, ADR_SERVICE,1,~VAL_HORS_SERVICE,&CodeErreurI2c,&TimeOutI2c);4:pas bon type de variable
    }
  else
    {
    if((Tmp[0]=='0')&&(ValService!=VAL_HORS_SERVICE))
      {
      //WriteI2c(EEPROM, ADR_SERVICE,1,VAL_HORS_SERVICE,&CodeErreurI2c,&TimeOutI2c);4:pas bon type de variable
      }    
    }
  // Recupere l'etat service ou non (!!! Pau hors service si (ValService==VAL_HORS_SERVICE))
  ReadI2c(EEPROM, ADR_SERVICE,1,&ValService,&CodeErreurI2c,&TimeOutI2c); // &ValService
  }

if(ValeurVariableLCR(cmd,"OPE=",Tmp,3))
  {
  // Modifie memoire uniquement si changement
  if(ValOperationnel!=(Tmp[0]-'0'))
    {
    ValOperationnel=Tmp[0]-'0';
    WriteI2c(EEPROM, ADR_OPERATIONNEL,1,&ValOperationnel,&CodeErreurI2c,&TimeOutI2c);
    }
  // Recupere l'etat service ou non (!!! Pau hors service si (ValService==VAL_HORS_SERVICE))
  ReadI2c(EEPROM, ADR_OPERATIONNEL,1,&ValOperationnel,&CodeErreurI2c,&TimeOutI2c);  // &ValOperationnel
  }

FabriqueReponseStatus(Reponse,StatusPau.MotErreurPAU,FrtReduit,SZ_CMD_REPONSE+1);
}



/*
void Traite_SMSFAIL(unsigned char *cmd,unsigned char *numSMS)
Appellé a la réception d'une commande SMSFAIL
ENTREE : cmd      = Commande recue
         num      = Numero du SMS si commande par SMS ou NULL si port serie 
SORTIE : Variable globale Reponse
*/
void Traite_SMSFAIL(unsigned char *cmd,unsigned char *numSMS)
{
unsigned char Tmp[3+1];
unsigned char Lg;


if(ValeurVariableLCR(cmd,"AUTR=",Tmp,1))
  {
  if(Tmp[0]=='1')
    {
    PositionneBitOptionsSoft(MSQ_OPTIONS_REPOND_FAIL,1);
    }
  else
    {
    PositionneBitOptionsSoft(MSQ_OPTIONS_REPOND_FAIL,0);
    }
  }

Lg=StrCopyPgmToChar(Reponse,DEB_REP_LOCALE"SMSFAIL NB=");
Lg+=uchartoa(RepFailSMS.NbRep,&Reponse[Lg],1);

Lg+=StrCopyPgmToChar(&Reponse[Lg]," SMS=");
if(RepFailSMS.NumSMS[0])
  Lg+=StrCopyToChar(&Reponse[Lg],RepFailSMS.NumSMS);
else
  Lg+=StrCopyPgmToChar(&Reponse[Lg],"Z");

Lg+=StrCopyPgmToChar(&Reponse[Lg]," HR=");
//Lg+=EcritHeureDansBuffer(&RepFailSMS.DtCmd,&Reponse[Lg]); Nathan

Lg+=StrCopyPgmToChar(&Reponse[Lg]," AUTR=");
if((OptionsSoft&MSQ_OPTIONS_REPOND_FAIL)==MSQ_OPTIONS_REPOND_FAIL)
  Lg+=StrCopyPgmToChar(&Reponse[Lg],"1");   
else
  Lg+=StrCopyPgmToChar(&Reponse[Lg],"0");

if(ValeurVariableLCR(cmd,"RAZ=",Tmp,1))
  {
  if(Tmp[0]=='1')
    {
    RepFailSMS.NbRep=0;
    Lg+=StrCopyPgmToChar(&Reponse[Lg]," RAZ=1");
    }
  }
}



/*
void Traite_TESTAUDIO(unsigned char *cmd,unsigned char *numSMS)
Appellé a la réception d'une commande TESTAUDIO
ENTREE : cmd      = Commande recue
         num      = Numero du SMS si commande par SMS ou NULL si port serie 
SORTIE : Variable globale Reponse
*/
void Traite_TESTAUDIO(unsigned char *cmd,unsigned char *numSMS)
{
unsigned char Tmp[3+1];
unsigned char Lg;

if(ValeurVariableLCR(cmd,"ACT=",Tmp,1))
  {
  if((Tmp[0]=='1')||(Tmp[0]=='2'))
    {
    TestDemLCR=(Tmp[0]-'0');
    if(FindFirstSTRInBuff (cmd," T=1", &Lg))
      TestDemLCR|=MSQ_TESTLCR_COMPLET;
    if(FindFirstSTRInBuff (cmd," PE=1", &Lg))
      TestDemLCR|=MSQ_TESTLCR_PRINT_E5;
    if(FindFirstSTRInBuff (cmd," MONO=1", &Lg))
      TestDemLCR|=MSQ_MONO_FREQ;
    }
  else
    {
    TestDemLCR=0;
    }
  
  
  // Memorise numero pour retour etat pau
  if((numSMS!=NULL)&&(CtrlTestSms.NumSMS[0]==0)  )
    {
    StrCopyToChar(CtrlTestSms.NumSMS,numSMS);
    CtrlTestSms.FormatCourt=0;
    }
  }

if(TestDemLCR)
  {
  Reponse[0]=0;
  Silence=1; // On va repondre apres le test, inutile de repondre
  }
else
  {
  Lg=StrCopyPgmToChar(Reponse,DEB_REP_LOCALE"TESTAUDIO ACT=");
  Lg+=uchartoa(TestDemLCR,&Reponse[Lg],1);
  }
}



/*
void Traite_PAUTST(unsigned char *cmd,unsigned char *numSMS)
Appellé a la réception d'une commande PAUTST
ENTREE : cmd      = Commande recue
         num      = Numero du SMS si commande par SMS ou NULL si port serie 
SORTIE : Variable globale Reponse
*/
void Traite_PAUTST(unsigned char *cmd,unsigned char *numSMS)
{
TestDemLCR=MSQ_TESTLCR_AVEC_TRANS;  // Demande de test et transmission de resultat

// Positionne flag silence pour eviter reponse fail
Silence=1;

// Memorise numero pour retour etat pau
if((numSMS!=NULL)&&(CtrlTestSms.NumSMS[0]==0)  )
  {
  StrCopyToChar(CtrlTestSms.NumSMS,numSMS);
  CtrlTestSms.FormatCourt=1;
  }
}




/*
void Traite_VANA(unsigned char *cmd,unsigned char *numSMS)
Appellé a la réception d'une commande VANA 
ENTREE : cmd = Commande recue
         num = Numero du SMS si commande par SMS ou NULL si port serie 
SORTIE : Variable globale Reponse
*/
void Traite_VANA(unsigned char *cmd,unsigned char *numSMS)
{
unsigned char Lg;
unsigned char Tmp[5+1];
uint16 ValTmp;


if(ValeurVariableLCR(cmd,"DEC=",Tmp,1))
  {
  if(Tmp[0]=='1')
    {
    DeconnecterChargeur=1;
    if((numSMS!=NULL)&&(*numSMS))
      {
      // Memorise num SMS qui sera emis apres mesure panneau
      StrCopyToChar(NumSmsVANA,numSMS);
      // Empeche envoi SMS
      *numSMS=0;
      // Empeche carte de dormir pour faire les mesures
      if(CptPresPC<10) 
        CptPresPC=10;
      }
    }
  }

if(ValeurVariableLCR(cmd,"FORCE=",Tmp,3))
  {
  ForcerChargeurSec=strtouc(Tmp);
  if(ForcerChargeurSec)
    DeconnecterChargeur=0;
  }

if(ValeurVariableLCR(cmd,"SBC=",Tmp,5))
  {
  ValTmp=strtoui(Tmp);
  if((ValTmp!=SeuilBasCharge)&&(ValTmp<=SEUIL_BAS_MAX_CHARGE))
    {
    WriteI2c(EEPROM,ADR_SEUIL_BAS_CHARGE,2,(unsigned char*)ValTmp,&CodeErreurI2c,&TimeOutI2c);
    ReadI2c(EEPROM,ADR_SEUIL_BAS_CHARGE,2,(unsigned char*)SeuilBasCharge,&CodeErreurI2c,&TimeOutI2c); // &SeuilBasCharge
    }
  }

if(ValeurVariableLCR(cmd,"SHC=",Tmp,5))
  {
  ValTmp=strtoui(Tmp);
  if((ValTmp!=SeuilHautCharge)&&(ValTmp<=SEUIL_HAUT_MAX_CHARGE))
    {
    WriteI2c(EEPROM,ADR_SEUIL_HAUT_CHARGE,2,(unsigned char*)ValTmp,&CodeErreurI2c,&TimeOutI2c); 
    ReadI2c(EEPROM,ADR_SEUIL_HAUT_CHARGE,2,(unsigned char*)SeuilHautCharge,&CodeErreurI2c,&TimeOutI2c); // &SeuilHautCharge 
    }
  }

if(ValeurVariableLCR(cmd,"DPBC=",Tmp,5))
  {
  ValTmp=strtoui(Tmp);
  if((ValTmp!=DeltaPanBatCharge)&&(ValTmp<=DELTA_MAX_CHARGE))
    {
    WriteI2c(EEPROM,ADR_DELTA_PAN_BAT_CHARGE,2,(unsigned char*)ValTmp,&CodeErreurI2c,&TimeOutI2c);
    ReadI2c(EEPROM,ADR_DELTA_PAN_BAT_CHARGE,2,(unsigned char*)DeltaPanBatCharge,&CodeErreurI2c,&TimeOutI2c);
    }
  }
FabriqueReponseVANA(Reponse);
}



#if 0 

/*
void Traite_GPSWRITE(unsigned char *cmd,unsigned char *numSMS)
Appellé a la réception d'une commande GPSWRITE
ENTREE : cmd = Commande recue
         num = Numero du SMS si commande par SMS ou NULL si port serie 
SORTIE : Variable globale Reponse
 * Envoi une chaine sur le port serie du GPS
*/
void Traite_GPSWRITE(unsigned char *cmd,unsigned char *numSMS)
{
unsigned char Lg;

// Saute la commande
while((*cmd) && (*cmd!=' ')) cmd++;
// Saute les espaces
while(*cmd==' ') cmd++;

if(*cmd)
  {
  EcritGPS(*cmd);
  }
Lg=StrCopyPgmToChar(Reponse,DEB_REP_LOCALE"GPSWRITE");
}
#endif


/*
void Traite_GPSREF(unsigned char *cmd,unsigned char *numSMS)
Appellé a la réception d'une commande GPSREF
ENTREE : cmd = Commande recue
         num = Numero du SMS si commande par SMS ou NULL si port serie 
SORTIE : Variable globale Reponse
GPSREF ACT=1 demande de mettre la position correcte en position de reference
*/
void Traite_GPSREF(unsigned char *cmd,unsigned char *numSMS)
{
unsigned char Tmp[3];
unsigned char Lg;

if(ValeurVariableLCR(cmd,"ACT=",Tmp,3))
  {
  if(Tmp[0]=='1')
    {
    CurVersRef=1;
    if(DemandeAcquisitionGPS(150))
      {
      if(numSMS!=NULL)
        {
        if(NumSMS1_GPS[0]==0)
          {
          StrCopyToChar(NumSMS1_GPS,numSMS);
          RepSMS1_GPSCourt=1;
          }
        else
          {
          StrCopyToChar(NumSMS2_GPS,numSMS);
          RepSMS2_GPSCourt=1;
          }
        // gerer si 2 sms deja en attente
        }
      numSMS[0]=0; // evite reponse immediate
      }
    }
  }
Lg=StrCopyPgmToChar(Reponse,DEB_REP_LOCALE"GPSREF ACT=");
Lg+=uchartoa(CurVersRef,&Reponse[Lg],0);

//FINIR Memoriser le numero SMS et repondre R GPSPOS
/*
 * 
 * 
        if(NumSMS1_GPS[0]==0)
          StrCopyToChar(NumSMS1_GPS,numSMS);
        else
          StrCopyToChar(NumSMS2_GPS,numSMS);
*/
}




/*
void Traite_GPS(unsigned char *cmd,unsigned char *numSMS)
Appellé a la réception d'une commande GPS
ENTREE : cmd = Commande recue
         num = Numero du SMS si commande par SMS ou NULL si port serie 
SORTIE : Variable globale Reponse
*/
void Traite_GPS(unsigned char *cmd,unsigned char *numSMS)
{
unsigned char Tmp[12];
unsigned char Lg;
unsigned long ValLong;
unsigned char MemOptionsSoft;

if(ValeurVariableLCR(cmd,"ACT=",Tmp,3))
  {
  MemOptionsSoft=OptionsSoft;
  if(Tmp[0]!='0')
    {
    // Remise en service du gps
    OptionsSoft|=MSQ_OPTIONS_GPS_ACTIF;
    if(Tmp[0]=='3')
      OptionsSoft|=MSQ_OPTIONS_GPS_ALIM_PERM;
    else
      OptionsSoft&=~MSQ_OPTIONS_GPS_ALIM_PERM;
    }
  else
    {
    // Arret du GPS et de l'alim permanente
    OptionsSoft&=~(MSQ_OPTIONS_GPS_ACTIF|MSQ_OPTIONS_GPS_ALIM_PERM);
    }

  if(MemOptionsSoft!=OptionsSoft)
    {
    WriteI2c(EEPROM, ADR_OPTIONS,1,&OptionsSoft,&CodeErreurI2c,&TimeOutI2c);  // &OptionsSoft
    // Recupere les options
    ReadI2c(EEPROM, ADR_OPTIONS,1,&OptionsSoft,&CodeErreurI2c,&TimeOutI2c);

    if((OptionsSoft&MSQ_OPTIONS_GPS_ACTIF)==MSQ_OPTIONS_GPS_ACTIF)
      {
      // Reset bit GPS inactif
      MotEtatGPS&=~MSQ_ETAT_GPS_INACTIF;
      }
    else
      {
      // Set bit GPS inactif
      MotEtatGPS|=MSQ_ETAT_GPS_INACTIF;
      }
    }
  }

if(ValeurVariableLCR(cmd,"REF=",Tmp,3))
  {
  if(Tmp[0]=='1')
    {
    CurVersRef=1;
    }
  if(Tmp[0]=='B')
    {
    CurVersRef=2;   // Memorisera mauvaise position pour forcer erreur deplacement
    }
  }

if(ValeurVariableLCR(cmd,"TP=",Tmp,3))
  {
  Lg=strtouc(Tmp);
  if(Lg!=ValTempoGPS_2Sec)
    {
    WriteI2c(EEPROM, ADR_GPS_TP_ATTENTE_2S,1,&Lg,&CodeErreurI2c,&TimeOutI2c);
    ReadI2c(EEPROM, ADR_GPS_TP_ATTENTE_2S,1,&ValTempoGPS_2Sec,&CodeErreurI2c,&TimeOutI2c); // &ValTempoGPS_2Sec
    }
  }

if(ValeurVariableLCR(cmd,"NMI=",Tmp,3))
  {
  Lg=strtouc(Tmp);
  if(Lg!=NbMesIgnoreesGPS)
    {
    WriteI2c(EEPROM, ADR_GPS_NB_MS_IGNO,1,&Lg,&CodeErreurI2c,&TimeOutI2c);
    ReadI2c(EEPROM, ADR_GPS_NB_MS_IGNO,1,&NbMesIgnoreesGPS,&CodeErreurI2c,&TimeOutI2c); // &NbMesIgnoreesGPS
    }
  }

if(ValeurVariableLCR(cmd,"ACQ=",Tmp,3))
  {
  if(Tmp[0]=='1')
    {
    if(DemandeAcquisitionGPS(151))
      {
      if(numSMS!=NULL)
        {
        if(NumSMS1_GPS[0]==0)
          {
          StrCopyToChar(NumSMS1_GPS,numSMS);
          RepSMS1_GPSCourt=0;
          }
        else
          {
          StrCopyToChar(NumSMS2_GPS,numSMS);
          RepSMS2_GPSCourt=0;
          }
        // gerer si 2 sms deja en attente
        }
      numSMS[0]=0; // evite reponse immediate
      }
    }
  }


if(ValeurVariableLCR(cmd,"TLA=",Tmp,11))
  {
  if(ExtraitLongChaine(Tmp,&ValLong,NB_CAR_DECI_GPS+2,0))
    {
    if(ValLong!=TolLatitude)
      {
          WriteI2c(EEPROM,ADR_GPS_TLA,4,(unsigned char*)ValLong,&CodeErreurI2c,&TimeOutI2c);
          ReadI2c(EEPROM,ADR_GPS_TLA,4,(unsigned char*)TolLatitude,&CodeErreurI2c,&TimeOutI2c);
      }
    }
  }

if(ValeurVariableLCR(cmd,"TLO=",Tmp,11))
  {
  if(ExtraitLongChaine(Tmp,&ValLong,NB_CAR_DECI_GPS+2,0))
    {
    if(ValLong!=TolLongitude)
      {
      WriteI2c(EEPROM,ADR_GPS_TLO,4,(unsigned char*)ValLong,&CodeErreurI2c,&TimeOutI2c); 
      ReadI2c(EEPROM,ADR_GPS_TLO,4,(unsigned char*)TolLongitude,&CodeErreurI2c,&TimeOutI2c); // &TolLongitude 
      }
    }
  }


if(ValeurVariableLCR(cmd,"TRACE=",Tmp,3))
  {
  if(Tmp[0]=='1')
    {
    PositionneBitOptionsSoft(MSQ_OPTIONS_TRACE_GPS,1);
    }
  else
    {
    PositionneBitOptionsSoft(MSQ_OPTIONS_TRACE_GPS,0);
    }
  }

Lg=StrCopyPgmToChar(Reponse,DEB_REP_LOCALE"GPS ACT=");
if(OptionsSoft&MSQ_OPTIONS_GPS_ACTIF)
  {
  if(OptionsSoft&MSQ_OPTIONS_GPS_ALIM_PERM)
    {
    Lg+=StrCopyPgmToChar(&Reponse[Lg],"3");
    }
  else
    {
    Lg+=StrCopyPgmToChar(&Reponse[Lg],"1");
    }
  }
else
  {
  Lg+=StrCopyPgmToChar(&Reponse[Lg],"0");
  }

Lg+=StrCopyPgmToChar(&Reponse[Lg]," ME=");
Lg+=uchartoa(MotEtatGPS,&Reponse[Lg],0);

Lg+=StrCopyPgmToChar(&Reponse[Lg]," TP=");
Lg+=uchartoa(ValTempoGPS_2Sec,&Reponse[Lg],0);

Lg+=StrCopyPgmToChar(&Reponse[Lg]," NMI=");
Lg+=uchartoa(NbMesIgnoreesGPS,&Reponse[Lg],0);

// Ecrit la tolerance
Lg+=StrCopyPgmToChar(&Reponse[Lg]," TLA=");
Lg+=ulongtoa(TolLatitude,&Reponse[Lg],12);
Lg+=StrCopyPgmToChar(&Reponse[Lg]," TLO=");
Lg+=ulongtoa(TolLongitude,&Reponse[Lg],12);

if(OptionsSoft&MSQ_OPTIONS_GPS_ACTIF)
  {
  // Ecrit la latitude de reference
  Lg+=StrCopyPgmToChar(&Reponse[Lg]," RLA=");
  Lg+=ulongtoa(RefLatitude,&Reponse[Lg],12);
  Lg+=StrCopyPgmToChar(&Reponse[Lg],".");
  Lg+=ulongtoa(RefDeci_Latitude,&Reponse[Lg],12);
  Reponse[Lg]=RefCarLatitude;
  Lg+=1;

  // Ecrit la latitude courante
  if(MotEtatGPS&MSQ_ETAT_GPS_AUCUNE_POS_MEM)  //en cours de demarrage, position incorrecte
    {
    // Aucune position GPS 
    Lg+=StrCopyPgmToChar(&Reponse[Lg]," CLA=Z");
    }
  else
    {
    Lg+=StrCopyPgmToChar(&Reponse[Lg]," CLA=");
    Lg+=ulongtoa(Dbp_Latitude,&Reponse[Lg],12);
    Lg+=StrCopyPgmToChar(&Reponse[Lg],".");
    Lg+=ulongtoa(Dbp_Deci_Latitude,&Reponse[Lg],12);
    Reponse[Lg]=Dbp_CarLatitude;
    Lg+=1;
    }

  // Ecrit la longitude de reference
  Lg+=StrCopyPgmToChar(&Reponse[Lg]," RLO=");
  Lg+=ulongtoa(RefLongitude,&Reponse[Lg],12);
  Lg+=StrCopyPgmToChar(&Reponse[Lg],".");
  Lg+=ulongtoa(RefDeci_Longitude,&Reponse[Lg],12);
  Reponse[Lg]=RefCarLongitude;
  Lg+=1;

  // Ecrit la longitude courante
  if(MotEtatGPS&MSQ_ETAT_GPS_AUCUNE_POS_MEM) //en cours de demarrage, position incorrecte
    {
    // Aucune position GPS 
    Lg+=StrCopyPgmToChar(&Reponse[Lg]," CLO=Z");
    }
  else
    {
    Lg+=StrCopyPgmToChar(&Reponse[Lg]," CLO=");
    Lg+=ulongtoa(Dbp_Longitude,&Reponse[Lg],12);
    Lg+=StrCopyPgmToChar(&Reponse[Lg],".");
    Lg+=ulongtoa(Dbp_Deci_Longitude,&Reponse[Lg],12);
    Reponse[Lg]=Dbp_CarLongitude;
    Lg+=1;
    }

  if(MotEtatGPS&MSQ_ETAT_GPS_AUCUNE_POS_MEM)
    {
    // Aucune position memorisee
    Lg+=StrCopyPgmToChar(&Reponse[Lg]," DLA=Z DLO=Z DA=Z");
    }
  else
    {
    // Ecrit delta latitude
    if(Lg<(SZ_DATA_SMS-16))
      {  
      DeltaDeci=DeltaLatLongTropGrand((Dbp_CarLatitude==RefCarLatitude),Dbp_Latitude,RefLatitude,Dbp_Deci_Latitude,RefDeci_Latitude);
      Lg+=StrCopyPgmToChar(&Reponse[Lg]," DLA=");
      Lg+=ulongtoa(DeltaDeci,&Reponse[Lg],12);
      }

    // Ecrit delta longitude
    if(Lg<(SZ_DATA_SMS-16))
      {
      DeltaDeci=DeltaLatLongTropGrand((Dbp_CarLongitude==RefCarLongitude),Dbp_Longitude,RefLongitude,Dbp_Deci_Longitude,RefDeci_Longitude);
      Lg+=StrCopyPgmToChar(&Reponse[Lg]," DLO=");
      Lg+=ulongtoa(DeltaDeci,&Reponse[Lg],12);
      }
    
    // Nb de satellite
    if(Lg<(SZ_DATA_SMS-16))
      {
      Lg+=StrCopyPgmToChar(&Reponse[Lg]," NS=");
      Lg+=uchartoa(Dbp_NbSat,&Reponse[Lg],0);
      }

    // Altitude
    if(Lg<(SZ_DATA_SMS-16))
      {
      Lg+=StrCopyPgmToChar(&Reponse[Lg]," AL=");
      Lg+=uinttoa(Dbp_Altitude,&Reponse[Lg]);
      }
    
    // Ecrite date acquisition s'il reste de la place
    if(Lg<(SZ_DATA_SMS-16))
      {
      Lg+=StrCopyPgmToChar(&Reponse[Lg]," DA=");
  //    Lg+=uchartoa(DbpGPS_DateAcq.An     ,&Reponse[Lg],2);
  //    Lg+=uchartoa(DbpGPS_DateAcq.Mois   ,&Reponse[Lg],2);   Nathan
  //    Lg+=uchartoa(DbpGPS_DateAcq.Jour   ,&Reponse[Lg],2);
  //    Lg+=uchartoa(DbpGPS_DateAcq.Heure  ,&Reponse[Lg],2);
  //    Lg+=uchartoa(DbpGPS_DateAcq.Minute ,&Reponse[Lg],2);
  //    Lg+=uchartoa(DbpGPS_DateAcq.Seconde,&Reponse[Lg],2);
      }
    }
  }
Reponse[Lg]=0;
}



/*
void Traite_GPSLOC(unsigned char *cmd,unsigned char *numSMS)
Appellé a la réception d'une commande GPSLOC (Demande de localisation)
ENTREE : cmd = Commande recue
         num = Numero du SMS si commande par SMS ou NULL si port serie 
SORTIE : Variable globale Reponse
*/
void Traite_GPSLOC(unsigned char *cmd,unsigned char *numSMS)
{
unsigned char Lg;

// Reset bit GPS inactif
//    MotEtatGPS&=~MSQ_ETAT_GPS_INACTIF;

Lg=StrCopyPgmToChar(Reponse,DEB_REP_LOCALE"GPSPOS LAT=");
// Ecrit la latitude courante
if(MotEtatGPS&MSQ_ETAT_GPS_AUCUNE_POS_MEM)  //en cours de demarrage, position incorrecte
  {
  // Aucune position GPS 
  Lg+=StrCopyPgmToChar(&Reponse[Lg],"Z LON=Z");
  }
else
  {
  // Latitude courante
  Lg+=ulongtoa(Dbp_Latitude,&Reponse[Lg],12);
  Lg+=StrCopyPgmToChar(&Reponse[Lg],".");
  Lg+=ulongtoa(Dbp_Deci_Latitude,&Reponse[Lg],12);
  Reponse[Lg]=Dbp_CarLatitude;
  Lg+=1;

  Lg+=StrCopyPgmToChar(&Reponse[Lg]," LON=");
  Lg+=ulongtoa(Dbp_Longitude,&Reponse[Lg],12);
  Lg+=StrCopyPgmToChar(&Reponse[Lg],".");
  Lg+=ulongtoa(Dbp_Deci_Longitude,&Reponse[Lg],12);
  Reponse[Lg]=Dbp_CarLongitude;
  Lg+=1;
  }
Lg+=StrCopyPgmToChar(&Reponse[Lg]," ET=");
if(MotEtatGPS&(MSQ_ETAT_GPS_TO|MSQ_ETAT_GPS_INACTIF))
  Reponse[Lg]='1';        // ET=0 si timeout ou inactif
else
  Reponse[Lg]='0';
Lg+=1;
Lg+=StrCopyPgmToChar(&Reponse[Lg]," STGPS=");
Lg+=uchartoa(MotEtatGPS,&Reponse[Lg],0);

Reponse[Lg]=0;
}



void Traite_VERIFNUMTELPAU(unsigned char *cmd,unsigned char *numSMS,unsigned char *DateSMS,unsigned char *HeureSMS)
{
unsigned char NumTelRecu[SZ_NUM_TEL+1];
unsigned char CuRecu[SZ_CU_VERIFNUM+1];
unsigned char TelMem[SZ_NUM_TEL+1];
unsigned char CmdDT[25];
unsigned char Lg;

if(ValeurVariableLCR(cmd,"NUM=",NumTelRecu,SZ_NUM_TEL))
  {
  if(ValeurVariableLCR(cmd,"CU=",CuRecu,SZ_CU_VERIFNUM))
    {
    if(!strcmp((char*)ChUniqueVerifNum,(char*)CuRecu))
      {
      memset(TelMem,0,SZ_NUM_TEL);
      ReadI2c(EEPROM, ADR_TEL_PAU,SZ_NUM_TEL,TelMem,&CodeErreurI2c,&TimeOutI2c);
      TelMem[SZ_NUM_TEL]=0;
      if(FinNumTelIdentique(TelMem,NumTelRecu)>5)
        {
        // Verification ok, debloque grafcet 7
        if(VerifNumPau==EVNP_ATTEND_SMS)
          {
          VerifNumPau=EVNP_SMS_OK;
          // Appelle la fonction DT avec l'horodate du SMS
          if((numSMS!=NULL)&&(DateSMS!=NULL)&&(HeureSMS!=NULL))
            {
            Lg=StrCopyPgmToChar(CmdDT,"DT ");
            Lg+=StrCopyToChar(&CmdDT[Lg],DateSMS);
            Lg+=StrCopyPgmToChar(&CmdDT[Lg]," ");
            Lg+=StrCopyToChar(&CmdDT[Lg],HeureSMS);
            Traite_DT(CmdDT,NULL,1);
            }
          // Positionne flag silence pour eviter reponse a nous meme
          Silence=1;
          }
        }
      }
	}
  }
else
  {
  if(ValeurVariableLCR(cmd,"GSM=",NumTelRecu,SZ_NUM_TEL))
    {
    if(NumTelRecu[0]=='1')
      {
      VerifNumPau=EVNP_GSM_DEMANDEE;
      }
	}

  // Commande sans parametre, demande d'envoi d'un SMS a lui meme
  if(VerifNumPau==EVNP_AUCUNE)
    {
    VerifNumPau=EVNP_NUM_DEMANDEE;
    }
  }
Lg=StrCopyPgmToChar(Reponse,DEB_REP_LOCALE"VERIFNUMTELPAU VNP=");
Lg+=uchartoa(VerifNumPau,&Reponse[Lg],0);
}



/*
void Traite_VERSION(unsigned char *cmd,unsigned char *numSMS)
Appellé a la réception d'une commande Traite_V
ENTREE : cmd = Commande recue
         num = Numero du SMS si commande par SMS ou NULL si port serie 
SORTIE : Variable globale Reponse
*/
void Traite_VERSION(unsigned char *cmd,unsigned char *numSMS)
{
unsigned char ToutFF;
unsigned char Lg;
unsigned char ii;
unsigned char SNumSerie[SZ_NUM_SERIE+1];
unsigned char SaisieNumSerie[(SZ_NUM_SERIE*3)+1];
unsigned char Vu[2];


Lg=StrCopyPgmToChar(Reponse,DEB_REP_LOCALE"VERSION GEN=");
Lg+=StrCopyPgmToChar(&Reponse[Lg],PRODUCT_NAME);
Lg+=StrCopyPgmToChar(&Reponse[Lg]," UC=");
Lg+=StrCopyPgmToChar(&Reponse[Lg],SOFTWARE_VERSION);

ReadI2c(EEPROM, ADR_NUM_SERIE,SZ_NUM_SERIE,SNumSerie,&CodeErreurI2c,&TimeOutI2c);
Lg+=StrCopyPgmToChar(&Reponse[Lg]," NS=");
if(SNumSerie[0]==0xff)
  ToutFF=1;
else
  ToutFF=0;
Lg+=uchartoHexa(SNumSerie[0],&Reponse[Lg]);
for(ii=1;ii<SZ_NUM_SERIE;ii++)
  {
  Lg+=StrCopyPgmToChar(&Reponse[Lg],":");
  Lg+=uchartoHexa(SNumSerie[ii],&Reponse[Lg]);
  if(SNumSerie[ii]!=0xff)
    ToutFF=0;
  }

if(ValeurVariableLCR(cmd,"VU=",Vu,1))
  {
  Vu[1]=0;
  // Met vu à '0' si hors plage ou numero de serie non renseigné
  if((Vu[0]<'0')||(Vu[0]>'9')||(ToutFF==1))
    Vu[0]='0';
  WriteI2c(EEPROM,ADR_VALID_U,1,Vu,&CodeErreurI2c,&TimeOutI2c);
  }

// Lit le champs de validité et le positionne à 0 si hors plage
Lg+=StrCopyPgmToChar(&Reponse[Lg]," VU=");
ReadI2c(EEPROM, ADR_VALID_U,1,Vu,&CodeErreurI2c,&TimeOutI2c);
Vu[1]=0;
if((Vu[0]<'0')||(Vu[0]>'9')||((Vu[0]!='0')&&(ToutFF==1)))
  {
  Vu[0]='0';
  Vu[1]=0;
  WriteI2c(EEPROM,ADR_VALID_U,1,Vu,&CodeErreurI2c,&TimeOutI2c);
  }
Lg+=StrCopyToChar(&Reponse[Lg],Vu);

if(Gsm_CCID[0])
  {
  Lg+=StrCopyPgmToChar(&Reponse[Lg]," CCID=");
  Lg+=StrCopyToChar(&Reponse[Lg],Gsm_CCID);
  }
else
  {
  Lg+=StrCopyPgmToChar(&Reponse[Lg]," CCID=Z");
  }


if(Gsm_IMEI[0])
  {
  Lg+=StrCopyPgmToChar(&Reponse[Lg]," IMEI=");
  Lg+=StrCopyToChar(&Reponse[Lg],Gsm_IMEI);
  }
else
  {
  Lg+=StrCopyPgmToChar(&Reponse[Lg]," IMEI=Z");
  }

if(Gsm_VERSION[0])
  {
  Lg+=StrCopyPgmToChar(&Reponse[Lg]," MOD=");
  Lg+=StrCopyToChar(&Reponse[Lg],Gsm_VERSION);
  }
else
  {
  Lg+=StrCopyPgmToChar(&Reponse[Lg]," MOD=Z");
  }


if(ValeurVariableLCR(cmd,"ARNS=",SaisieNumSerie,(SZ_NUM_SERIE*3)))
  {
  if(SaisieNumSerie[0]=='1')
    ToutFF=1;
  }

if((ToutFF==1)&&(ValeurVariableLCR(cmd,"NS=",SaisieNumSerie,(SZ_NUM_SERIE*3))))
  {
  // Raz du numero memorise
  for(ii=0;ii<SZ_NUM_SERIE;ii++)
    SNumSerie[ii]=0;

  ii=0;  // Idx sur numero serie (SNumSerie)
  Lg=0;  // Idx sur chaine recue (SaisieNumSerie)
  while((SaisieNumSerie[Lg])&&(ii<SZ_NUM_SERIE))
    {
    if(SaisieNumSerie[Lg]==':')
      {
      ii++;
      }
    else
      {
      SNumSerie[ii] = SNumSerie[ii] << 4;
      SNumSerie[ii]|= AscToHex(SaisieNumSerie[Lg]);
      }
    Lg++;
    }


  if(ii==SZ_NUM_SERIE-1)
    {
    WriteI2c(EEPROM, ADR_NUM_SERIE,SZ_NUM_SERIE,SNumSerie,&CodeErreurI2c,&TimeOutI2c);
    
    // Met à 0 le champs de validité usine
    Vu[0]='0';
    Vu[1]=0;
    WriteI2c(EEPROM,ADR_VALID_U,1,Vu,&CodeErreurI2c,&TimeOutI2c);

    // Le numero de serie a change, on rappelle la fonction pour repondre correctement
    Vu[0]=0;
    Traite_VERSION(Vu,numSMS);
    }
  }
}


/*
Fonction appellée pour decider si on repond FAIL, ou pas à une commande eronnée
NumSMS  = Numero du SMS pour la reponse, si Null commande série
*/
unsigned char AutorisationReponseFail(unsigned char *numSMS)
{
unsigned char Retour;

Retour=0;
if(numSMS==NULL) 
  {
  // Reponse fail autorisee sur port serie
  Retour=1;
  }
else
  {
  // Ne repond pas aux numeros abreges, les numeros doivent commencer par + et l'option positionnee SMSFAIL AUTR=1
  if((OptionsSoft&MSQ_OPTIONS_REPOND_FAIL)&&(numSMS[0]=='+'))
    {
    // Ne repond pas si autorise mais 3 mauvaises commandes successives
    if(RepFailSMS.NbRep<2)
      {
      RepFailSMS.NbRep++;
      Retour=1;
      }
    // Memorise le dernier numero de mauvaise reponse (déplacé plus bas V170817)
//    StrCopyToChar(RepFailSMS.NumSMS,numSMS);
//    LectureDateHeure(&RepFailSMS.DtCmd); Nathan
    }
  // Memorise le dernier numero de mauvaise commande V170817
  StrCopyToChar(RepFailSMS.NumSMS,numSMS);
  //LectureDateHeure(&RepFailSMS.DtCmd); Nathan
  }
return(Retour);
}



/*
Fonction de traitement d'une commande
cmd     = commande recue
NumSMS  = Numero du SMS pour la reponse, si Null
*/
void TraiteCommande(unsigned char *cmd,unsigned char *numSMS,unsigned char *DateSMS,unsigned char *HeureSMS)
{
unsigned char Lg;
unsigned char ReponseAutorisee;
unsigned char LgCmd;


// Raz de la variable globale
Reponse[0]=0;
if(FindFirstSTRInBuff (cmd," SI=1",&Lg))
  Silence=1;          
else
  Silence=0;

switch(cmd[0])
  {
  case 'A':
    if(StrCmpInBuff(cmd,"APPEL"))
      Traite_APPEL(cmd,numSMS);
#ifdef GAINDEPLACE 
    if(StrDebute(cmd,"ALARME"))
      Traite_ALARME(cmd,numSMS);
#endif
    break;

  case 'B':
    if(StrCmpInBuff(cmd,"BIP"))
      Traite_BIP(cmd,numSMS);
    break;

  case 'C':
    if(StrCmpInBuff(cmd,"CFA"))
      Traite_CFA(cmd,numSMS);
    if(StrCmpInBuff(cmd,"CFPAU"))
      Traite_CFPAU(cmd,numSMS);
     if(StrCmpInBuff(cmd,"CFPIN"))
      Traite_CFPIN(cmd,numSMS);
    if(StrCmpInBuff(cmd,"CFS"))
      Traite_CFS(cmd,numSMS);
    if(StrCmpInBuff(cmd,"CPT"))
      Traite_CPT(cmd,numSMS);
    break;

  case 'D':
    if(StrCmpInBuff(cmd,"DT"))
      Traite_DT(cmd,numSMS,0);
    break;

  case 'F':
    if((StrCmpInBuff(cmd,"FREQ"))&&(numSMS==NULL))  // Commande uniquement depuis port serie
      Traite_FREQ(cmd,numSMS);
    break;

  case 'G':
    if(StrCmpInBuff(cmd,"GAIN"))
      Traite_GAIN(cmd,numSMS);
    if(StrCmpInBuff(cmd,"GRAF"))
      Traite_GRAF(cmd,numSMS);
    if(StrCmpInBuff(cmd,"GUSINE"))
      Traite_GUSINE(cmd,numSMS);
    if(StrCmpInBuff(cmd,"GPSLOC"))
      Traite_GPSLOC(cmd,numSMS);
    if(StrCmpInBuff(cmd,"GPSREF"))
      Traite_GPSREF(cmd,numSMS);
    if((StrCmpInBuff(cmd,"GPS"))&&((cmd[3]==0)||(cmd[3]==' ')))
      Traite_GPS(cmd,numSMS);
    
 //   if(StrDebute(cmd,"GPSWRITE"))
 //     Traite_GPSWRITE(cmd,numSMS);
    
    break;

  case 'I':
    // Attention pas de commande commançant par IPAU, les SMS commencant par IPAU ne sont pas traites
    // Car SMS d'info des PAU distant 

    if(StrCmpInBuff(cmd,"INFO"))
      Traite_INFO(cmd,numSMS);
    if(StrCmpInBuff(cmd,"ISDST"))
      Traite_ISDSTATUS(cmd,numSMS);
    break;


  case 'L':
    if(StrCmpInBuff(cmd,"LCONF"))
      Traite_LCONF(cmd,numSMS);
    break;

  case 'M':
    if(StrCmpInBuff(cmd,"MSGTEST"))
      Traite_MSGTEST(cmd,numSMS);
    if((StrCmpInBuff(cmd,"MSGEFFACE"))&&(numSMS==NULL))  // Commande uniquement depuis port serie
      Traite_MSGEFFACE(cmd,numSMS);
    if((StrCmpInBuff(cmd,"MSGENREG"))&&(numSMS==NULL))  // Commande uniquement depuis port serie
      Traite_MSGENREG(cmd,numSMS);
    if(StrCmpInBuff(cmd,"MODEM")) 
      Traite_MODEM(cmd,numSMS);
    if((StrCmpInBuff(cmd,"MAPPING"))&&(numSMS==NULL))  // Commande uniquement depuis port serie
      Traite_MAPPING(cmd,numSMS);
    break;

  case 'O':
    if(StrCmpInBuff(cmd,"OPERATEUR")) 
      Traite_OPERATEUR(cmd,numSMS);
    break;
    
  case 'P':
    if(StrCmpInBuff(cmd,"PC")) 
      Traite_PC(cmd,numSMS);
    if(StrCmpInBuff(cmd,"PAUSTA")) // Pausta = demande de status en protocole APRR
      Traite_ST_PAUSTA(cmd,numSMS,1);
    if(StrCmpInBuff(cmd,"PAUTST")) // PAUTST = demande de test en protocole APRR
      Traite_PAUTST(cmd,numSMS);
    break;

  case 'R':  
    if((StrCmpInBuff(cmd,"RACCROCHE"))&&(numSMS==NULL))  // Commande uniquement depuis port serie
      Traite_RACCROCHE(cmd,numSMS);
    if((StrCmpInBuff(cmd,"REPONDRE"))&&(numSMS==NULL))  // Commande uniquement depuis port serie
      Traite_REPONDRE(cmd,numSMS);
    if(StrCmpInBuff(cmd,"REBOOT"))  // Commande uniquement depuis port serie
      Traite_REBOOT(cmd,numSMS);
    break;

  case 'S':  
    if(StrCmpInBuff(cmd,"ST"))
      {
      Traite_ST_PAUSTA(cmd,numSMS,0);
      }
    else
      {
      if((StrCmpInBuff(cmd,"SMS"))&&(numSMS==NULL))  // Commande SMS uniquement depuis port serie
        Traite_SMS(cmd);
      if((StrCmpInBuff(cmd,"S "))&&(numSMS==NULL))   // envoi de SMS uniquement depuis port serie
        Traite_S(cmd);
      if(StrCmpInBuff(cmd,"SIMULE"))
        Traite_SIMULE(cmd,numSMS);
      if(StrCmpInBuff(cmd,"SLEEP"))
        Traite_SLEEP(cmd,numSMS);
      if(StrCmpInBuff(cmd,"SMSFAIL"))
        Traite_SMSFAIL(cmd,numSMS);
      }
    break;

  case 'T':  
    if((StrCmpInBuff(cmd,"TWDG"))&&(numSMS==NULL))
      {
      while(1); // TWDG
      }

    if(StrCmpInBuff(cmd,"TESTAUDIO"))
      Traite_TESTAUDIO(cmd,numSMS);
    break;

  case 'V':  
    if(StrCmpInBuff(cmd,"VANA"))
      Traite_VANA(cmd,numSMS);
    if(StrCmpInBuff(cmd,"VERSION"))
      Traite_VERSION(cmd,numSMS);
    if(StrCmpInBuff(cmd,"VERIFNUMTELPAU"))
      Traite_VERIFNUMTELPAU(cmd,numSMS,DateSMS,HeureSMS);
    break;
  }

if(Silence)
  {
  // Si silence on efface buffer et on emet rien
  Reponse[0]=0;
  }
else
  {
  // Ajout d'une reponse Fail, si pas de réponse et numero recu commencant par +
  if(Reponse[0]==0)
    {
    ReponseAutorisee=AutorisationReponseFail(numSMS);  // repond fail par SMS sous condition

    if(ReponseAutorisee)
      {
      Lg=StrCopyPgmToChar(Reponse,DEB_REP_LOCALE"FAIL ");
      LgCmd=0;
      while((Lg<SZ_CMD_REPONSE)&&(cmd[LgCmd]))
        {
        Reponse[Lg]=cmd[LgCmd];
        Lg++;
        LgCmd++;
        }
      Reponse[Lg]=0;
      }
    else
      {
      DebutTraceDebugConsole();
      StrToConsole("Pas de reponse fail a : ");
      StrToConsole(numSMS);
      StrToConsole("\n");
      }
    }
  else
    {
    if(numSMS!=NULL)
      {
      if(!strcmp((char*)RepFailSMS.NumSMS,(char*)numSMS))
        {
        // question correcte depuis numero SMS mis de coté, on repond et enleve l'indicateur de non repone
        RepFailSMS.NbRep=0;
        }
      }
    // Commande correcte, on ne s'endort pas
    CptPresPC=30;
    }

  // si commande port serie, on repond dessus
  if((numSMS==NULL)&&(Reponse[0]))
    {
    StrToConsole(Reponse);
    StrToConsole("\n");
    Reponse[0]=0;
    }
  
  // Si reception SMS, on raz eventuelle erreur de reseau
  if(numSMS!=NULL)
    ResetBitMotErreur(MSQEPAU_GSM_MT_RESEAU_GSM,1);
  }  
}

int ValeurVariableLCR(unsigned char *ChaineRecue,unsigned char* NomVar,unsigned char *ValVar,int SzVar)
        {
            int Retour;
            unsigned char Idx;
            int ii;

            Retour=0;
            if(ChaineRecue!=NULL)
                {
                    if(FindFirstSTRInBuff (ChaineRecue,NomVar,&Idx))
                        {
                            // Rajoute la longueur de la variable a l'index retourne
                            Idx+=StrLength(NomVar);

                            // Recopie la valeur de la variable
                            ii=0;
                            while((ii<SzVar)&&(ChaineRecue[Idx]!=' ')&&(ChaineRecue[Idx]!=LF)&&(ChaineRecue[Idx]!=CR)&&(ChaineRecue[Idx]))
                                {
                                    ValVar[ii]=ChaineRecue[Idx];
                                    Idx++;
                                    ii++;
                                }
                            ValVar[ii]=0;
                            Retour=1;
                        }
                }
            return(Retour);
        }
    
    unsigned char SendDTMF(unsigned char * BuffData, unsigned char Lengt , unsigned char Time)
        {
            unsigned char i=0;

            if(Lengt>SZ_DTMF_OUT ||Lengt==0 ||  Time>20 || Time==0)return 0;

            //Controle le buffer d'entrée
            for(i=0;i<Lengt;i++)
                {
                    switch (BuffData[i])
                        {
                            case '0' :
                            case '1' :
                            case '2' :
                            case '3' :
                            case '4' :
                            case '5' :
                            case '6' :
                            case '7' :
                            case '8' :
                            case '9' :
                            case '*' :
                            case '#' :
                            case 'A' :
                            case 'B' :
                            case 'C' :
                            case 'D' :
                              DTMFOut.Buff[i]=BuffData[i];
                              break;

                            default :
                              return 0;
                              break;
                        };
                }
            DTMFOut.Lengt=Lengt;
            DTMFOut.Time=Time;
            DTMFOut.Erreur=0;
            DTMFOut.ATraiter=1;
            return 1;
        }
    
    /*
unsigned char CodePinValide(unsigned char *CodePin)
Retourne une valeur differente de 0 si la chaine recue represente un code PIN valide.
ENTREE : NumTel  = Numero de téléphone a analyser
SORTIE : Aucune
RETOUR : 0=code non valide, 1 = code valide  (SZ_CODE_PIN car de  '0' à '9')
Au minimum 1 chiffre pouvant etre precede par un +
*/
unsigned char CodePinValide(unsigned char *CodePin)
{
unsigned char Retour;

Retour=0;

while(isnumber(*CodePin))
  {
  CodePin++;
  Retour++;
  }

if(Retour==SZ_CODE_PIN)
  Retour=1;
else
  Retour=0;
return(Retour);
}

#endif