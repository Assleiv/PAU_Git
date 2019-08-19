#ifndef PAU_PRO_H
#define PAU_PRO_H

extern void pau_init(void);
extern void Positionne8563DepuisEE(void);

/* cmd_traitement.c */
extern void DebutTraceDebugConsole(void);
extern void FabriqueReponseConfSMS(unsigned char *Rs);
extern void TraiteCommande(unsigned char *cmd,unsigned char *numSMS,unsigned char *DateSMS,unsigned char *HeureSMS);
extern unsigned char SMSEnvoi_MsgCFA(unsigned char NumLigne,unsigned char *PhoneNumber,unsigned char *SmsMsgCFA);
extern unsigned char SMSEnvoi(unsigned char *PnoneNumber,unsigned char *SmsText,unsigned char FlagAuto);
extern unsigned char SMSEnvoi_MsgCFA_AvantAppelSortant(unsigned char NumLigne,unsigned char *NumSMS,unsigned char *NumTelAudio);
extern unsigned char SMSEnvoi_MsgCFA_EchecAppelSortant(unsigned char IdxCfa,unsigned char *NumSMS,unsigned char *NumTelAudio,unsigned char NumLigne,unsigned char NbrLigne,unsigned char Err,unsigned char Ext,unsigned char FormatReduit);
extern unsigned char SMSEnvoi_MsgCFA_ApresAppelEntrant(unsigned char NumLigne,unsigned char *NumSMS,unsigned char *NumTelEntrant,unsigned int DureeSec,unsigned char FormatReduit);
extern unsigned char SMSEnvoi_MsgCFA_ApresAppelSortant(unsigned char NumLigne,unsigned char *NumSMS,unsigned char *NumTelSortant,unsigned int DureeSec);
extern unsigned char SMSEnvoi_StatusPau(unsigned char *NumSMS, uint16 MotErreurEmis,char FrtReduit);
extern unsigned char SMSEnvoi_VanaPau(unsigned char *NumSMS);
extern unsigned char SMSEnvoi_MsgCFA_DemandeRappel(unsigned char NumLigne,unsigned char *NumSMS);

      

extern unsigned char NumTelValide(unsigned char *NumTel);
extern unsigned char CodePinValide(unsigned char *CodePin);
extern unsigned char FinNumTelIdentique(unsigned char *Nt1,unsigned char *Nt2);

extern void EmetSMSPortSerie(const char *Entete,unsigned char *Texte,unsigned char *numSMS,unsigned char *DateSMS,unsigned char *HeureSMS);
extern unsigned char StrCopyToChar(unsigned char *Out,unsigned char *In);
extern unsigned char StrCopyPgmToChar(unsigned char *Out,const char *In);
extern unsigned char uchartoa(unsigned char Valeur,unsigned char *Out,unsigned char Nb0Format);
extern unsigned char uinttoa(unsigned int Valeur,unsigned char *Out);
extern unsigned char ulongtoa(unsigned long Valeur,unsigned char *Out,unsigned char szout);
extern unsigned char StrLength(unsigned char *chaine);
extern unsigned char uchartoHexa(unsigned char Valeur,unsigned char *Out);

extern void LectureCFS(unsigned char Ligne);
extern void Traite_VANA(unsigned char *cmd,unsigned char *numSMS);
extern void Traite_GPS(unsigned char *cmd,unsigned char *numSMS);
extern void Traite_GPSLOC(unsigned char *cmd,unsigned char *numSMS);
extern void Traite_DT(unsigned char *cmd,unsigned char *numSMS,char FtAMJ_VT);

// task4
extern unsigned char DemandeAppelVoix(unsigned char *Numero, char EmetDTMF, unsigned char TempoAch, unsigned char Urgent, unsigned char NumLigne, unsigned char NbrLigne, unsigned char Ext, unsigned char Court);
extern void JoueMessageVocal(unsigned char Numero);

// task9
extern void GereGrafFiltrage(unsigned char IdxGraf,unsigned char CopieEntree);

// fnstatus
extern unsigned char FabriqueChaineSorties(unsigned char *Che);
extern unsigned char FabriqueChaineEntrees(unsigned char *Che);
extern void FabriqueReponseStatus(unsigned char *Rs,uint16 MotErreurEmis,char FrtReduit, unsigned char SzBuf);

extern void FabriqueEtEmetReponseStatus(void);
extern void FixeStatus_EtatPau(unsigned char Valeur);
extern void FixeStatus_GsmState(unsigned char Valeur);
extern void FixeStatus_GsmRSSI(unsigned char Valeur);
extern unsigned char ResetBitMotErreur(uint16 Masque,unsigned char EmetSiModif);
extern unsigned char SetBitMotErreur(uint16 Masque,unsigned char EmetSiModif);

extern void FabriqueReponseVANA(unsigned char *Rs);
extern void FabriqueEtEmetReponseVANA(void);

//FnGSM
extern void AjouteTraceCharTxGSM(unsigned char cm);
extern void AjouteTraceCharRxGSM(unsigned char cm);

// task 12
extern unsigned char EnvoiSmsAutoAutorise(char TraceDepassement);
extern unsigned char FabriqueChaineValeurEcho(unsigned char *Buffer);
extern void ReceptionCaractereGSM(unsigned char Car);
extern unsigned char GestionTimeOutModem(unsigned char ToDectecte);

// task 13
extern unsigned char EcritGPS(unsigned char *Trame);
extern unsigned char DemandeAcquisitionGPS(unsigned char Demamdeur);
extern char CoupureGPS(void);
extern unsigned long DegreMinuteEnMinute(unsigned long DegMin);
extern unsigned long MaxMoinsMin(unsigned long v1,unsigned long v2);

extern unsigned char ExtraitLongChaine(unsigned char *Chaine,unsigned long *Valeur,unsigned char LongMax,unsigned char LongMin);
extern unsigned long DeltaLatLongTropGrand(unsigned char MemeCote,unsigned long ValEntiere,unsigned long RefEntiere,unsigned long ValDeci,unsigned long RefDeci);


// task 2
extern unsigned char FaitBip(unsigned char NbBip,unsigned char BipLong);

#endif