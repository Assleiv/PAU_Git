/*
 * File:   Conversion.c
 * Author: clement 
 *
 * Created on 9 août 2019, 08:45
 */


#include"xc.h"
#include"Recherche.h"
#include"Copy_Compare.h"
#include"Conversion.h"


unsigned char strtouc(unsigned char *In)
        {
            unsigned char *PtIn;
            unsigned char Retour;

            Retour=0;
            PtIn=In;
            while((*PtIn>='0')&&(*PtIn<='9'))
                {
                    Retour=Retour*10;
                    Retour=Retour+((*PtIn)-'0');
                    PtIn++;
                }

            return(Retour);
        }

unsigned char uinttoa(unsigned int Valeur,unsigned char *Out)
            {
                unsigned char *OutIn;
                unsigned char Retour;

                OutIn=Out;
                if(Valeur>9999)
                    {
                        *Out='0'+(Valeur/10000);
                        Out++;
                        Valeur=Valeur%10000;
                    }

                if(Valeur>999)
                    {
                        *Out='0'+(Valeur/1000);
                        Out++;
                        Valeur=Valeur%1000;
                    }
                else
                    {
                        if(OutIn!=Out)
                            {
                                *Out='0';
                                Out++;
                            }
                    }
                if(Valeur>99)
                    {
                        *Out='0'+(Valeur/100);
                        Out++;
                        Valeur=Valeur%100;
                    }
                else
                    {
                        if(OutIn!=Out)
                            {
                                *Out='0';
                                Out++;
                            }
                    }

                if(Valeur>9)
                    {
                        *Out='0'+(Valeur/10);
                        Out++;
                    }
                else
                    {
                        if(OutIn!=Out)
                            {
                                *Out='0';
                                Out++;
                            }
                    } 

                Valeur=Valeur%10;
                *Out='0'+Valeur;
                Out++;
                *Out=0;
                Retour=(Out-OutIn);
                return(Retour);
        }

unsigned char AscToHex(char Asc)
        {
        /*
            switch(Asc)
            {
                case '0':return 0x00;
                case '1':return 0x01;
                case '2':return 0x02;
                case '3':return 0x03;
                case '4':return 0x04;
                case '5':return 0x05;
                case '6':return 0x06;
                case '7':return 0x07;
                case '8':return 0x08;
                case '9':return 0x09;
                case 'A':return 0x0A;
                case 'B':return 0x0B;
                case 'C':return 0x0C;
                case 'D':return 0x0D;
                case 'E':return 0x0E;
                case 'F':return 0x0F;
                default:
                    return 0x00;
            }	
        */
        if((Asc>='0')&&(Asc<='9'))
            {
                return(Asc-'0');
            }
        //else (inutile car return, gain de place memoire programme)  
        if((Asc>='A')&&(Asc<='F'))
            {
                return(Asc-'A'+0x0A);
            }
          //else (inutile car return, gain de place memoire programme)
        return(0x00);
        }

unsigned char HexToAsc(char HEXValue)
    {
        /* modifié v151210 pour gain de place

            switch(HEXValue)
            {

                case 0x00:return '0';
                case 0x01:return '1';
                case 0x02:return '2';
                case 0x03:return '3';
                case 0x04:return '4';
                case 0x05:return '5';
                case 0x06:return '6';
                case 0x07:return '7';
                case 0x08:return '8';
                case 0x09:return '9';
                case 0x0A:return 'A';
                case 0x0B:return 'B';
                case 0x0C:return 'C';
                case 0x0D:return 'D';
                case 0x0E:return 'E';
                case 0x0F:return 'F';
                default:
                    return '0';
            }	
        */
        if((HEXValue>=0x00)&&(HEXValue<=0x09))
            {
                return(HEXValue+'0');
            }
        if((HEXValue>=0x0A)&&(HEXValue<=0x0F))
            {
                return(HEXValue-0x0A+'A');
            }          
        return('0');   
    }

unsigned long ConvertChaineEntier(unsigned char* chaine, unsigned char size) 
        {
            unsigned char w;
            unsigned char x;
            unsigned char y;
            unsigned long z;
            unsigned long resultat = 0;
            for (x = 0; x < size; x++) 
                {
                    y = chaine[x] - 48;
                    z = 1;
                    for (w = 1; w < (size - x); w++) //calcul puissance
                        {
                            z *= 10;
                        }
                    resultat += (z * (unsigned long) y);
                }
            return resultat;
        }

unsigned char StrLength(unsigned char *chaine)
    {
        unsigned char Retour;

        Retour=0;
        if(chaine!=0)
            {
                while(chaine[Retour])
                    {
                        Retour++;
                    }
            }
        return(Retour);
    }

unsigned char uchartoa(unsigned char Valeur,unsigned char *Out,unsigned char Nb0Format)
{
unsigned char *OutIn;
unsigned char Retour;


OutIn=Out;
if((Valeur>99)||(Nb0Format>2))
  {
  *Out='0'+(Valeur/100);
  Out++;
  Valeur=Valeur%100;
  }

if(Valeur>9)
  {
  *Out='0'+(Valeur/10);
  Out++;
  }
else
  {
  if((OutIn!=Out)||(Nb0Format>1))
    {
    *Out='0';
    Out++;
    }
  }
Valeur=Valeur%10;
*Out='0'+Valeur;
Out++;
*Out=0;
Retour=(Out-OutIn);
return(Retour);
}

unsigned char ulongtoa(unsigned long Valeur,unsigned char *Out,unsigned char szout)
{
unsigned char IdxEcrit;
unsigned char Idx;
unsigned long ValeurMod10;

if(Valeur)
  {
  IdxEcrit=szout-1;
  Out[IdxEcrit]=0;
  while((IdxEcrit) && (Valeur))
    {
    IdxEcrit-=1;
    ValeurMod10=Valeur % 10;
    Out[IdxEcrit]='0'+ValeurMod10;
    Valeur=Valeur/10;
    }

  // recopie en debut de buffer
  if(IdxEcrit)
    {
    Idx=0;
    while(Out[IdxEcrit])
      {
      Out[Idx]=Out[IdxEcrit];
      Idx+=1;
      IdxEcrit+=1;
      }
    Out[Idx]=0;
    return(Idx);
    }
  else
    {
    return(szout);
    }
  }
else
  {
  Out[0]='0';
  Out[1]=0;
  return(1);
  }
}
/*
unsigned char uchartoHexa(unsigned char Valeur,unsigned char *Out)
Ecrit la valeur d'un uchar en hexa dans une chaine de caracteres
ENTREE : Valeur    = Valeur a convertir
         Out       = Pointeur sur buffer de reception (3 car)
SORTIE : Out
RETOUR : Nombre de caracteres positionnés dans out
*/
unsigned char uchartoHexa(unsigned char Valeur,unsigned char *Out)
{
unsigned char PFort;
unsigned char PFaible;

PFaible=Valeur & 0x0F;
PFort=Valeur & 0xF0;
PFort=PFort>>4;
Out[0]=HexToAsc(PFort);
Out[1]=HexToAsc(PFaible);
Out[2]=0;


return(2);
}

/*
unsigned int strtoui(unsigned char *In)
Converti une chaine de caractere numerique en unsigned int
ENTREE : in  = Pointeur sur entree
SORTIE : Aucune
RETOUR : Valeur de l'entree
*/
unsigned int strtoui(unsigned char *In)
{
unsigned char *PtIn;
unsigned int Retour;

Retour=0;
PtIn=In;
while((*PtIn>='0')&&(*PtIn<='9'))
  {
  Retour=Retour*10;
  Retour=Retour+((*PtIn)-'0');
  PtIn++;
  }

return(Retour);
}