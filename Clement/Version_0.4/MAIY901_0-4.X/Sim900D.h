#ifndef _SIM900D_H
#define _SIM900D_H

#include "cmd_traitement.h"


//Type de reponse GSM attendues
#define NO_WAIT                 0	
#define SIMPLE_VALID            1
#define RESULT_WITH_OK          2
#define SIMPLE_RESULT           3
#define RESULT_WITH_SEPRAR_OK   4
#define PARTICULAR_CHAR         5


#define TIME_INTER_SMS_LIST		((unsigned int) 3000)	//en millisecondes, temps entre 2 interrogations liste des sms entrant
#define TIME_INTER_RSSI_TEST	((unsigned int) 5000)	//en millisecondes, temps entre 2 interrogations niveau RSSI
#define TIME_CHECK_CALL_ACTIF	((unsigned int) 2000)	//en millisecondes, temps entre 2 interrogations liste des appels, pendant appel actif
#define TIME_CHECK_CALL_ALERT	((unsigned int)  500)	//en millisecondes, temps entre 2 interrogations liste des appels, pau au repos
#define TIME_INTER_CPIN	        ((unsigned int) 6000)	//en millisecondes, temps entre 2 interrogations CPIN, pour detecter rebbot interne du modem seurite)


extern enum GSMState 	
{
	GSM_ST_POWER_OFF,	//module eteint (pas d'alimentation)
	GSM_ST_POWER_ON,	//phase d'alimentation du module
	GSM_ST_CONFIG,		//phase d'init
	GSM_ST_PRET,		//Connecté au réseau
	GSM_ST_NO_SIGNAL,	//Deconnecté
	GSM_ST_SLEEP,		//Mode veille actif
	GSM_ST_ERROR		//ERREUR - (pas d'alimentation)
            
}GSM_State;

extern enum GsmErr 	
{
	NO_ERROR,			//Pas d'erreur
	CCID_ERROR,			//Le CCID n'a pu être lu
	SIM_ERROR,			//Erreur carte SIM
	PIN_CODE_ERROR		//Code PIN erroné
            
};

extern enum GsmErr GSM_ErrorFred;

extern unsigned char GsmRSSI;	//RSSI  valeur entiere de 0 à 99 (mise a jour regulierement par le moteur gsm)
								//	0 			-115 dBm or less
								//	1 			-111 dBm
								//	2...30 		-110... -54 dBm
								//	31 			-52 dBm or greater
								//	99 			not known or not detectable

extern struct GsmMod
{
	unsigned RequestOff:1; 
	unsigned RequestSleep:1;
	unsigned Bit2:1;
	unsigned Bit3:1;
	unsigned Bit4:1;
	unsigned Bit5:1;
	unsigned Bit6:1;
	unsigned Bit7:1;
};

extern struct GsmMod GsmModeFred;

extern unsigned char IndexRxGsm;
extern unsigned char IndexLectGsm;

extern unsigned char GsmRxType;

extern unsigned char NbInitGsm;
extern unsigned char NbMauvaisPin;


extern struct CALLin
{
	unsigned HangUp:1;
	unsigned ModeReponse:1;
	unsigned ReponseAutor:1;
	unsigned Bit4:1;
	unsigned Bit5:1;
	unsigned Bit6:1;
	unsigned Bit7:1;

	unsigned char BuffNum[SZ_NUM_TEL+1];
	unsigned char Status;
	unsigned int Time;

};

extern struct CALLin AppelEntrant;


struct CALLout
{
//	unsigned ATraiter:1; //flag indiquant qu'un appel entrant est en attente de traitement
	unsigned HangUp:1;
	unsigned Bit2:1;
	unsigned Bit3:1;
	unsigned Bit4:1;
	unsigned Bit5:1;
	unsigned Bit6:1;
	unsigned Bit7:1;

	unsigned char BuffNum[SZ_NUM_TEL+1];
	unsigned char Status;
	unsigned int Time;
    unsigned char Tpa_S7;

};

extern struct CALLout AppelSortant;


extern struct Channel
{
	unsigned ATraiter:1; //flag indiquant qu'un appel entrant est en attente de traitement
	unsigned Bit1:1;
	unsigned Bit2:1;
	unsigned Bit3:1;
	unsigned Bit4:1;
	unsigned Bit5:1;
	unsigned Bit6:1;
	unsigned Bit7:1;

	unsigned char ChannelNb;
	unsigned char MicLevel;
	unsigned char SpkLevel;

};

extern struct Channel AudioChannel;



#endif