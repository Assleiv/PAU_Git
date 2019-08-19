
#ifndef CMD_TRAITEMENT_H
#define	CMD_TRAITEMENT_H


#define SET_MSQ(c,msq) (c)|=(msq) // positionne le bit b de l'octet p à 1
#define RESET_MSQ(c,msq) (c)&=~(msq) // positionne le bit b de l'octet p à 1
#define SZ_NUM_TEL ((unsigned char) 15)
extern int ValeurVariableLCR(unsigned char *ChaineRecue,unsigned char* NomVar,unsigned char *ValVar,int SzVar);
extern unsigned char CodePinValide(unsigned char *CodePin);

/*
unsigned char DecodeChaine(unsigned char *cmd, unsigned char sep, unsigned char *data, unsigned char *stop)
Fonction destinée à simplifier le traitement d'une commande DT
Extrait une valeur numerique d'une chaine jusqu'a la fin de cette chaine ou un séparateur
ENTREE : cmd  = pointeur sur la chaine a traiter
         sep  = caractère séparateur
         
SORTIE : data = Valeur numerique representé par la chaine
         stop = décalage entre la chaine cmd d'entree et le sparateur si existant (Retour = 1)

Retour   1 si séparateur ou fin de chaine atteind
         0 si caractère non numérique et different du séparateur rencontré
*/
extern unsigned char DecodeChaine(unsigned char *cmd, unsigned char sep, unsigned char *data, unsigned char *stop);

#endif	/* XC_HEADER_TEMPLATE_H */

