#include"xc.h"
#include <string.h>    // definition de NULL
#include<stdio.h>
#include"Recherche.h"
#include"Copy_Compare.h"
#include"Conversion.h"
#include"cmd_traitement.h"
#include"pau_glo.h"
#include"pau_cst.h"
#include"pau_typ.h"
#include"Plan_Memoire.h"
#include"Noyau.h"
#include"pau_pro.h"
#include"ApiGsm.h"
#include"Sim900D.h"
#include"1_Gsm.h"
#include"5_IoExt.h"
#include"6_IcsI2c.h"
#include"8_Com232.h"



unsigned char NomPau[SZ_NOM_PAU+1];
unsigned char MotEtatGPS;
unsigned char ValService;
enum GsmErr GSM_ErrorFred;
unsigned char ValOperationnel;
unsigned char Reponse[SZ_CMD_REPONSE+1];
unsigned int VAna12V;
unsigned int VAna4V2;
unsigned int VAna3V3;
unsigned int VAnaE4;
unsigned int VAnaE3;
unsigned int VAnaE2;
unsigned int VAnaE1;
unsigned int VAnaPanneauVide;
unsigned char ChargeurActif;
unsigned char ForcerChargeurSec;
uint16 SeuilBasCharge;
uint16 SeuilHautCharge;
uint16 DeltaPanBatCharge;

/*
unsigned char FabriqueChaineEntrees(unsigned char *Che)
Ecrit la valeur des entrees dans une chaine de caracteres
Les entrées sont actives à l'état bas, contact entre Masse et connecteur pour activer l'entree, 1 dans la trame
ENTREE : Che    = Chaine de reception (! pas de controle de taille, doit faire au moins 6 car )
SORTIE : Out
RETOUR : Nombre de caracteres positionnés dans che
*/
unsigned char FabriqueChaineEntrees(unsigned char *Che)
{
if(_EXT_I1)
  Che[0]='0';
else
  Che[0]='1';
if(_EXT_I2)
  Che[1]='0';
else
  Che[1]='1';
if(_EXT_I3)
  Che[2]='0';
else
  Che[2]='1';
if(_EXT_I4)
  Che[3]='0';
else
  Che[3]='1';
/*
//Che[4]=0;
if(IN_MIC_DETECT)
  Che[4]='0';
else
  Che[4]='1';
 */
Che[5]=0;
return(5);
}

/*
unsigned char FabriqueChaineSorties(unsigned char *Che)
Ecrit la valeur des sortiees dans une chaine de caracteres
ENTREE : Che    = Chaine de reception (! pas de controle de taille, doit faire au moins 5 car )
SORTIE : Out
RETOUR : Nombre de caracteres positionnés dans che
*/
unsigned char FabriqueChaineSorties(unsigned char *Che)
{
if(_EXT_O1)
  Che[0]='1';
else
  Che[0]='0';
if(_EXT_O2)
  Che[1]='1';
else
  Che[1]='0';
if(_EXT_O3)
  Che[2]='1';
else
  Che[2]='0';
if(_EXT_O4)
  Che[3]='1';
else
  Che[3]='0';
Che[4]=0;

return(4);
}

void FabriqueReponseStatus(unsigned char *Rs,uint16 MotErreurEmis,char FrtReduit, unsigned char SzBuf)
{
unsigned char Lg;
unsigned char Tmp;

if(FrtReduit)
  {
  Lg=StrCopyPgmToChar(Rs,DEB_REP_LOCALE"PAUSTA PAU=\"");
  
  memset(NomPau,0,SZ_NOM_PAU);
  ReadI2c(EEPROM, ADR_NOM_PAU,SZ_NOM_PAU,NomPau,&CodeErreurI2c,&TimeOutI2c);
  NomPau[SZ_NOM_PAU]=0;
  Lg+=StrCopyToChar(&Rs[Lg],NomPau);

  Lg+=StrCopyPgmToChar(&Rs[Lg],"\" STPAU=");
  /// Lg+=uchartoa(MotErreurEmis,&Rs[Lg],3);   V171207
  Lg+=uinttoa(MotErreurEmis,&Rs[Lg]);


  Lg+=StrCopyPgmToChar(&Rs[Lg]," STGPS=");
  Lg+=uchartoa(MotEtatGPS,&Rs[Lg],0);  
  }
else
  {
  Lg=StrCopyPgmToChar(Rs,DEB_REP_LOCALE"ST ME=");
  Lg+=uinttoa(MotErreurEmis,&Rs[Lg]);

  Lg+=StrCopyPgmToChar(&Rs[Lg]," EE=");
  Rs[Lg]=StatusPau.EtatPAU;
  Lg+=1;
  Rs[Lg]=0;
  
  if(Lg<(SzBuf-5-SZ_NUM_TEL))
    {
    if(AppelEntrant.Status==APPEL_ACTIF)//si appel en status actif
      {
      Lg+=StrCopyPgmToChar(&Rs[Lg]," N=");
      Lg+=StrCopyToChar(&Rs[Lg],IncomCallPN);
      }
    else
      {
      if(AppelSortant.Status!=APPEL_AUCUN)//si appel en status actif [
        {
        Lg+=StrCopyPgmToChar(&Rs[Lg]," N=");
        Lg+=StrCopyToChar(&Rs[Lg],OutgoingCallPN);
        }
      else
        {
        Lg+=StrCopyPgmToChar(&Rs[Lg]," N=Z");
        }
      }
    }

  if(Lg<(SzBuf-9))
    {
    Lg+=StrCopyPgmToChar(&Rs[Lg]," GM=");
    Tmp=GetGsmMode();                         
    switch(Tmp)
      {
      case GSM_MODE_NORMAL:
        Rs[Lg]='N';
        break;
      case GSM_MODE_SLEEP:
        Rs[Lg]='S';
        break;
      case GSM_MODE_OFF:
        Rs[Lg]='O';
        break;
      default:
        Rs[Lg]='?';
        break;
      }
    Lg+=1;
    Rs[Lg]=0;
    }

  if(Lg<(SzBuf-9))
    {
    Lg+=StrCopyPgmToChar(&Rs[Lg]," GS=");
    Lg+=uchartoa(StatusPau.GsmState,&Rs[Lg],3);
    }
  if(Lg<(SzBuf-9))
    {
    Lg+=StrCopyPgmToChar(&Rs[Lg]," GE=");
    Lg+=uchartoa(GSM_ErrorFred,&Rs[Lg],3);
    }
  if(Lg<(SzBuf-9))
    {
    Lg+=StrCopyPgmToChar(&Rs[Lg]," QS=");
    Lg+=uchartoa(StatusPau.DerGsmRSSI,&Rs[Lg],2);
    }

  if((StatusPau.MoyRssiAcquise)&&(Lg<(SzBuf-9)))
    {
    Lg+=StrCopyPgmToChar(&Rs[Lg]," QSM=");
    Lg+=uchartoa(StatusPau.ValMoyGsmRSSI,&Rs[Lg],2);
    }
  if(Lg<(SzBuf-9))
    {
    Lg+=StrCopyPgmToChar(&Rs[Lg]," IN=");
    Lg+=FabriqueChaineEntrees(&Rs[Lg]);
    }
  if(Lg<(SzBuf-9))
    {
    Lg+=StrCopyPgmToChar(&Rs[Lg]," OUT=");
    Lg+=FabriqueChaineSorties(&Rs[Lg]);
    }
  if(Lg<(SzBuf-9))
    {
    Lg+=StrCopyPgmToChar(&Rs[Lg]," SPC=");
    Lg+=uchartoa(StatusPau.SPC,&Rs[Lg],3);
    }
  if(Lg<(SzBuf-9))
    {
    Lg+=StrCopyPgmToChar(&Rs[Lg]," GPS=");
    Lg+=uchartoa(MotEtatGPS,&Rs[Lg],0);
    }
  if(Lg<(SzBuf-9))
    {
    Lg+=StrCopyPgmToChar(&Rs[Lg]," SRV=");
    if(ValService==VAL_HORS_SERVICE)
      Lg+=StrCopyPgmToChar(&Rs[Lg],"0");
    else
      Lg+=StrCopyPgmToChar(&Rs[Lg],"1");
    }
  }

if(Lg<(SzBuf-9))
  {
  Lg+=StrCopyPgmToChar(&Rs[Lg]," OPE=");
  Lg+=uchartoa(ValOperationnel,&Rs[Lg],0);
  }

if((FrtReduit==0)&&(Lg<(SzBuf-6)))
  {
  Lg+=StrCopyPgmToChar(&Rs[Lg]," WD=");
  //Lg+=uchartoa(DateCourante.wd,&Rs[Lg],0);     Nathan


  // Ajout des compteurs de SMS emis
  if(Lg<(SzBuf-9))
    {
    Lg+=StrCopyPgmToChar(&Rs[Lg]," SH=");             // Sortant hier
    Lg+=uchartoa(SMSSortant.CptHier ,&Rs[Lg],0);
    }

  if(Lg<(SzBuf-9))
    {
    Lg+=StrCopyPgmToChar(&Rs[Lg]," SHA=");             // Sortant hier a l'initiative de la borne
    Lg+=uchartoa(SMSSortant.CptHierAuto ,&Rs[Lg],0);
    }
  }
}

void FabriqueEtEmetReponseStatus(void)
{
FabriqueReponseStatus(Reponse,StatusPau.MotErreurPAU,0,SZ_CMD_REPONSE+1);
StrToConsole((char*)Reponse);
StrToConsole("\n");
Reponse[0]=0;
}

unsigned char ResetBitMotErreur(uint16 Masque,unsigned char EmetSiModif)
{
unsigned char Retour;

if((StatusPau.MotErreurPAU&Masque)==0)
  {
  Retour=0;   // Le bit est deja a 0
  }
else
  {
  StatusPau.MotErreurPAU&=~Masque;
  Retour=1;
  if(EmetSiModif)
    FabriqueEtEmetReponseStatus();
  }
return(Retour);
}


unsigned char SetBitMotErreur(uint16 Masque,unsigned char EmetSiModif)
{
unsigned char Retour;

if((StatusPau.MotErreurPAU&Masque)==Masque)
  {
  Retour=0;   // Le ou les bits sont deja a 1
  }
else
  {
  StatusPau.MotErreurPAU|=Masque;
  Retour=1;
  if(EmetSiModif)
    FabriqueEtEmetReponseStatus();
  }
return(Retour);
}

void FabriqueReponseVANA(unsigned char *Rs)
{
unsigned char Lg;

Lg=StrCopyPgmToChar(Rs,DEB_REP_LOCALE"VANA V12=");
Lg+=uinttoa(VAna12V,&Rs[Lg]);
Lg+=StrCopyPgmToChar(&Rs[Lg]," V4.2=");
Lg+=uinttoa(VAna4V2,&Rs[Lg]);
Lg+=StrCopyPgmToChar(&Rs[Lg]," V3.3=");
Lg+=uinttoa(VAna3V3,&Rs[Lg]);
Lg+=StrCopyPgmToChar(&Rs[Lg]," VE4=");
Lg+=uinttoa(VAnaE4,&Rs[Lg]);
Lg+=StrCopyPgmToChar(&Rs[Lg]," VE3=");
Lg+=uinttoa(VAnaE3,&Rs[Lg]);
Lg+=StrCopyPgmToChar(&Rs[Lg]," VE2=");
Lg+=uinttoa(VAnaE2,&Rs[Lg]);
Lg+=StrCopyPgmToChar(&Rs[Lg]," VE1=");
Lg+=uinttoa(VAnaE1,&Rs[Lg]);
Lg+=StrCopyPgmToChar(&Rs[Lg]," VPV=");
Lg+=uinttoa(VAnaPanneauVide,&Rs[Lg]);

if(ChargeurActif)
  Lg+=StrCopyPgmToChar(&Rs[Lg]," CB=1");
else
  Lg+=StrCopyPgmToChar(&Rs[Lg]," CB=0");
/*
if(OutAux)
  Lg+=StrCopyPgmToChar(&Rs[Lg],"1");
else
  Lg+=StrCopyPgmToChar(&Rs[Lg],"0");                    ANCIEN SOFT, PLUS D'ACTUALITE

if(OutPWM)
  Lg+=StrCopyPgmToChar(&Rs[Lg],"1");
else
  Lg+=StrCopyPgmToChar(&Rs[Lg],"0");
*/
Lg+=StrCopyPgmToChar(&Rs[Lg]," FORCE=");
Lg+=uchartoa(ForcerChargeurSec,&Rs[Lg],1);


Lg+=StrCopyPgmToChar(&Rs[Lg]," SBC=");
Lg+=uinttoa(SeuilBasCharge,&Rs[Lg]);

Lg+=StrCopyPgmToChar(&Rs[Lg]," SHC=");
Lg+=uinttoa(SeuilHautCharge,&Rs[Lg]);

Lg+=StrCopyPgmToChar(&Rs[Lg]," DPBC=");
Lg+=uinttoa(DeltaPanBatCharge,&Rs[Lg]);

}



void FabriqueEtEmetReponseVANA(void)
{
FabriqueReponseVANA(Reponse);
StrToConsole((char*)Reponse);
StrToConsole("\n");
Reponse[0]=0;
}