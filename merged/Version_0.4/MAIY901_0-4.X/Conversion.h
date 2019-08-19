#ifndef CONVERSION_H
#define CONVERSION_H
extern unsigned char StrLength(unsigned char *chaine);
extern unsigned long ConvertChaineEntier(unsigned char* chaine, unsigned char size);
extern unsigned char HexToAsc(char HEXValue);
extern unsigned char AscToHex(char Asc);
extern unsigned char uinttoa(unsigned int Valeur,unsigned char *Out);
extern unsigned char strtouc(unsigned char *In);
extern unsigned char uchartoa(unsigned char Valeur,unsigned char *Out,unsigned char Nb0Format);
extern unsigned char ulongtoa(unsigned long Valeur,unsigned char *Out,unsigned char szout);
unsigned char uchartoHexa(unsigned char Valeur,unsigned char *Out);
unsigned int strtoui(unsigned char *In);
#endif

