#ifndef PLAN_MEMOIRE_H
#define PLAN_MEMOIRE_H

#include "pau_cst.h"

/*
!!!! !!!! ATTENTION  !!!! !!!!
Les mémoires sont organisées en page de 64 octets.
La premiere page commence à l'adresse 0
Il ne faut pas lire ou ecrire d'un coup sur 2 pages differentes
*/

/*
Pour chaque ligne, l'octet ECDM contient les infos Cyclique / entrees / DTMF
b0     = info appel cyclique sur cette ligne de config
b1-b4  = Entrees associées a cette ligne de config
b5     = utilisation de la dtmf si appel voix
b6     = Info numero exterieur 
*/
#define MSQ_CAR_ECDM_CYCLIQUE 0x01
#define MSQ_CAR_ECDM_E1       0x02
#define MSQ_CAR_ECDM_E2       0x04
#define MSQ_CAR_ECDM_E3       0x08
#define MSQ_CAR_ECDM_E4       0x10
#define MSQ_CAR_ECDM_DTMF     0x20
#define MSQ_CAR_ECDM_NUM_EXT  0x40  // Tel = numero exterieur

// !!!! E1 à E4 et ii de 0 a 3 pour chaine E commande CFA
#define MSQ_CAR_ECDM_E(ii) (MSQ_CAR_ECDM_E1<<(ii))
#define NB_E_TOR 4

#define NB_LIGNES_TAB_CFA (unsigned int) 10

//EE_SLAVE_ADDR_0
// Page 0
#define ADR_CODE_PIN   (unsigned int)  0                               // 0
#define ADR_NOM_PAU	   (unsigned int)  (ADR_CODE_PIN + SZ_CODE_PIN)    // 4
#define ADR_DTMF_PAU   (unsigned int)  (ADR_NOM_PAU  + SZ_NOM_PAU)     // 24
#define ADR_2O_HTJ     (unsigned int)  (ADR_DTMF_PAU + SZ_DTMF_PAU)    // 27  ( 2 octets HH et MM test audio en eeprom)
#define ADR_DMC_MIN    (unsigned int)  (ADR_2O_HTJ + 2)                // 29  ( DureeMaxComm en minute)

#define ADR_VALID_U    (unsigned int)  (ADR_DMC_MIN + 1)               // 30  ( Champs validé usine)

#define ADR_G_HPN      (unsigned int)  (ADR_VALID_U + 1)                // 31  ( HP Normal
#define ADR_G_MIN      (unsigned int)  (ADR_G_HPN  + 1)                 // 32  ( Micro Normal
#define ADR_G_HPME     (unsigned int)  (ADR_G_MIN  + 1)                 // 33  ( HP mal entendant
#define ADR_G_MIME     (unsigned int)  (ADR_G_HPME + 1)                 // 34  ( Micro mal entendant
#define ADR_G_HPT_lib  (unsigned int)  (ADR_G_MIME + 1)                 // 35  ( HP test  (libre frequence faite par oscillateur, non reglable)
#define ADR_G_MIT_lib  (unsigned int)  (ADR_G_HPT_lib + 1)            // 36  ( Micro test (libre frequence detectee par micro, non reglable)
#define ADR_G_HPDM_lib (unsigned int)  (ADR_G_MIT_lib + 1)            // 37  ( HP diffusion message (libre frequence detectee par micro, non reglable)
#define ADR_TEL_PAU    (unsigned int)  (ADR_G_HPDM_lib  + 1)            // 38  ( Numero de telephone du PAU

#define ADR_VT_PAU     (unsigned int)  (ADR_TEL_PAU  + SZ_NUM_TEL)      // 53 ( Verification telephone '1' ou '0'
#define ADR_DS_PAU     (unsigned int)  (ADR_VT_PAU   + 1)               // 54 ( Duree sommeil en minute) 
#define ADR_NUM_SERIE  (unsigned int)  (ADR_DS_PAU   + 1)               // 55 ( Numero de serie sur 3 octets) 
#define ADR_OPTIONS    (unsigned int)  (ADR_NUM_SERIE + SZ_NUM_SERIE)   // 59 // Option du logiciel B0=Trace modem

   #define MSQ_OPTIONS_TRACE_MODEM     0x01   
   #define MSQ_OPTIONS_REPOND_FAIL     0x02   
   #define MSQ_OPTIONS_GPS_ACTIF       0x04
   #define MSQ_OPTIONS_TRACE_GPS       0x08   // Visu des traces GPS
   #define MSQ_OPTIONS_GPS_ALIM_PERM   0x10   // Alimentation permanente du GPS
   #define MSQ_OPTIONS_TST_AUDIO_AAEC  0x20   // Test audio apres appel entrant

//#define MSQ_OPTIONS_VANA_CHGT_libre    0x20   // Envoi des SMS VANA sur chgt charge Libre, remplace par CFPAU WDVA bit 7
//#define MSQ_OPTIONS_VANA_TEST_libre    0x40   // Envoi des SMS VANA apres test Libre, remplace par CFPAU WDVA bits 0 à 6
   #define MSQ_REBOOT_TEST_ROAMING     0x80   // Reboot apres les tests si roaming


#define ADR_DC_DTMF    (unsigned int)  (ADR_OPTIONS + 1)     // 60   // Durée d'un caractere DTMF en 1/10 sec
#define ADR_INT_TRAIN  (unsigned int)  (ADR_DC_DTMF + 1)     // 61

#define ADR_ECHO_MODEM (unsigned int)  (ADR_INT_TRAIN + 1)    // 62  // Parametre de l'anti echo
#define ADR_SVR_MODEM  (unsigned int)  (ADR_ECHO_MODEM + 1)   // 63

#define PLEIN_PLUS_ADR_libre_1    (unsigned int)  (ADR_SVR_MODEM+ 1)   // 64   !!!! PLEIN

// Page 1
#define ADRESSE_BASE_0_CFA  (unsigned int) 64
#define OFF_CAR_ECDM  (unsigned int)  0 // Adresse de base = caractere ECDM
#define OFF_TPA       (unsigned int)  1 // Adresse 1       = Tempo attente
#define OFF_TYPE_SMS  (unsigned int)  2 // Adresse 2       = type SMS info
#define OFF_NUM_TEL   (unsigned int)  3 // Numero appel audio
#define OFF_NUM_SMS   (unsigned int) (OFF_NUM_TEL+SZ_NUM_TEL) // Numero d'appel SMS (3+15 = 18)
#define OFF_TXT_SMS   (unsigned int) (OFF_NUM_SMS+SZ_NUM_TEL) // texte SMS (18+15 = 33)
#define OFF_MWD_SMST  (unsigned int) (OFF_TXT_SMS+SZ_TXT_SMS_APPEL) // Masque week day envoi des SMS (33+12 = 45)

#define OFF_cfa_libre (unsigned int) (OFF_WD_SMST+1) // Numero d'appel SMS (45+1 = 46)

//SZ_TXT_SMS_APPEL
#define OFFSET_INTER_LIGNE_CFA             (unsigned int) 64  // Intervalle entre 2 enregistrements = Taille d'une page eeprom

// Tableau de config des sorties apres tableau CFA

#define ADR_BASE_CFS  (unsigned int) (ADRESSE_BASE_0_CFA + (NB_LIGNES_TAB_CFA * OFFSET_INTER_LIGNE_CFA))   //704            (Debut de page)
#define OFF_CFS_FIXE  (unsigned int)  0 // Masque fixe
#define OFF_CFS_CLIG  (unsigned int)  1 // Masque clignotant
#define OFF_CFS_INV   (unsigned int)  2 // masque des inversions

#define OFFSET_INTER_LIGNE_CFS        4 // Intervalle entre 2 enregistrements
#define NB_LIGNES_TAB_CFS             NB_SORTIE // Nombre de ligne = Nombre de sorties

// Masque sur la configuration des sorties
#define MSQ_CFS_APPEL         0x01  // Appel en cours
#define MSQ_CFS_PHONIE        0x02  // Phonie en cours
#define MSQ_CFS_DEFAUT        0x04  // Defaut (MotErreur !=0))
#define MSQ_CFS_F1            0x08  // Forcé
#define MSQ_CFS_PAU_DEPLACE   0x10  // Pau deplacé
#define MSQ_CFS_TEST_BP       0x20  // Sortie de commande du bouton testable, activee pendant test audio 
#define MSQ_CFS_RESULT_TEST   0x40  // Clignotement en fin de test suivant resultat
#define MSQ_CFS_TEST_EN_COURS 0x80  // Test en cours


#define ADR_GPS_TLA          (unsigned int)  (ADR_BASE_CFS + (NB_LIGNES_TAB_CFS * OFFSET_INTER_LIGNE_CFS))  // 720
#define ADR_GPS_TLO          (unsigned int)  (ADR_GPS_TLA + 4)   // 724
#define ADR_GPS_REF_LAT      (unsigned int)  (ADR_GPS_TLO + 4)   // 728
#define ADR_GPS_REF_DECI_LAT (unsigned int)  (ADR_GPS_REF_LAT + 4)   // 732
#define ADR_GPS_REF_CAR_LAT  (unsigned int)  (ADR_GPS_REF_DECI_LAT + 4)   // 736

#define ADR_GPS_REF_LON      (unsigned int)  (ADR_GPS_REF_CAR_LAT + 1)   // 737
#define ADR_GPS_REF_DECI_LON (unsigned int)  (ADR_GPS_REF_LON + 4)   // 741
#define ADR_GPS_REF_CAR_LON  (unsigned int)  (ADR_GPS_REF_DECI_LON + 4)   // 745

#define ADR_SERVICE          (unsigned int)  (ADR_GPS_REF_CAR_LON + 1)   // 746
   #define VAL_HORS_SERVICE 0x5A   

#define ADR_GPS_TP_ATTENTE_2S  (unsigned int)  (ADR_SERVICE + 1)   // 747
#define ADR_GPS_NB_MS_IGNO     (unsigned int)  (ADR_GPS_TP_ATTENTE_2S + 1)   // 748
#define ADR_OPERATIONNEL       (unsigned int)  (ADR_GPS_NB_MS_IGNO + 1)   // 749


#define ADR_SEUIL_BAS_CHARGE     (unsigned int) (ADR_OPERATIONNEL         + 1) // 750 Seuil bas de debut de charge en mv (uint16)
#define ADR_SEUIL_HAUT_CHARGE    (unsigned int) (ADR_SEUIL_BAS_CHARGE     + 2) // 752 Seuil haut de fin de charge en mv (uint16)
#define ADR_DELTA_PAN_BAT_CHARGE (unsigned int) (ADR_SEUIL_HAUT_CHARGE    + 2) // 754 Delta min (panneau-bat) pour debut charge en mv (uint16)

#define ADR_MSQ_CHGT_DEF_SMS     (unsigned int) (ADR_DELTA_PAN_BAT_CHARGE + 2) // 756 Masque sur MotErreurPAU pour filtrage defauts envoi SMS chgt etat
#define ADR_MWD_SMS_VERIFNUM     (unsigned int) (ADR_MSQ_CHGT_DEF_SMS + 2)     // 758 Masque sur jour semaine pour test du reseau par envoi d'un SMS a nous même
#define ADR_MWD_SMS_TEST         (unsigned int) (ADR_MWD_SMS_VERIFNUM + 1)     // 759 Masque sur jour semaine pour envoi SMS a nous meme
#define ADR_MWD_SMS_GPS          (unsigned int) (ADR_MWD_SMS_TEST     + 1)     // 760
#define ADR_MWD_SMS_VANA         (unsigned int) (ADR_MWD_SMS_GPS      + 1)     // 761
#define ADR_MAX_SMS_AUTO_JOUR    (unsigned int) (ADR_MWD_SMS_VANA     + 1)     // 762 Nb max de SMS emis par jour à l'initiative de la borne

#define ADR_librelibre           (unsigned int) (ADR_MAX_SMS_AUTO_JOUR+ 1)     // 763

#endif

        

// !! fin de page (derniere adresse de la page)  767 <<<<= ((12*64)-1)
