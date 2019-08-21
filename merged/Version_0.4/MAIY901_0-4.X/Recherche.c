/*
 * File:   Recherche.c
 * Author: cleme
 *
 * Created on 9 août 2019, 09:04
 */


#include"xc.h"
#include"Copy_Compare.h"
#include"Recherche.h"
#include"Conversion.h"
#include"ApiGsm.h"

unsigned char FindFirstInBuff(unsigned char*Buff,unsigned char CLookFor, unsigned char * result)
    {
        unsigned char ind=0;

        while(Buff[ind])
            {
                if(Buff[ind]==CLookFor)
                    {
                        *result=ind;
                        return 1;
                    }
                ind++;
            }
        return 0;
    }

unsigned char FindFirstSTRInBuff (unsigned char*Buff,unsigned char* ChaineCherchee, unsigned char* result)
        {
            unsigned char IndInst;
            unsigned char ResultRech;
            unsigned char CarDeb;


            IndInst=0;
            ResultRech=0;
            CarDeb=ChaineCherchee[0];

            while(1) // FindFistSTRInBuff
                {
                    if(FindFirstInBuff(&Buff[IndInst],CarDeb,&ResultRech)==1)
                        { 
                            if(StrCmpInBuff(&Buff[IndInst+ResultRech],ChaineCherchee)==1)
                                {
                                    *result=IndInst+ResultRech;
                                    return 1;
                                }
                        }
                    else
                        { 
                            return 0;
                        }
                    IndInst+=ResultRech;
                    IndInst++;
              }
            return 0;
        }

 /*
unsigned char ExtraitLongChaine(unsigned char *Chaine,unsigned long *Valeur,unsigned char LongMax,unsigned char LongMin)

Extrait d'une chaine de caracteres la valeur d'un entier long

ENTREE
  Chaine   : Pointeur sur le debut de l'entier en litteral
  *Valeur  : Pointeur pour retourner la valeur    
  LongMax  : Nombre de caracteres max a prendre en compte
  LongMin  : Longueur mini de la valeur retournee une fois reconvertie en litteral (permet de travailler en longueur fixe apres la virgule)
SORTIE
  *Valeur : Pointeur pour retourner la valeur    
RETOUR
  Nombre de caracteres lus dans la chaine
*/
unsigned char ExtraitLongChaine(unsigned char *Chaine,unsigned long *Valeur,unsigned char LongMax,unsigned char LongMin)
{
unsigned char Offset;
unsigned char MemOffset;

Offset=0;
*Valeur=0;
while((Chaine[Offset]>='0')&&(Chaine[Offset]<='9'))
  {
  if(Offset<LongMax)
    {
    *Valeur=*Valeur*10;
    *Valeur=*Valeur+(Chaine[Offset]-'0');
    }
  Offset+=1;
  }
MemOffset=Offset;
// Formattage si necessaire pour partie decimale
while(Offset<LongMin)
  {
  *Valeur=*Valeur*10;
  Offset+=1;
  }

return(MemOffset);
}

/*
unsigned char FinNumTelIdentique(unsigned char *Nt1,unsigned char *Nt2)
Retourne le nombre de caracteres egaux, en partant de la fin
ENTREE : Nt1  = Premier numero a comparer
         Nt2  = Deuxième numero a comparer
SORTIE : Aucune
RETOUR : 1+Nombre de caracteres egaux en partant de la fin (! +1 car compare les 2 zero terminaux)
*/
unsigned char FinNumTelIdentique(unsigned char *Nt1,unsigned char *Nt2)
{
unsigned char Retour;
unsigned char *Ptf1;
unsigned char *Ptf2;

// Positionne les pointeurs sur fin de chaine
Ptf1=Nt1;
while(*Ptf1) Ptf1++;
Ptf2=Nt2;
while(*Ptf2) Ptf2++;

Retour=0;
while((*Ptf1==*Ptf2)&&(Ptf1>=Nt1)&&(Ptf2>=Nt2))
  {
  Retour++;
  Ptf1--;
  Ptf2--;
  }
return(Retour);
}

unsigned char GetGsmMode(void)
{
//Cas mode NORMAL
if(GsmModeFred.RequestSleep==0 && GsmModeFred.RequestOff==0)return GSM_MODE_NORMAL;
//Cas mode OFF
if(GsmModeFred.RequestSleep==0 && GsmModeFred.RequestOff==1)return GSM_MODE_OFF;
//Cas mode SLEEP
if(GsmModeFred.RequestSleep==1 && GsmModeFred.RequestOff==0)return GSM_MODE_SLEEP;

//Cas ne devant pas se produire (sleep et off en meme temps)
if(GsmModeFred.RequestSleep==1 && GsmModeFred.RequestOff==1)
  {
  SetGsmMode(GSM_MODE_SLEEP);
  return GSM_MODE_SLEEP;
  }
return 0;
}


void SetGsmMode(unsigned char mode)
{
switch (mode)
    {
        case GSM_MODE_NORMAL :
          GsmModeFred.RequestSleep=0;
          GsmModeFred.RequestOff=0;
          break;
        case GSM_MODE_SLEEP :
          GsmModeFred.RequestSleep=1;
          GsmModeFred.RequestOff=0;
          break;
        case GSM_MODE_OFF :
          GsmModeFred.RequestSleep=0;
          GsmModeFred.RequestOff=1;
          break;
    }
}