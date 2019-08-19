#ifndef _APIGSM_H
#define _APIGSM_H

#include "Sim900D.h"

//Abstarct Layer for USER

//*******************************************************************************************************************
//************************************************   GENERAL   ******************************************************
//*******************************************************************************************************************
//Etat du GSM


//***** MODE GSM ***************************************************************
//******************************************************************************

//Definition des modes
#define GSM_MODE_NORMAL	0
#define GSM_MODE_SLEEP	1
#define GSM_MODE_OFF	2

//Fonctions/macros :

/*----------------------------------------------------------------------------
Changer le mode de fonctionnement du GSM :
------------------------------------------------------------------------------
parametres :
	- unsigned char mode : Mode désiré (GSM_MODE_NORMAL ou GSM_MODE_SLEEP ou GSM_MODE_OFF)
------------------------------------------------------------------------------
Return :
	- none
----------------------------------------------------------------------------*/
extern void SetGsmMode(unsigned char mode);


/*----------------------------------------------------------------------------
Connaitre le mode de fonctionnement actuel :
------------------------------------------------------------------------------
parametres :
	- none
------------------------------------------------------------------------------
Return :
	- unsigned char		mode actuel (GSM_MODE_NORMAL ou GSM_MODE_SLEEP ou GSM_MODE_OFF)
----------------------------------------------------------------------------*/
extern unsigned char GetGsmMode(void);

//***** ETAT GSM ***************************************************************
//******************************************************************************

/*----------------------------------------------------------------------------
 Connaitre l'état  actuel du module :
------------------------------------------------------------------------------
parametres :
	- none
------------------------------------------------------------------------------
Return :
	- unsigned char		eat actuel du module
						0 GSM_ST_POWER_OFF,	//module eteint (pas d'alimentation)
						1 GSM_ST_POWER_ON,	//phase d'alimentation du module
						2 GSM_ST_CONFIG,		//phase d'init
						3 GSM_ST_PRET,		//Connecté au réseau
						4 GSM_ST_NO_SIGNAL,	//Deconnecté
						5 GSM_ST_SLEEP,		//Mode veille actif
						6 GSM_ST_ERROR		//ERREUR - (pas d'alimentation)
----------------------------------------------------------------------------*/
extern unsigned char GetGsmState(void);


/*----------------------------------------------------------------------------
 Connaitre l'erreur rencontrée dans le cas ou le GSM est en etat "ERROR"
------------------------------------------------------------------------------
parametres :
	- none
------------------------------------------------------------------------------
Return :
	- unsigned char		erreur actuelle
						0 NO_ERROR,			//Pas d'erreur
						1 CCID_ERROR,			//Le CCID n'a pu être lu
						2 SIM_ERROR,			//Erreur carte SIM
						3 PIN_CODE_ERROR		//Code PIN erroné
----------------------------------------------------------------------------*/
extern unsigned char GetGsmError(void);


/*----------------------------------------------------------------------------
Connaitre la qualité du signal (RSSI) :
------------------------------------------------------------------------------
parametres :
	- none
------------------------------------------------------------------------------
Return :
	- unsigned char		entier sur 8 bits image de la qualité du signal (RSSI) défini comme suit:
								0 			-115 dBm or less
								1 			-111 dBm
								2...30 		-110... -54 dBm
								31 			-52 dBm or greater
								99 			not known or not detectable
----------------------------------------------------------------------------*/




//*******************************************************************************************************************
//*********************************************   SMS INTERFACE   ***************************************************
//*******************************************************************************************************************

//***** Envoi d'un SMS *********************************************************
//******************************************************************************


/*----------------------------------------------------------------------------
Envoyer un sms avec comme paramètre d'entrée des chaines en rom (flash memory):
------------------------------------------------------------------------------
parametres :
	- unsigned char *PnoneNumber	- pointeur sur le 1er carcactère du buffer en RAM contenant le numero de telephone
	- unsigned char *SmsText		- pointeur sur le 1er carcactère du buffer en RAM contenant le texte du SMS

------------------------------------------------------------------------------
Return :
	- unsigned char  	1 si sms placé dans la boite d'envoi
						0 sinon (2 causes possibles, un sms est déja en attente d'envoi ou le 
								nombre maximum de sms envoyé à dépassé le seuil paramétré).
----------------------------------------------------------------------------*/
extern unsigned char SMSSend (unsigned char *PnoneNumber,unsigned char *SmsText,unsigned char FlagAuto);


/*----------------------------------------------------------------------------
Permet de savoir si le sms à été envoyé (Boite d'envoi vide)
------------------------------------------------------------------------------
parametres :
	- none
------------------------------------------------------------------------------
Return :
	- unsigned char  	1 si sms envoyé et boite d'envoi libre
						0 si sms en attente d'envoi (boite d'envoi occupée)
----------------------------------------------------------------------------*/
#define SmsSent() 			(~SMSSortant.ATraiter&0x01)


/*----------------------------------------------------------------------------
Permet de savoir si le SMS qui à été envoyé à généré une erreur
------------------------------------------------------------------------------
parametres :
	- none
------------------------------------------------------------------------------
Return :
	- unsigned char  	1 si le dernier sms envoyé à généré une erreur
						0 si le SMS à été envoyé avec succès
----------------------------------------------------------------------------*/
#define SmsSendingError()	(SMSSortant.SendingError&0x01)


/*----------------------------------------------------------------------------
Permet de savoir si le nombre de SMS envoyé à dépassé le nombre maximum
de SMS pouvant être envoyés
------------------------------------------------------------------------------
parametres :
	- none
------------------------------------------------------------------------------
Return :
	- unsigned char  	1 si le nb de SMS envoyé est > au maximum (impossible d'envoyer un SMS)
						0 sinon
----------------------------------------------------------------------------*/
//#define NbMaxSmsReach()		(CptSmsSent>=MaxSmsSent)

//Variables globales :
//extern unsigned int MaxSmsSent;	//Nombre maximum de SMS pouvant être envoyés

extern unsigned int CptSmsSent; //Compteur de SMS envoyés (incrémenté par le moteur GSM à chaque envoi)


//***** Reception d'un SMS *****************************************************
//******************************************************************************

//Fonctions/macros :

/*----------------------------------------------------------------------------
Permet de savoir si un SMS est en attente de traitement
------------------------------------------------------------------------------
parametres :
	- none
------------------------------------------------------------------------------
Return :
	- unsigned char  	1 si un sms est en attente de traitement
						0 sinon
----------------------------------------------------------------------------*/
#define SmsReady()			(SMSEntrant.ATraiter&0x01)


/*----------------------------------------------------------------------------
Permet d'acquiter le sms entrant - autorisation d'écraser le sms
------------------------------------------------------------------------------
parametres :
	- none
------------------------------------------------------------------------------
Return :
	- none
----------------------------------------------------------------------------*/
#define SmsAck()			{SMSEntrant.ATraiter=0;SMSEntrant.AEffacer=1;}

//Variables globales :
#define SmsTxt				SMSEntrant.BuffTxt	//Buffer contenant le texte du SMS recu
#define SmsTxtLen			SMSEntrant.TxtLengt	//nombre de caractère dans le buffer "SmsTxt"
#define SmsPN				SMSEntrant.BuffNum 	//Buffer contenant le numéro de telephone de l'emetteur  du SMS recu
#define SmsDate				SMSEntrant.BuffDate	//Buffer contenant la date du SMS recu (AA/MM/DD)
#define SmsHeure			SMSEntrant.BuffHeure//Buffer contenant l'heure du SMS recu (HH/MM/SS)


//*******************************************************************************************************************
//*********************************************   AUDIO INTERFACE  ***************************************************
//*******************************************************************************************************************

/*----------------------------------------------------------------------------
Permet de choisir le canal audio est de le parametrer
------------------------------------------------------------------------------
parametres :
	- ChannelNumber	 	1 = canal gsm non lié au HP et au micro
						2 =canal gsm lié au HP et au micro

	- MicLvl			entier de 0 à 15 (niveau du microphone)
	- SpkLvl			entier de 0 à 100 (niveau du HP)
------------------------------------------------------------------------------
Return :
	- unsigned char		1	Si tout les paramètres sont corrects et que la demande à été prise en compte
						0	Sinon

----------------------------------------------------------------------------*/
extern unsigned char  SetAudioChannel (unsigned char ChannelNumber , unsigned char MicLvl , unsigned char SpkLvl);


//*******************************************************************************************************************
//**************************************   APPEL VOIX FONCTIONS COMMUNES ********************************************
//*******************************************************************************************************************
//Etat des appels
#define APPEL_AUCUN			0	//Pas d'appel en cours
#define APPEL_TRAITEMENT	1	//Appel en cours de traitement 
#define APPEL_ALERT			2	//Appel en alerte (attend le décrochage ou l'autorisation de décrocher)
#define APPEL_ACTIF			3	//Appel actif (connection établie le corrspondant à décroché)
#define APPEL_DISCONNECT	6	//Appel deconnecte (Tombe sur un disque numero non atribue)

/*----------------------------------------------------------------------------
arreter toutes les connexions existantes (raccrocher)
------------------------------------------------------------------------------
parametres :
	- none
------------------------------------------------------------------------------
Return :
	- unsigned char		1	si la demande d’arret des communications à été prise en compte
						0	sinon (GSM en mode SLEEP ou OFF)
----------------------------------------------------------------------------*/
extern unsigned char HangUp(void);

/*----------------------------------------------------------------------------
Savoir s’il y a un appel en cours :
------------------------------------------------------------------------------
parametres :
	- none
------------------------------------------------------------------------------
Return :
	- unsigned char		1 	Si un appel est en cours (entrant ou sortant)
						0	sinon 
----------------------------------------------------------------------------*/
extern unsigned char VoiceCallInProgress(void);

//*******************************************************************************************************************
//*******************************************   APPEL VOIX ENTRANTS *************************************************
//*******************************************************************************************************************

#define REPONSE_AUTO		0
#define REPONSE_MANUELLE	1
/*----------------------------------------------------------------------------
Choix du mode de réponse à un appel entrant :
------------------------------------------------------------------------------
parametres :
	- mode 	 		REPONSE_AUTO
			 		REPONSE_MANUELLE
------------------------------------------------------------------------------
Return :
	- none
----------------------------------------------------------------------------*/
extern void SetAnsweringMode (unsigned char mode);

/*----------------------------------------------------------------------------
Connaitre le mode de réponse à un appel entrant actuel:
------------------------------------------------------------------------------
parametres :
	- none	
------------------------------------------------------------------------------
Return :
	- unsigned char 	REPONSE_AUTO
			 			REPONSE_MANUELLE
----------------------------------------------------------------------------*/
extern unsigned char GetAnsweringMode (void);


/*----------------------------------------------------------------------------
Connaitre l’état d’un appel entrant :
------------------------------------------------------------------------------
parametres :
	- none	
------------------------------------------------------------------------------
Return :
	- unsigned char 	APPEL_AUCUN
						APPEL_TRAITEMENT
						APPEL_ALERT	
						APPEL_ACTIF
----------------------------------------------------------------------------*/
extern unsigned char GetIncommingCallStat(void);

/*----------------------------------------------------------------------------
Donner l’autorisation de décrocher au moteur gsm (cas du mode de reponse manuel):
------------------------------------------------------------------------------
parametres :
	- none	
------------------------------------------------------------------------------
Return :
	- unsigned char 	1  Si la demande d’autorisation à été prise en compte
						0	Sinon (pas d’appel entrant en cours ou mode réponse auto actif)
----------------------------------------------------------------------------*/
extern unsigned char AnswerIncommingCall(void);

/*----------------------------------------------------------------------------
Connaitre la durée de l’appel entrant :
------------------------------------------------------------------------------
parametres :
	- none	
------------------------------------------------------------------------------
Return :
	- unsigned int	   entier 16bits – durée de l’appel entrant en secondes.
----------------------------------------------------------------------------*/
extern unsigned int GetIncommingCallTime(void);


//Variables globales :
#define IncomCallPN			AppelEntrant.BuffNum
#define IncomCallPNLengt	AppelEntrant.NumLengt


//*******************************************************************************************************************
//*******************************************   APPEL VOIX SORTANTS *************************************************
//*******************************************************************************************************************
/*----------------------------------------------------------------------------
Savoir si il est possible d’émettre un appel :
------------------------------------------------------------------------------
parametres :
	- none	
------------------------------------------------------------------------------
Return :
	- unsigned char	   	1	Si il est actuellement possible de passer un appel
						0	Sinon (non connecté au réseau ou un appel est déjà en cours)
----------------------------------------------------------------------------*/
extern unsigned char IsVoiceCallReady(void);

/*----------------------------------------------------------------------------
Demande d’émission d’un appel (paramètres en RAM) :
------------------------------------------------------------------------------
parametres :
	- 	PnoneNumber – pointeur sur un buffer contenant le numéro de téléphone de l’interlocuteur
		
------------------------------------------------------------------------------
Return :
	- unsigned char	   	1	Si la demande d’appel à été prise en compte
						0	Sinon (parametres erronés ou impossible de passer un appel actuellement).
----------------------------------------------------------------------------*/
extern unsigned char MakeVoiceCall (unsigned char *PnoneNumber,unsigned char Tpa);


/*----------------------------------------------------------------------------
Connaitre l’état d’un appel sortant :
------------------------------------------------------------------------------
parametres :
	- none	
------------------------------------------------------------------------------
Return :
	- unsigned char 	APPEL_AUCUN
						APPEL_TRAITEMENT
						APPEL_ALERT	
						APPEL_ACTIF
----------------------------------------------------------------------------*/
//extern unsigned char GetOutgoingCallStat(void);

/*----------------------------------------------------------------------------
Connaitre la durée de l’appel sortant :
------------------------------------------------------------------------------
parametres :
	- none	
------------------------------------------------------------------------------
Return :
	- unsigned int	   entier 16bits – durée de l’appel entrant en secondes.
----------------------------------------------------------------------------*/
extern unsigned int GetOutgoingCallTime(void);

//Variables globales :
#define OutgoingCallPN		    AppelSortant.BuffNum




extern unsigned char SendDTMF(unsigned char * BuffData, unsigned char Lengt , unsigned char Time);
extern unsigned char GetDTMFError (void);
extern unsigned char GetDTMFStatus (void);

#endif