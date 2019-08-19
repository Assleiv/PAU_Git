/*
 * File:   Copy_Compare.c
 * Author: clement
 *
 * Created on 9 août 2019, 09:08
 */


#include "xc.h"
#include"Copy_Compare.h"
#include"Conversion.h"
#include"Recherche.h"



//Compare a string inflash with a buffer in ram
unsigned char StrCmpInBuff(unsigned char*Buff, unsigned char * chain) // équivalent de StrDebute
    {
        unsigned char lengt = 0;
        unsigned char i = 0;
        lengt = StrLength(chain);
        for (i = 0; i < lengt; i++) 
            {
                if (Buff[i] != chain[i])
                    return 0;
            }
        return 1;
    }


unsigned char StrnCompare(unsigned char*Buff,const char * chain,unsigned char lengt)
    {
        unsigned char i;
        for (i=0;i<lengt;i++)
            {
                if(Buff[i]!=chain[i])return 0;
            }
        return 1;
    }

unsigned char StrCopyToChar(unsigned char *Out,unsigned char *In)
    {
        unsigned char *PtOut;
        unsigned char *PtIn;
        unsigned char Retour;


        PtIn=In;
        PtOut=Out;
        Retour=0;
        while( *PtIn )
            {
                *PtOut=*PtIn;
                PtIn++;
                PtOut++;
                Retour++;
            }
        *PtOut=0;
        return(Retour);
    }

unsigned char StrCopyPgmToChar(unsigned char *Out,const char *In)
    {
        unsigned char Tmp;
        unsigned char *PtOut;
        const char *PtIn;
        unsigned char Retour;


        PtIn=In;
        PtOut=Out;
        Retour=0;
        while( *PtIn )
            {
                Tmp=(unsigned char)*PtIn;
                *PtOut=Tmp;
                PtIn++;
                PtOut++;
                Retour++;
            }
        *PtOut=0;
        return(Retour);
    }
/*
unsigned char isdigit(unsigned char c)
Retourne 1 si le caractère entré est numérique
ENTREE : c = Caractère a tester
SORTIE : Aucune
RETOUR : 1 si caractère numérique
         0 sinon
*/

unsigned char isdigit(unsigned char c)
{
if((c>='0')&&(c<='9'))
  return 1;
else
  return 0;
}

/*
unsigned char NumTelValide(unsigned char *NumTel)
Retourne une valeur differente de 0 si la chaine recue represente un numero de tel valide.
ENTREE : NumTel  = Numero de téléphone a analyser
SORTIE : Aucune
RETOUR : 0=numero non valide, !=0 = numero valide
Au minimum 1 chiffre pouvant etre precede par un +
*/

unsigned char NumTelValide(unsigned char *NumTel)
{
unsigned char Retour;

Retour=0;

// Accepte un + en debut de chaine
if((*NumTel)=='+') NumTel++;

while(isdigit(*NumTel))
  {
  NumTel++;
  Retour++;
  }

if(*NumTel!=0)
  Retour=0;
return(Retour);
}