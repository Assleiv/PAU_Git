
#ifndef PAU_TYP_H
#define	PAU_TYP_H

#include"pau_cst.h"
#include"stdtype.h"




struct STR_SMS_SORTANT
{
	unsigned ATraiter:1; //flag indiquant qu'un sms est en attende d'envoi
	unsigned SendingError:1;

	unsigned Bit2:1;
	unsigned Bit3:1;
	unsigned Bit4:1;
	unsigned FlagAuto:1;      // Le SMS sortant est a l'initiative de la borne 
	unsigned Tempo:2;
/*
	unsigned Bit6:1;
	unsigned Bit7:1;
*/
	unsigned char BuffTxt[SZ_DATA_SMS+1];
	unsigned char BuffNum[SZ_NUM_TEL+1];
    unsigned long Cpt;

	unsigned char CptJour;     // SMS envoyés depuis minuit
	unsigned char CptHier;     // SMS envoyés hier
	unsigned char CptJourAuto; // SMS envoyés depuis minuit a l'initiative de la borne
	unsigned char CptHierAuto; // SMS envoyés hier a l'initiative de la borne
};

extern struct STR_SMS_SORTANT SMSSortant;

struct STR_SMS_ENTRANT
{
	unsigned ATraiter:1; //flag indiquant qu'un sms est en attente de traitement
	unsigned AEffacer:1;
	unsigned ToutEffacer:1;
	unsigned Plusieurs:1;
	unsigned Bit4:1;
	unsigned Bit5:1;
	unsigned Bit6:1;
	unsigned Bit7:1;
	unsigned char BuffTxt[SZ_DATA_SMS+1];
	unsigned char BuffNum[SZ_NUM_TEL+1];
	unsigned char BuffDate[SZ_DATE_SMS+1];
	unsigned char BuffHeure[SZ_HEURE_SMS+1];

	unsigned char ModuleIndex;
	unsigned char TxtLengt;
    unsigned long Cpt;

	unsigned char CptJour; // SMS reçus depuis minuit
	unsigned char CptHier; // SMS reçus hier

};

extern struct STR_SMS_ENTRANT SMSEntrant;



struct STR_DTMF_OUT
{
	unsigned ATraiter:1; //flag indiquant qu'une DTMF est en attente de traitement
	unsigned Erreur:1;
	unsigned Bit2:1;
	unsigned Bit3:1;
	unsigned Bit4:1;
	unsigned Bit5:1;
	unsigned Bit6:1;
	unsigned Bit7:1;
	unsigned char Buff[SZ_DTMF_OUT+1];
	unsigned char Time;
	unsigned char Lengt;
};

extern struct STR_DTMF_OUT DTMFOut;

struct STR_STATUS
{
    uint16 MotErreurPAU;   //   Mot d'erreur du PAU
    unsigned char EtatPAU;
    unsigned char SPC;

    unsigned char GsmState;
    uint8  DerGsmRSSI;     // Dernier RSSI GSM lu
    uint8  ValGsmRSSI[NB_MES_RSSI];     // Dernier RSSI GSM lu
    uint8  ValMoyGsmRSSI;     // Moyenne Dernier RSSI GSM lu    
    
    unsigned IdxRssi:4;     // Indexx de la mesure dans tableau 
 	unsigned BipRssiOk:1;   // 1 si bip premier rssi ok emis
	unsigned MoyRssiAcquise:1; // 1 si la moyenne a ete calculee
	unsigned ErrMoyPre:1;   // 1 si erreur moyenne precedente
	unsigned Bit7:1;
};

extern struct STR_STATUS StatusPau;

struct STR_EQUATION_SORTIE
{
    unsigned char Fixe;
    unsigned char Clig;
    unsigned char Inv;
};


struct STR_MEM_ES
{
    unsigned MemE1:1;
    unsigned MemE2:1;
    unsigned MemE3:1;
    unsigned MemE4:1;
    unsigned MemE5:1;
    unsigned zero :3;

    unsigned MemS1:1;
    unsigned MemS2:1;
    unsigned MemS3:1;
    unsigned MemS4:1;
};


struct STR_CTRL_LCONF
{
    unsigned char NumSMS1[SZ_NUM_TEL+1];
    unsigned char NumSMS2[SZ_NUM_TEL+1];
    unsigned char Console;
    unsigned char CurIdx;
    unsigned char ActSMS1;
    unsigned char ActSMS2;
};


struct STR_DET_ABS_SON
{
    unsigned int TMsDetectAbs; // Tempo detection pas de son car son trop court
    unsigned char NumSon;
    unsigned char Absent;
    unsigned char CptSecRAZ;   // Tempo seconde pour tout razer si son absent detecte
};


struct STR_CTRL_FAIL_SMS
{
    unsigned char NumSMS[SZ_NUM_TEL+1];
    unsigned char NbRep;
// attendre Nathan    struct STR_DATE DtCmd;                      // Horodate de la derniere mauvaise commande
};


// Structure de reception des trames GPS
struct STR_CUR_TRAME_GPS
{
    unsigned char Data[SZ_TRAME_GPS+1];  // Données
    unsigned char Idx;                   // index de reception
    unsigned char TropLong;
};


// Structure de controle du test par SMS    
struct STR_CTRL_TEST_SMS
{
    unsigned char NumSMS[SZ_NUM_TEL+1];
    unsigned char FormatCourt;
};

    
#endif

