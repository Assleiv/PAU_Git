#ifndef PAU_CST_H
#define PAU_CST_H

#define SOFTWARE_VERSION	"181107"
#define PRODUCT_NAME		"MTXGV2"

#define CH_NUM_NON_PRESENTE_GSM "00000000000000"

#define TAILLE_BUFF_CONSOLE	((unsigned int) 256)

// Taille max d'un SMS
#define SZ_DATA_SMS ((unsigned char) 160)

// Taille max des buffer de commande et de reponses
#define SZ_CMD_REPONSE  ((unsigned char) (SZ_DATA_SMS+5))

// Taille chaine DTMF max
#define SZ_DTMF_OUT 10

#define CR 13
#define LF 10

#define NB_SORTIE      4

#define SZ_CU_VERIFNUM 6   // Taille chaine unique dans SMS de verification du numero pau

#define SZ_NOM_PAU  20
#define SZ_CODE_PIN 4
#define SZ_DTMF_PAU 3
#define SZ_HTJ      5  // chaine hh:mm test jounaliers ! stockee en 2 octets h et mm
#define SZ_DMC      4  // chaine DMC duree max comm ! stockee en int 2 octets
#define SZ_TXT_SMS_APPEL 12  // Taille chaine texte SMS // ! si valeur augmentee, penser au plan memoire et  SZ_MAX_CH_CFA

#define SZ_NUM_TEL ((unsigned char) 15) // ! si valeur augmentee, penser au plan memoire et  SZ_MAX_CH_CFA et SZ_TRACE

#define SZ_TRACE 20

#define SZ_MAX_CH_CFA SZ_NUM_TEL   // Maximum entre SZ_TXT_SMS_APPEL et SZ_NUM_TEL

#define SZ_NUM_SERIE 3

#define DEB_REP_LOCALE   "R "
#define DEB_TRANS_SMS    "S "

// Entete ligne sur port serie trace 
//#define DEB_TRACE_DEBUG   "\r\nTRACEINFO "   // Debut de ligne de trace
#define DEB_TRACE_DEBUG_CONS   "\r\nTRACEINFO "   // Debut de ligne de trace

// Entete ligne sur port serie trace des SMS
#define DEB_TRACE_SMS_IN_REP   "SMSREP "   // SMS de reponse recu
#define DEB_TRACE_SMS_IN_CMD   "SMSCMD "   // SMS de commande recu
#define DEB_TRACE_SMS_ENV      "SMSENV "   // Autre SMS envoye
#define DEB_TRACE_SMS_ERR      "SMSERR "   // Erreur sur envoi SMS
#define DEB_TRACE_SMS_ATT_ENV  "SMSATT "   // Demande prise en compte 
#define DEB_TRACE_SMS_DEM_NTR  "SMSDNT "   // Demande non traitable 

// Debut de SMS envoye par ligne CFA

#define DEB_SMS_APPEL_SORTANT    "IPAUAAS "  // SMS avant appel sortant
#define DEB_SMS_ECHEC_SORTANT    "IPAUEAS "  // SMS Echec Appel Sortant
#define DEB_SMS_RESUME_SORTANT   "IPAURAS "  // SMS Resume Appel Sortant
#define DEB_SMS_MSG_CFA          "IPAUMSG "  // Message associe a une entree
#define DEB_SMS_DEMA_RAPPEL      "IPAUDMR "  // SMS pau demande a etre rapellé
#define DEB_SMS_RESUME_ENTRANT   "IPAURAE "  // SMS Resume Appel Entrant

#define DEB_SMS_INFORMATION      "IPAU"      // Debut des SMS d'info, si recu par SMS, on renvoi sur port serie

#define DEB_SMS_RESUME_ENTRANT_REDUIT   "R PAUAUDIO "  // SMS Resume Appel entrant format reduit
#define DEB_SMS_ECHEC_SORTANT_REDUIT    "R APPKO "   // SMS Echec Appel Sortant format reduit
 

// Si la commande recue sur port serie commence par DEB_TRANS_SMS, elle est émise en SMS au numero précédement paramétré

// Si le SMS recu commence par  DEB_REP_LOCALE, il est émis sur le port série, précédé du numéro

// Une réponse locale en série ou SMS commence toujours par  DEB_REP_LOCALE

// Masque binaire sur champs I du CFA
#define MSQ_SMS_APPEL_PAU      0x01
#define MSQ_SMS_RESUME         0X02
#define MSQ_SMS_ECHEC          0X04
#define MSQ_SMS_APPEL_ENTRANT  0X08
#define MSQ_SMS_TESTAUDIO      0X10
#define MSQ_SMS_DEM_RAPPEL     0X20
#define MSQ_SMS_CHGT_ERREUR    0X40
#define MSQ_SMS_FRT_REDUIT     0X80   // Format reduit, Protocole APRR, positionne par CFA


enum ENU_DTMF_APPEL 
{
  EDTA_AUCUNE,              // Pas de DTMF emise
  EDTA_PRESENTATION,        // 3 x DTMF PAU
  EDTA_TEST,                // Etat du PAU au format comprehensible par PIRAU
};

// Masque binaire sur variable globale T4_EtatAppelVoix
#define MSQ_AV_POSSIBLE           0x01   // Appel possible
#define MSQ_AV_APPEL_EN_COURS     0x02   // Appel en courseuati
#define MSQ_AV_PHONIE_EN_COURS    0x04   // phonie en cours
#define MSQ_AV_PRE_DTMF_EN_COURS  0x08   // Presentation DTMF en cours

#define MSQ_AV_FINI_OK            0x20   // Appel fini OK
#define MSQ_AV_FINI_ERREUR        0x40   // Appel fini en erreur


// Tempo acheminement par défaut
#define TPA_DEFAUT            60


enum ENU_VERIF_NUM_PAU 
{
  EVNP_AUCUNE,         // Pas de verification en cours
  EVNP_NUM_DEMANDEE,   // verification numero demandée
  EVNP_GSM_DEMANDEE,   // verification GSM demandée
  EVNP_ATTEND_SMS,     // Attente retour SMS
  EVNP_SMS_OK,         // Retour SMS OK, numero validé
};


enum ENU_ETAT_PAU
{
  EEP_REPOS='R',   // Repos
  EEP_TEST ='T',   // Test audio

  EEP_APP_S='A',   // Appel sortant
  EEP_APP_E='E',   // Appel entrant, Sonnerie
  EEP_DTMF ='D',   // Emission DTMF
  EEP_PHO_S='O',   // Phonie appel sortant
  EEP_PHO_E='I',   // Phonie appel entrant
  EEP_FIN_P='M',   // Fin appel sortant (envoi des SMS)
  EEP_WAIT_R='W',   // Attente rappel
};

// Entree associee au BP
#define InTOR_BP	    InTOR1

// Index du grafcet de filtrage de l'entree 4 pour detection porte ouverte 
#define IDX_TOR_PORTE   3

#define NUM_MSG_APPEL  1
#define NUM_MSG_ERREUR 2


// Bit mot etat PAU GSM MT 
#define MSQEPAU_GSM_MT_COUV_LIMITE       1 // B0 pau radio    -> B28 Pirau, couverture faible
#define MSQEPAU_GSM_MT_BATTERIE          2
#define MSQEPAU_GSM_MT_ENTREE_BLOQUEE    4
#define MSQEPAU_GSM_MT_PANNEAU_SOLAIRE   8
#define MSQEPAU_GSM_MT_RESEAU_GSM       16
#define MSQEPAU_GSM_MT_PHONIE           32 // B5 Defaut de phonie
#define MSQEPAU_GSM_MT_APPEL_NON_ABOUTI 64 // B6 Appel non abouti
#define MSQEPAU_GSM_MT_PORTE_OUVERTE   128 // B7 Contact porte sur E4

#define MSQEPAU_GSM_MT_DEF_BOUTON      256 // B8 Defaut bouton testable
#define MSQEPAU_GSM_MT_ROAMING         512 // B9 Modem accroché en roaming

// Masque GUSINE champs CFPAU MDSC (envoi de SMS sur chgt defaut) Tous sauf niveau reception
#define MSQ_GUSINE_MDSC ((uint16)(MSQEPAU_GSM_MT_BATTERIE|MSQEPAU_GSM_MT_ENTREE_BLOQUEE|MSQEPAU_GSM_MT_PANNEAU_SOLAIRE|MSQEPAU_GSM_MT_RESEAU_GSM|MSQEPAU_GSM_MT_PHONIE|MSQEPAU_GSM_MT_APPEL_NON_ABOUTI|MSQEPAU_GSM_MT_PORTE_OUVERTE|MSQEPAU_GSM_MT_DEF_BOUTON|MSQEPAU_GSM_MT_ROAMING))



///// !!!!!!!!  Si ajout defaut, modifier champs CFPAU MDSC dans fonction traite_gusine

// Valeur du signal pour generer erreur
#define SEUIL_RSII_BAS 9      // Seuil en dessous duquel le niveau est considere mauvais
#define SEUIL_RSII_HAUT 14    // Seuil en dessus duquel le niveau est considere bon

// Tempo d'acheminement par defaut si appel demande avec tempo==0
#define TEMPO_ACHEMINEMENT_DEFAUT 60


// Valeurs possibles pour le champs CurIdx de la structure STR_CTRL_LCONF
enum ENU_IDX_CTRL_LCONF 
{
  ECLC_AUCUN,  // Pas de lecture en cours
  ECLC_SMS1,   // Reponse au numero de SMS 1
  ECLC_SMS2,   // Reponse au numero de SMS 2
  ECLC_SERIE,  // Reponse sur le port serie
};


#define SZ_DATE_SMS  8
#define SZ_HEURE_SMS 8


#define SZ_CCID 20
#define SZ_IMEI 20
#define SZ_VER_MODEM 30
#define SZ_MODELE 10
#define SZ_ATI9 15

// Taille des tableaux d'espionage GSM
#define SZ_TAB_ESP_GSM 100   // reduire a 100 si besoin place

// Valeur pour une heure non valide
#define HEURE_NON_POSITIONNEE 25

// Valeurs min max et defaut pour la duree d'un car DTMF
#define DCD_MIN     1
#define DCD_MAX    20
#define DCD_DEFAUT 5

// Valeurs min max et defaut pour la duree entre 2 trains DTMF
#define DIT_MIN      1
#define DIT_MAX    250
#define DIT_DEFAUT  80

// Duree minimum d'un fichier son
#define DUREE_MINI_MS_FICSON   1000
#define DUREE_ATT_MS_ACCES_ISD  500

// Principe de la detection d'absence d'un son
// 1) On lance le son
// 2) On attend 500 ms pour voir si le son est fini (evite acces et relance en permanence a l'ISD)
// 3) Si le son est fini entre 500 et 1000 ms, c'est que le fichier est absent, on ne le rejoue plus


// Nombre de time out successif du modem pour forcer l reboot
#define NB_TO_MODEM_REBOOT 10

// caracteres debut de trame GPS
#define START_GPS     '$'
// caracteres avant checksum dans trame GPS
#define START_CHK_GPS '*'
// caracteres  fin de trame GPS
#define STOP_GPS      CR

// Taille fifo interne uart2
// #define SZ_FIFO_U2 64

// Taille max d'un trame gps
#define SZ_TRAME_GPS 85  

// Taille fifo interne uart2
#define SZ_FIFO_U2 SZ_TRAME_GPS

// Taille max commande modem test
#define SZ_TRAME_MODEM_TST 20

// debut de trame GPS contenant la position
#define DEB_TRAME_POS_GPS "$GPGGA"
// Taille du debut de trame
#define SZ_DEB_TRAME 6
//#define OFFSET_POS 7

// Nombre de mesures successives pour decreter stable (valeur GUSINE))
#define NB_MES_GPS_SUCC_PAU_STABLE_DEFAUT 60
// Temps de fonctionnement du GPS avant coupure si pas de position
#define PRESET_TEMPO_2S_MAINTIENT_GPS_DEFAUT 100


// Masque sur le mot d'etat du GPS
#define MSQ_ETAT_GPS_TO                    1   //   1 Defaut Timeout GPS, aucune acquisition
#define MSQ_ETAT_GPS_POS_INSTABLE          2   //   2 Position instable non prise en compte (tempo au demarrage)
#define MSQ_ETAT_GPS_PAU_DEPLACE           4   //   4 PAU deplace
#define MSQ_ETAT_GPS_POS_INCORRECTE        8   //   8 Position lue incorrecte (GPS pas callé, demarrage) 
#define MSQ_ETAT_GPS_AUCUNE_POS_MEM     0x10   //  16 Aucune Position memorisee (Mise sous tension)
#define MSQ_ETAT_GPS_ACQ_EN_COURS       0x20   //  32 Acquisition demandee
#define MSQ_ETAT_GPS_PAU_LEGER_DEPLACE  0x40   //  64 Petit deplacement detecté (-4*Tolerance), on refait une acquisition avant de positionner l'erreur
#define MSQ_ETAT_GPS_INACTIF            0x80   // 128 GPS inactif


// Valeur par defaut du parametre anti echo 0x69 -> 6,4,1
#define DEFAUT_ECHO_MODEM 0x69
#define MAX_ES_AE_MOD 8         // Valeur max du premier parametre ES
#define MAX_SES_AE_MOD 6        // Valeur max du deuxieme parametre SES

// Valeurs pour deuxieme parametre faitBip())
#define BIP_COURT 0
#define BIP_LONG 1

// Le parametre ECHO est stocké en flash sur un octet
// b7 à b4 = Valeur du parametre  ES de 0 à 8
// b3 à b1 = Valeur du parametre SES de 0 à 6
// b0      = activation de l'anti echo


#define MAX_SVR_MODEM 16       // Valeur max du parametre SVR

// nb de car utilise dans la partie decimale des coordonnees gps
#define NB_CAR_DECI_GPS 5
// Valeur d'une unite (minute) GPS en fonction des decimales (1 suivi de NB_CAR_DECI_GPS zero)
#define UNITE_GPS_DECI  100000

#define SEUIL_BAS_CHARGE_DEFAUT  12500
#define SEUIL_HAUT_CHARGE_DEFAUT 14000
#define DELTA_PAN_BAT_CHARGE_DEFAUT 500

#define SEUIL_BAS_MAX_CHARGE 16000
#define SEUIL_HAUT_MAX_CHARGE 16500
#define DELTA_MAX_CHARGE 6000

// Valeur par defaut du champ DS
#define CFPAU_DS_DEFAUT 30

// Valeur par defaut du champ CPT MJA
#define MAX_SMS_AUTO_JOUR_DEFAUT 50

// Nb de mesures RSSI pour la moyenne
#define NB_MES_RSSI 8

#define SZ_NOM_OPERATEUR 20   // Taille nom operateur GSM

// Valeurs possibles pour la variabl RebootDemande

#define REBOOT_DEMANDE_AUCUN 0
#define REBOOT_DEMANDE_PAU   1
#define REBOOT_DEMANDE_MODEM 2


// Retour local du resultat des tests en nb de clignotement (0=tout cassé  1=Audio ok, 2=Bp Ok 3=Audio et bp ok  
// masques sur les bits CligInfoTestCligBP
#define MSQ_INFTST_SYNC_BC  0x80 // 0x80 = synchro pour attendre un FM de bit clig )
#define MSQ_INFTST_AUDIO    0x40 // 0x40=Test audio en cours
#define MSQ_INFTST_BP       0x20 // 0x20=Test bouton en cours
#define MSQ_INFTST_VAL_TEST 0x03 // Valeur de retour du test

// Valeur par defaut du bip (62191 env 1 khz))
#define PRESET_DEF_TP1 0xF2EF 

// Preset timer 1 pour test audio
#define PRESET_MIN_TP1 60000 // 770 hz
#define PRESET_MAX_TP1 63000 // 1.5 khz
#define PAS_TP1          400 // pas inc freq

#define PRESET_MONO_TP1 0xF2EF  // Frequence test avant 181025

// Masque sur TestDemLCR
#define MSQ_TESTLCR_AVEC_TRANS    1
#define MSQ_TESTLCR_SANS_TRANS    2
#define MSQ_TESTLCR_COMPLET       4
#define MSQ_TESTLCR_PRINT_E5      8 // print chgt E5
#define MSQ_MONO_FREQ          0x10 // Frequence idem ancien soft
#define MSQ_TESTLCR_MEMO_E5    0x80 // memoire E5 pour trace chgt E5

#endif