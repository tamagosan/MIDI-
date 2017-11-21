#include <xc.h>
#include <math.h>
#define _XTAL_FREQ 32000000

#define OUT RA2
#define LED RA3

#pragma config FOSC = INTOSC
#pragma config WDTE = OFF
#pragma config PWRTE = OFF
#pragma config MCLRE = OFF
#pragma config CP = OFF
#pragma config CPD = OFF
#pragma config BOREN = OFF
#pragma config CLKOUTEN = OFF
#pragma config IESO = OFF
#pragma config FCMEN = OFF

#pragma config WRT = OFF
#pragma config PLLEN = ON
#pragma config STVREN = OFF
#pragma config BORV = LO
#pragma config LVP = OFF

unsigned char RcvFlag;
unsigned char Buffer[3];
int index;

void main(void){
    unsigned char fos1[80];
    unsigned char fos2[80];
    unsigned char i;
    unsigned int count[4]={0,0,0,0};
    unsigned int set[4];
    unsigned char uflag[4]={0,0,0,0};
    float swork;
    OSCCON=0xf0;
    TRISA=0x00;
    TRISB=0x02;
    PORTA=0x00;
    PORTB=0x00;
    ANSELA=0x00;
    ANSELB=0x00;
    TXSTA=0x24;
    RCSTA=0x90;
    RXDTSEL=0;
    BAUDCON=0x08;
    SPBRG=255;
    index=0;
    RcvFlag=0;

    PIR1bits.RCIF=0;
    PIE1bits.RCIE=1;
    INTCONbits.PEIE=1;
    INTCONbits.GIE=1;
    OUT=1;

    for(i=85;i>12;i--){
        swork=22*pow(1.059463,85-i);
        fos2[i-12]=(unsigned char)(swork/256);
        fos1[i-12]=(unsigned char)((char)(swork)%256);
    }
    for(i=0;i<7;i++){
        fos1[i+73]=(unsigned char)(21-i);
        fos2[i+73]=0;
    }

    while(1){
        if(RcvFlag){
            RcvFlag=0;
            if((Buffer[0] & 0xf0)==0x90){
                while(Buffer[1]>91)Buffer[1]-=12;
                while(Buffer[1]<12)Buffer[1]+=12;
                if(Buffer[2]==0x00){
                    i=0;
                    while(i<4){
                        if(set[i]==fos2[Buffer[1]-12]*256+fos1[Buffer[1]-12]){
                            uflag[i]=0;
                            LED=0;
                        }
                        i++;
                    }
                }else{
                    i=0;
                    while(i<4){
                        if(uflag[i]==0)break;
                        i++;
                    }
                    if(i<4){
                        set[i]=fos2[Buffer[1]-12]*256+fos1[Buffer[1]-12];
                        uflag[i]=1;
                        count[i]=0;
                        LED=1;
                    }
                }
            }else if((Buffer[0] & 0xf0)==0x80){
                while(Buffer[1]>91)Buffer[1]-=12;
                while(Buffer[1]<12)Buffer[1]+=12;
                i=0;
                while(i<4){
                    if(set[i]==fos2[Buffer[1]-12]*256+fos1[Buffer[1]-12]){
                        uflag[i]=0;
                        LED=0;
                    }
                    i++;
                }
            }
        }

        for(i=0;i<4;i++){
            if(count[i]==set[i] && uflag[i]){
                OUT=0;
            }
            if(count[i]==set[i]+1){
                OUT=1;
                count[i]=0;
            }
            count[i]++;
        }
    }
}
void interrupt isr(void){
    char cn=3;
    if(PIR1bits.RCIF){
        PIR1bits.RCIF=0;
        Buffer[index]=RCREG;
        index++;
        if((Buffer[0] & 0xf0)==0xc0) cn=2;
        if((Buffer[0] & 0xf0)==0xd0) cn=2;
        if(Buffer[0]==0xf1) cn=2;
        if(Buffer[0]==0xf3) cn=2;
        if(Buffer[0]==0xf6) cn=1;
        if(Buffer[0]==0xf8) cn=1;
        if(Buffer[0]==0xfa) cn=1;
        if(Buffer[0]==0xfb) cn=1;
        if(Buffer[0]==0xfc) cn=1;
        if(Buffer[0]==0xfe) cn=1;
        if(Buffer[0]==0xff) cn=1;
        if(index==cn){
            index=0;
            RcvFlag=1;
        }
    }
}
