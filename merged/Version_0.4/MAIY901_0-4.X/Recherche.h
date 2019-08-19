#ifndef XC_HEADER_TEMPLATE_H
#define	XC_HEADER_TEMPLATE_H
extern unsigned char FindFirstSTRInBuff (unsigned char*Buff,unsigned char* ChaineCherchee, unsigned char* result);
extern unsigned char FindFirstInBuff(unsigned char*Buff,unsigned char CLookFor, unsigned char * result);
unsigned char ExtraitLongChaine(unsigned char *Chaine,unsigned long *Valeur,unsigned char LongMax,unsigned char LongMin);
unsigned char FinNumTelIdentique(unsigned char *Nt1,unsigned char *Nt2);
#endif	/* XC_HEADER_TEMPLATE_H */

