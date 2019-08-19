// <editor-fold defaultstate="collapsed" desc="Config bit">
// FSEC
#pragma config BWRP = OFF    //Boot Segment Write-Protect bit->Boot Segment may be written
#pragma config BSS = DISABLED    //Boot Segment Code-Protect Level bits->No Protection (other than BWRP)
#pragma config BSEN = OFF    //Boot Segment Control bit->No Boot Segment
#pragma config GWRP = OFF    //General Segment Write-Protect bit->General Segment may be written
#pragma config GSS = DISABLED    //General Segment Code-Protect Level bits->No Protection (other than GWRP)
#pragma config CWRP = OFF    //Configuration Segment Write-Protect bit->Configuration Segment may be written
#pragma config CSS = DISABLED    //Configuration Segment Code-Protect Level bits->No Protection (other than CWRP)
#pragma config AIVTDIS = OFF    //Alternate Interrupt Vector Table bit->Disabled AIVT
// FBSLIM
#pragma config BSLIM = 8191    //Boot Segment Flash Page Address Limit bits->8191
// FOSCSEL
#pragma config FNOSC = FRCPLL    //Oscillator Source Selection->Fast RC Oscillator with divide-by-N with PLL module (FRCPLL) 
#pragma config PLLMODE = PLL96DIV2    //PLL Mode Selection->96 MHz PLL. (8 MHz input)
#pragma config IESO = OFF    //Two-speed Oscillator Start-up Enable bit->Start up with user-selected oscillator source
// FOSC
#pragma config POSCMD = NONE    //Primary Oscillator Mode Select bits->Primary Oscillator disabled
#pragma config OSCIOFCN = OFF    //OSC2 Pin Function bit->OSC2 is clock output
#pragma config SOSCSEL = OFF    //SOSC Power Selection Configuration bits->Digital (SCLKI) mode
#pragma config PLLSS = PLL_FRC    //PLL Secondary Selection Configuration bit->PLL is fed by the on-chip Fast RC (FRC) oscillator
#pragma config IOL1WAY = OFF    //Peripheral pin select configuration bit->not Allow only one reconfiguration
#pragma config FCKSM = CSECMD    //Clock Switching Mode bits->Clock switching is enabled,Fail-safe Clock Monitor is disabled
// FWDT
#pragma config WDTPS = PS32768    //Watchdog Timer Postscaler bits->1:32768
#pragma config FWPSA = PR128    //Watchdog Timer Prescaler bit->1:128
#pragma config FWDTEN = OFF    //Watchdog Timer Enable bits->WDT and SWDTEN disabled
#pragma config WINDIS = OFF    //Watchdog Timer Window Enable bit->Watchdog Timer in Non-Window mode
#pragma config WDTWIN = WIN25    //Watchdog Timer Window Select bits->WDT Window is 25% of WDT period
#pragma config WDTCMX = WDTCLK    //WDT MUX Source Select bits->WDT clock source is determined by the WDTCLK Configuration bits
#pragma config WDTCLK = LPRC    //WDT Clock Source Select bits->WDT uses LPRC
// FPOR
#pragma config BOREN = ON    //Brown Out Enable bit->Brown Out Enable Bit
#pragma config LPCFG = OFF    //Low power regulator control->No Retention Sleep
#pragma config DNVPEN = ENABLE    //Downside Voltage Protection Enable bit->Downside protection enabled using ZPBOR when BOR is inactive
// FICD
#pragma config ICS = PGD1    //ICD Communication Channel Select bits->Communicate on PGEC1 and PGED1
#pragma config JTAGEN = OFF    //JTAG Enable bit->JTAG is disabled
#pragma config BTSWP = OFF    //BOOTSWP Disable->BOOTSWP instruction disabled
// FDEVOPT1
#pragma config ALTCMPI = DISABLE    //Alternate Comparator Input Enable bit->C1INC, C2INC, and C3INC are on their standard pin locations
#pragma config TMPRPIN = OFF    //Tamper Pin Enable bit->TMPRN pin function is disabled
#pragma config SOSCHP = ON    //SOSC High Power Enable bit (valid only when SOSCSEL = 1->Enable SOSC high power mode (default)
#pragma config ALTVREF = ALTREFEN    //Alternate Voltage Reference Location Enable bit->VREF+ and CVREF+ on RA10, VREF- and CVREF- on RA9
// FBOOT
#pragma config BTMODE = SINGLE    //Boot Mode Configuration bits->Device is in Single Boot (legacy) mode
// </editor-fold>
// <editor-fold defaultstate="collapsed" desc="includes">
#include "xc.h"
#include "Noyau.h"
#include "0_Main.h"
#include "1_Gsm.h"
#include "2_Leds.h"
#include "3_Chrg.h"
#include "4_Alims.h"
#include "5_IoExt.h"
#include "6_IcsI2c.h"
#include "7_Com485.h"
#include "8_Com232.h"
#include "9_Audio.h"
// </editor-fold>
//variables globales
// <editor-fold defaultstate="collapsed" desc="">
volatile STR_TASK TASK[NB_TASK];
// </editor-fold>
//variables locales au fichier
// <editor-fold defaultstate="collapsed" desc="">
char d;
//volatile unsigned int timer0ReloadVal16bit; // variable permettant de recharger le timer du noyau
// </editor-fold>

void __attribute__ ( ( interrupt, no_auto_psv ) ) _T1Interrupt (  ) //cadenseur du noyau
{// <editor-fold defaultstate="collapsed" desc="">
    if (IEC0bits.T1IE == 1 && IFS0bits.T1IF == 1)
    {
        //Decrementation du temps de delay pour les taches en sommeil
        volatile unsigned char z;
        for (z=0;z<NB_TASK;z++)
        {
            if (TASK[z].STATE==WAIT)
            {
                if(TASK[z].DELAY>0)
                    TASK[z].DELAY=TASK[z].DELAY-1;
                if (TASK[z].DELAY==0)
                {
                    TASK[z].STATE=READY;
                }
            }
        }
        for(z=0;z<IndGsmReq;z++)    //décrémentation des timeout des requêtes en cours
        {
            if(GsmReq[z].Timeout>0)
            {
                GsmReq[z].Timeout--;
                if(GsmReq[z].Timeout==0)
                {
                    GsmState=GSM_NO_RESPONSE;
                }
            }
        }
        if(TimeOutI2c>0)    //gestion blocage I2C
        {
            TimeOutI2c--;
        }
    } 
    IFS0bits.T1IF = 0;  //clear flag
}// </editor-fold>

void __attribute__ ( ( interrupt, no_auto_psv ) ) _U2RXInterrupt( void )
{
    if(U2STAbits.URXDA==1)
    {
        d=U2RXREG;              // Echo console pour test
        if (d!=0)
            U2TXREG=d;
    }
}

void __attribute__ ( ( interrupt, no_auto_psv ) ) _U1RXInterrupt( void )    //Reception sur l'uart1 (GSM)
{// <editor-fold defaultstate="collapsed" desc="">
    if (IEC0bits.U1RXIE == 1 && IFS0bits.U1RXIF == 1)
    {
        while((U1STAbits.URXDA == 1))
        {
            ReceiveUart();
    //        *uart1_obj.rxTail = U1RXREG;
    //        uart1_obj.rxTail++;
    //        if(uart1_obj.rxTail == (uart1_rxByteQ + UART1_CONFIG_RX_BYTEQ_LENGTH))
    //        {
    //            uart1_obj.rxTail = uart1_rxByteQ;
    //        }
    //        uart1_obj.rxStatus.s.empty = false;
    //        if(uart1_obj.rxTail == uart1_obj.rxHead)
    //        {
    //            //Sets the flag RX full
    //            uart1_obj.rxStatus.s.full = true;
    //            break;
    //        }
        }
    }
    IFS0bits.U1RXIF = 0;
}// </editor-fold>


/*******************************************************************************
*                               PROGRAMME                                      *
*******************************************************************************/
int main(void)
{
//init du microcontrôleur
    //Init_Osc();
    Init_Pins();
    Init_Interrupt();
    Init_Adc();
    Init_Tasks();
//séquenceur
    while (1)
    {
        if (TASK[TASK0_ID].STATE==READY) TASK0();
        if (TASK[TASK1_ID].STATE==READY) TASK1();
        if (TASK[TASK2_ID].STATE==READY) TASK2();
        if (TASK[TASK3_ID].STATE==READY) TASK3();
        if (TASK[TASK4_ID].STATE==READY) TASK4();
        if (TASK[TASK5_ID].STATE==READY) TASK5();
        if (TASK[TASK6_ID].STATE==READY) TASK6();
        if (TASK[TASK7_ID].STATE==READY) TASK7();
        if (TASK[TASK8_ID].STATE==READY) TASK8();
        if (TASK[TASK9_ID].STATE==READY) TASK9();
//            CLRWDT();
    }
    return 1;
}

/*******************************************************************************
*                               FONCTIONS                                      *
*******************************************************************************/
// <editor-fold defaultstate="collapsed" desc="">
void SET_READY(unsigned char taskID)
{
	TASK[taskID].STATE=READY;
}
void SET_STOP(unsigned char taskID)
{
	TASK[taskID].STATE=HALTE;
}
void SET_WAIT(unsigned char taskID,unsigned int y)
{
	TASK[taskID].DELAY=y;
	TASK[taskID].STATE=WAIT;
}
void Init_Pins(void) // initialisation des I/O microcontrôleur
{
    /****************************************************************************
     * Setting the Output Latch SFR(s)
     ***************************************************************************/
    LATB = 0x0804;
    LATC = 0x0000;
    LATD = 0x1F07;
    LATE = 0x0004;
    LATF = 0x007C;
    LATG = 0x0180;
    /****************************************************************************
     * Setting the GPIO Direction SFR(s)
     ***************************************************************************/
    TRISB = 0x3F3;
    TRISC = 0x9FFF;
    TRISD = 0xF8F4;
    TRISE = 0xFFF5;
    TRISF = 0xFFFF;
    TRISG = 0xFEBF;
    /****************************************************************************
     * Setting the Weak Pull Up and Weak Pull Down SFR(s)
     ***************************************************************************/
    IOCPDB = 0x0000;
    IOCPDC = 0x0000;
    IOCPDD = 0x0000;
    IOCPDE = 0x0000;
    IOCPDF = 0x0000;
    IOCPDG = 0x0000;
    IOCPUB = 0x0000;
    IOCPUC = 0x0000;
    IOCPUD = 0x0000;
    IOCPUE = 0x0004;
    IOCPUF = 0x0000;
    IOCPUG = 0x0000;
    /****************************************************************************
     * Setting the Open Drain SFR(s)
     ***************************************************************************/
    ODCB = 0x0000;
    ODCC = 0x0000;
    ODCD = 0x0000;
    ODCE = 0x0000;
    ODCF = 0x0000;
    ODCG = 0x0000;
    /****************************************************************************
     * Setting the Analog/Digital Configuration SFR(s)
     ***************************************************************************/
    ANSB = 0x03F0;
    ANSC = 0x0000;
    ANSD = 0x0000;
    ANSE = 0x0000;
    ANSG = 0x0000;
    /****************************************************************************
     * Set the PPS
     ***************************************************************************/
    __builtin_write_OSCCONL(OSCCON & 0xbf); // unlock PPS
    RPOR9bits.RP19R = 3;        //RG8->UART1:U1TX
    RPINR18bits.U1RXR = 26;     //RG7->UART1:U1RX
    RPOR5bits.RP11R = 19;       //RD0->UART3:U3TX
    RPINR17bits.U3RXR = 12;     //RD11->UART3:U3RX
    RPOR12bits.RP24R = 5;       //RD1->UART2:U2TX
    RPINR19bits.U2RXR = 23;     //RD2->UART2:U2RX
    __builtin_write_OSCCONL(OSCCON | 0x40); // lock PPS
}
void Init_Interrupt(void)    // initialisation des interruptions
{
    //TIMER1:
    IPC0bits.T1IP = 1;  //PRIORITY 1
    TMR1 = 0x00;
    PR1 = 0x3E7F;   //Period = 0.001 s; Frequency = 16000000 Hz; PR1 15999; 
    T1CON = 0x8000; //TCKPS 1:1; TON enabled; TSIDL disabled; TCS FOSC/2; TECS SOSC; TSYNC disabled; TGATE disabled; 
    IFS0bits.T1IF = 0;
    IEC0bits.T1IE = 1;
    //UART1:
    IPC16bits.U1ERIP = 1;   //UART1 Error PRIORITY 1
    IPC3bits.U1TXIP = 1;    //UART1 Transmitter PRIORITY 1
    IPC2bits.U1RXIP = 1;//UART1 Receiver PRIORITY 1
}
void Init_Tasks(void)    //Configuration parametres taches au demarrage
{
    TASK[TASK0_ID].STATE=READY;TASK[TASK0_ID].DELAY=0;TASK[TASK0_ID].STEP=0;//TASK[TASK0_ID].PHASE=AMBRION;
	TASK[TASK1_ID].STATE=HALTE;TASK[TASK1_ID].DELAY=0;TASK[TASK1_ID].STEP=0;TASK[TASK1_ID].PHASE=AMBRION;
    TASK[TASK2_ID].STATE=HALTE;TASK[TASK2_ID].DELAY=0;TASK[TASK2_ID].STEP=0;TASK[TASK2_ID].PHASE=AMBRION;
    TASK[TASK3_ID].STATE=HALTE;TASK[TASK3_ID].DELAY=0;TASK[TASK3_ID].STEP=0;TASK[TASK3_ID].PHASE=AMBRION;
    TASK[TASK4_ID].STATE=HALTE;TASK[TASK4_ID].DELAY=0;TASK[TASK4_ID].STEP=0;TASK[TASK4_ID].PHASE=AMBRION;
    TASK[TASK5_ID].STATE=HALTE;TASK[TASK5_ID].DELAY=0;TASK[TASK5_ID].STEP=0;TASK[TASK5_ID].PHASE=AMBRION;
    TASK[TASK6_ID].STATE=READY;TASK[TASK6_ID].DELAY=0;TASK[TASK6_ID].STEP=0;TASK[TASK6_ID].PHASE=AMBRION;
    TASK[TASK7_ID].STATE=HALTE;TASK[TASK7_ID].DELAY=0;TASK[TASK7_ID].STEP=0;TASK[TASK7_ID].PHASE=AMBRION;
    TASK[TASK8_ID].STATE=READY;TASK[TASK8_ID].DELAY=0;TASK[TASK8_ID].STEP=0;TASK[TASK8_ID].PHASE=AMBRION;
    TASK[TASK9_ID].STATE=HALTE;TASK[TASK9_ID].DELAY=0;TASK[TASK9_ID].STEP=0;TASK[TASK9_ID].PHASE=AMBRION;
}
void Init_Adc(void)
{
    AD1CON1 = 0x8070;   //0x8000: ASAM disabled; DMABM disabled; ADSIDL disabled; DONE disabled; DMAEN disabled; FORM Absolute decimal result, unsigned, right-justified; SAMP disabled; SSRC Clearing sample bit ends sampling and starts conversion; MODE12 10-bit; ADON enabled;
    AD1CON2 = 0x0000;   //0x0004 CSCNA disabled; NVCFG0 AVSS; PVCFG AVDD; ALTS disabled; BUFM disabled; SMPI 1; BUFREGEN disabled;
    AD1CON3 = 0x0A30;   // SAMC 0; EXTSAM disabled; PUMPEN disabled; ADRC FOSC/2; ADCS 0;
    AD1CHS = 0x0006;    // CH0SA AN0; CH0SB AN0; CH0NB AVSS; CH0NA AVSS;
    AD1CSSH = 0x0000;   // CSS25 disabled; CSS24 disabled; CSS23 disabled; CSS22 disabled; CSS21 disabled; CSS20 disabled; CSS30 disabled; CSS19 disabled; CSS18 disabled; CSS29 disabled; CSS17 disabled; CSS28 disabled; CSS16 disabled;
    AD1CSSL = 0x0000;   // CSS9 disabled; CSS8 disabled; CSS7 disabled; CSS6 disabled; CSS5 disabled; CSS4 disabled; CSS3 disabled; CSS2 disabled; CSS15 disabled; CSS1 disabled; CSS14 disabled; CSS0 disabled; CSS13 disabled; CSS12 disabled; CSS11 disabled; CSS10 disabled;
    AD1CHITH = 0x0000;  // CHH20 disabled; CHH22 disabled; CHH21 disabled; CHH24 disabled; CHH23 disabled; CHH25 disabled; CHH17 disabled; CHH16 disabled; CHH19 disabled; CHH18 disabled;
    AD1CTMENH = 0x0000; // CTMEN23 disabled; CTMEN24 disabled; CTMEN21 disabled; CTMEN22 disabled; CTMEN20 disabled; CTMEN18 disabled; CTMEN19 disabled; CTMEN16 disabled; CTMEN17 disabled; CTMEN25 disabled;
    ANCFG = 0x0000;     // VBGADC disabled; VBGUSB disabled; VBGEN disabled; VBGCMP disabled;
}
unsigned int ReadAdc(unsigned char channel)
{
    AD1CHS = channel;       //channel select
//    AD1CON1bits.ADON=1;
    AD1CON1bits.SAMP = 1;   //start sample
    Nop();
//    AD1CON1bits.SAMP = 0;   //stop sample
    while (!AD1CON1bits.DONE)   //attente fin de conversion
    {
    }
    return ADC1BUF0;
}
// </editor-fold>
/**
 End of File
 */