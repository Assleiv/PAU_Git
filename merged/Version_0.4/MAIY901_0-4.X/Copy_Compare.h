// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef XC_HEADER_TEMPLATE_H
#define	XC_HEADER_TEMPLATE_H
extern unsigned char StrCmpInBuff(unsigned char*Buff, unsigned char * chain); // équivalent de StrDebute
extern unsigned char StrnCompare(unsigned char*Buff,const char * chain,unsigned char lengt);
extern unsigned char StrCopyToChar(unsigned char *Out,unsigned char *In);
extern unsigned char StrCopyPgmToChar(unsigned char *Out,const char *In);
extern unsigned char isnumber(unsigned char c);
extern unsigned char NumTelValide(unsigned char *NumTel);
#endif	/* XC_HEADER_TEMPLATE_H */

