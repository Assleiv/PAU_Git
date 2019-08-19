#ifndef PAU_GLO_H
#define PAU_GLO_H

#include"Recherche.h"
#include"Conversion.h"
#include"Copy_Compare.h"
#include"pau_typ.h"
#include"pau_cst.h"
#include"pau_glo.h"
#include"Sim900D.h"
#include"stdtype.h"
#include"Plan_Memoire.h"

extern unsigned char _indexRXConsole;
extern unsigned char _indexTXConsole;

extern unsigned char CptPresPC;

// Commande en cours d'acquisition sur le port serie
extern unsigned char CurCmdSerie[SZ_CMD_REPONSE+1];
extern unsigned char IdxCmdSerie;

extern unsigned char NumSMSCible[SZ_NUM_TEL+1];
extern unsigned char VerifNumPau;     // gestion envoi d'un SMS a nous meme pour verifier numero et lecture heure voir ENU_VERIF_NUM_PAU
extern unsigned char ChUniqueVerifNum[SZ_CU_VERIFNUM+1];  // chaine unique (HHMMSS)

// Reponse a une commande 
extern unsigned char Reponse[SZ_CMD_REPONSE+1];
extern unsigned char Trace[SZ_TRACE+1];

// Parametre tempo max de comm en secondes (! en minute dans eprom)
extern unsigned int  CurDMCSec;

// Gains audio
extern unsigned char CurGainHpN;    //   (Hp Normal)
extern unsigned char CurGainMiN;    //   (Micro Normal)
extern unsigned char CurGainHpMe;   //   (HP mal entendant)
extern unsigned char CurGainMiMe;   //   (Micro mal entendant)

// Options du logiciel
extern unsigned char OptionsSoft;
// Pau en service
extern unsigned char ValService;
// Pau operationnel
extern unsigned char ValOperationnel;

// Etat de l'appel voix sortant géré par T4 
// (Signification binaire, voir les masque MSQ_AV_xxxxxx)
extern unsigned char T4_EtatAppelVoix;
extern unsigned char T4_DTMF;                     // 1 = Emission DTMF pour appel normal, 2 = Emission DTMF de test et raccroche

// Grafcets de filtrage des entrees dans T9
extern unsigned char T9_Gfe[NB_E_TOR];            // Grafcet filtrage entrees
extern unsigned char Tps_Blocage_ent[NB_E_TOR];   // Grafcet filtrage entrees
extern unsigned char EntreeSimulee;               // Entree simulee par commande SIMULE

// Mot d'etat du PAU
extern struct STR_STATUS StatusPau;

extern struct STR_EQUATION_SORTIE EquaCFS[NB_SORTIE];

// Heure et minute du test audio [0]=Heure [1]=Minute
extern unsigned char HeureMinuteTest[2];   
// Test audio demande par LCR
extern unsigned char TestDemLCR;

// Valeur analogiques en milli volt
extern unsigned int VAna12V;
extern unsigned int VAna4V2;
extern unsigned int VAna3V3;
extern unsigned int VAnaE4;
extern unsigned int VAnaE3;
extern unsigned int VAnaE2;
extern unsigned int VAnaE1;
extern unsigned int VAnaPanneauVide;   // Tension du panneau solaire a vide (chargeur deconnecté)

extern unsigned char BCligSortie;


extern struct STR_MEM_ES MemES_bits;
extern struct STR_MEM_ES PhotoES_bits;

extern unsigned char ReveilParIt;

extern unsigned char DecrocheLCR;


extern struct STR_CTRL_LCONF CtrlLConf;

extern unsigned char ModifAEModem;
extern unsigned char ModifSVRModem;

extern unsigned char GrafChargeur;
extern unsigned char DeconnecterChargeur;       // Deconnection du chargeur sur demande pour mesure panneau vide
                                                // 0=Init 1=deconnexion demandee 2=VPV memorisee
extern unsigned char ForcerChargeurSec;         // Forcage du chargeur sur demande pour maintenance
                                                // 0=Init >0=connexion demandee
extern unsigned char NumSmsVANA[SZ_NUM_TEL+1];

// Espionnage tx rx gsm adlynx
extern unsigned char CurCmdGSM[SZ_TAB_ESP_GSM+1];
extern unsigned char IdxCurCmdGSM;
extern unsigned char CurRxGSM[SZ_TAB_ESP_GSM+1];
extern unsigned char IdxCurRxGSM;


extern unsigned char CurRxGSM_IT[256];          // Tableau des caracteres recus du modem, rempli sous IT
extern unsigned char IdxCurRxGSM_IT;           // Idx Ecriture, gerepar IT     
extern unsigned char IdxCurLectRxGSM_IT_Rep;   // Idx lecture, pour gestion de la reponse
extern unsigned char Gsm_CCID[SZ_CCID+1];
extern unsigned char Gsm_IMEI[SZ_IMEI+1];
extern unsigned char Gsm_VERSION[SZ_VER_MODEM+1];
extern unsigned char Gsm_MODELE[SZ_MODELE+1];
extern unsigned char Gsm_ATI9[SZ_ATI9+1];

extern unsigned int TimeOutGSM_T12;
extern unsigned char NbToModemSucc;    // Nb de TO modem successif pour reboot

extern unsigned char LgnLigneGSM;     // Nb de car depuis CR ou LF, longueur de la ligne recue du modem
extern unsigned char ReceptionSMS;     //
extern unsigned char IdxLigneSMS;      // Index ecriture du SMS en cours de reception

extern unsigned char MemLigneGSM[256];

extern unsigned char T3_TimeOutSec;               // Time out en secondes pour sortir des etapes de Graf3
extern unsigned char T4_TimeOutSec;               // Time out en secondes pour sortir des etapes de Graf4
extern unsigned char T5_TimeOutSec;               // Time out en secondes pour sortir des etapes de Graf5
extern unsigned char T6_TimeOutSec;               // Time out en secondes pour sortir des etapes de Graf6

extern unsigned char T9_TimeOutSec;               // Time out en secondes pour sortir des etapes de Graf9
extern unsigned char T11_TimeOutSec;              // Time out en secondes pour sortir des etapes de Graf11
extern unsigned char T01_TimeOutSec;              // Time out en secondes pour sortir des etapes de Graf13


extern unsigned int CptCycleSec;

extern unsigned int  TimeSMSList;
extern unsigned int  TimeRSSI;
extern unsigned int  TimeChkcall;
extern unsigned int  TimeChkcpin;
extern unsigned char TesterPIN;

extern unsigned char ChargeurActif;               // Sert au filtrage de l'activite chargeur

extern unsigned char T02_NbBipRestant;


extern unsigned char EchecSequenceAppelSortant;   // Flag pour jouer message d'erreur en fin de seqence d'appels, si plusieurs programmes

extern struct STR_DET_ABS_SON DetectAbsSon;

extern unsigned char NomPau[SZ_NOM_PAU+1];

extern unsigned char NbEssaiHangUp;              // Nb d'essai hang up pour securite

//extern struct STR_DATE DateCourante;

//extern struct STR_DATE DateBoot;                 // Horodate du boot de la carte attendre Nathan
//extern struct STR_DATE DateReseauOK;             // Horodate de l'inscription ok sur reseau attendre Nathan
//extern struct STR_DATE DateErreurReseau;         // Horodate de non inscription reseau attendre Nathan

extern unsigned char DefautInscription;          // Flag efaut inscription GSM géré par tache modem
extern unsigned char NbDefautInscription;        // Nombre de retour en defaut d'inscription successifs
extern unsigned char NbRebootTOModem;            // Nombre de reboot modem a cause de timeout modem (modif V140404 suite pb montpellier)

extern unsigned char CurDurCarDTMF;              // Duree caractere DTMF en 1/10 sec
extern unsigned char CurDurInterTrainDTMF_20ms;  // Duree entre 2 trains DTMF (* 20 ms)

extern struct STR_CTRL_FAIL_SMS RepFailSMS;

extern struct STR_CUR_TRAME_GPS CurTrameGPS;

extern unsigned char MotEtatGPS;
extern unsigned char NbMesureGPSOk;
extern unsigned char ChTstU2[SZ_FIFO_U2+1];
extern unsigned char ChCmdModemTst[SZ_TRAME_MODEM_TST+1];

extern unsigned char NumSMS1_GPS[SZ_NUM_TEL+1];
extern unsigned char RepSMS1_GPSCourt;
extern unsigned char NumSMS2_GPS[SZ_NUM_TEL+1];
extern unsigned char RepSMS2_GPSCourt;

extern unsigned char TempoMaintientSecGPS_2Sec;

extern struct STR_CTRL_TEST_SMS CtrlTestSms;

// Derniere bonne Position lue 
//extern struct STR_DATE DbpGPS_DateAcq; attendre Nathan

extern unsigned long Dbp_Latitude;   // Parti reelle de la latitude (en degres minute)
extern unsigned long Dbp_Longitude;  // Parti reelle de la longitude (en degres minute)
extern unsigned long Dbp_Deci_Latitude;  // 3727
extern unsigned long Dbp_Deci_Longitude;  // 3735  (4 octets)
extern unsigned char Dbp_CarLongitude;
extern unsigned char Dbp_CarLatitude;
extern unsigned char Dbp_NbSat;
extern unsigned int  Dbp_Altitude;

extern unsigned char CurVersRef;  // Mis a 1 par cmd GPS REF=1, demande memorisation position courante

extern unsigned long RefLatitude;   // Parti reelle de la latitude (en degres minute)
extern unsigned long RefLongitude;  // Parti reelle de la longitude (en degres minute)
extern unsigned long RefDeci_Latitude;  // 3727
extern unsigned long RefDeci_Longitude;  // 3735  (4 octets)
extern unsigned char RefCarLongitude;
extern unsigned char RefCarLatitude;

extern unsigned long TolLatitude; 
extern unsigned long TolLongitude;

extern unsigned char ValTempoGPS_2Sec;
extern unsigned char NbMesIgnoreesGPS;

extern unsigned char NbCycleAcqGPS;
extern unsigned long DeltaDeci;   // Parti reelle de la latitude (en degres minute)
extern unsigned long DeltaEntier;  // Parti reelle de la longitude (en degres minute)

extern unsigned char ErrPCF8363;


extern uint16 SeuilBasCharge;
extern uint16 SeuilHautCharge;
extern uint16 DeltaPanBatCharge;

extern uint8  CptSecIgnoreAppel;
extern uint8  BpPendantTest;

extern unsigned char NomOperateur[SZ_NOM_OPERATEUR+1];
extern unsigned char ValReg;

extern unsigned char RebootDemande;

extern uint16 MsqDefautsSmsChgt;  // Masque sur defauts transmis par SMS chgt etat

extern uint8 MsqWdSmsVerifNum;    // Masque jours semaine pour emission du SMS de verification réseau apres test  (B0=Dimache, ....... ,B6=Samedi)
extern uint8 MsqWdSmsTest;        // Masque jours semaine pour emission des SMS d'etat apres test des lignes CFA  (B0=Dimache, ....... ,B6=Samedi)    
extern uint8 MsqWdSmsGps;         // Masque jours semaine pour emission des SMS GPS apres test des lignes CFA     (B0=Dimache, ....... ,B6=Samedi)    
extern uint8 MsqWdSmsVana;           // Masque jours semaine pour emission des SMS VANA apres test des lignes CFA    (B0=Dimache, ....... ,B6=Samedi   B7=Chgt etat chargeur ligne CFA avec SMS changement etat)

extern uint8 MaxSmsJourAutoAutorise; // Nombre maxi de SMS sortant autorises par jour à l'initiative de la borne

extern unsigned char CodeDtmfPau[SZ_DTMF_PAU+1];     // Code DTMF du PAU lu dans flash
extern unsigned char CligInfoTestCligBP;

// Acquisition GPS par appui BP
extern unsigned char AcqGpsBp;  // Si 1, acquisition GPS demandee par appui sur BP
extern uint16 PresetT1_bip;

#endif