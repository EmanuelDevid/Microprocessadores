/*
 * File:   main.c
 * Author: Emanuel Dêvid
 *
 * Created on 28 de Novembro de 2022, 22:10
 */

#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include "lcd.h"
#define _XTAL_FREQ 4000000
#pragma config FOSC = XT_XT     // Oscillator Selection bits (XT oscillator (XT))
#pragma config WDT = OFF        // Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))
#pragma config PBADEN = OFF     // PORTB A/D Enable bit (PORTB<4:0> pins are configured as digital I/O on Reset)
#pragma config MCLRE = ON       // MCLR Pin Enable bit (MCLR pin enabled; RE3 input pin disabled)
#pragma config LVP = OFF        // Single-Supply ICSP Enable bit (Single-Supply ICSP disabled)
unsigned int mediador=0; // sinaliza se o botão do mediador foi pressionado
unsigned int j1 = 0;//sinaliza se o botão do jogador 1 foi pressionado
unsigned int j2 = 0;//sinaliza se o botão do jogador 2 foi pressionado

// para escrever caracteres no lcd com printf()
void putch(char data)
{
  escreve_lcd(data);   
}

 int contj1=0; //armazena o tempo do jogador1
 int contj2=0; //armazena o tempo do jogador2

void main(void)
{
    TRISBbits.TRISB0 = 1;     // RD0 como entrada -> button mediador
    TRISBbits.TRISB1 = 1;     // RD1  como entrada -> button jogador1
	TRISBbits.TRISB2 = 1;     // RD2 como entrada -> button jogador2
    
    TRISAbits.RA3 = 0 ; // RA3 omo saída ->led mediador 
    TRISAbits.RA0 = 0 ; // RA0 omo saída ->BUZZER
	TRISAbits.RA1 = 0 ; // RA1  como saída ->led jogador1
	TRISAbits.RA2 = 0 ; // RA2 RD2 como saída ->led jogador2 
	TRISBbits.TRISB3 = 0;	// RD3 como saída ->buzzer
    
    PORTD = 0;//utilização para o LCD
    TRISD = 0x00;
    ADCON1 = 0x0F;

    T2CONbits.T2CKPS1 = 0; // Pre scaler 1:4
    T2CONbits.T2CKPS0 = 1; // Pre scaler 1:4
    PR2 = 250;  // 250 Para estourar a cada 1 ms
    //T2OUTPS3 a T2OUTPS0 controlam quantos 
    // estouros de timer2 precisam pra levar TMR2IF pra 1     
    T2CONbits.T2OUTPS3 = 0;
    T2CONbits.T2OUTPS2 = 0;
    T2CONbits.T2OUTPS1 = 0;
    T2CONbits.T2OUTPS0 = 1;
    INT0IE = 1; // habilita interrupção int0
    
    PEIE = 1; // habilita interrupções dos periféricos
    GIE = 1;  // habilita interrupções globalmente
   // unsigned int contj1=0; //armazena o tempo do jogador1
	//unsigned int contj2=0; //armazena o tempo do jogador2
     TMR2=0;
      
    T2CONbits.TMR2ON = 1; // Coloca timer pra contar
    int mostra=0; //variavel para indicar se não houve print na tela ainda

    while (1){
        while(mediador==1){
           
            if (TMR2IF) {
            TMR2IF = 0;
            if (j1 == 0)
             contj1++;
            if (j2 == 0)
             contj2++;
            }
            if((j1==1)&&(j2==1)){
                mediador=0;
                
               if(contj1<contj2){
                    PORTAbits.RA1=1;
                }else if(contj1>contj2){
                    PORTAbits.RA2=1;
               }
                mostra=1;
            }
            
            
           
        }
        
        while(mostra){
            inicializa_lcd();
            limpa_lcd();
            caracter_inicio(1,1);  //define o ponto de inicio da frase na primeira linha   
             printf("TEMPO PLAYER1: = %d ms",contj1); 
              __delay_ms(250);//escreve string no LCD
               __delay_ms(250);//escreve string no LCD
             caracter_inicio(2,1);  //define o ponto de inicio da frase na SEGUNDA linha   
             printf("TEMPO PLAYER2: = %d ms",contj2); 
              __delay_ms(250);//escreve string no LCD
               __delay_ms(250);//escreve string no LCD  
               mostra=0;//impede que fique printando na tela infinitamente
        }
    }
    // Daqui pra frente mostra contagens
}
   
void __interrupt(high_priority) isr() {
    if (INT0IF) {
        // Atendimento do evento produzido pelo push button ligado
        // ao pino RB0/INT0
        
        if((j1==0)&&(j2==0)){
            mediador=1;
            PORTAbits.RA3=1;
           INT1IE=1;// habilita interrupção int1
            INT2IE=1;// habilita interrupção int2
            contj1 = 0;
            contj2 = 0;
            PORTAbits.RA0=1;
             __delay_ms(500);
             PORTAbits.RA0=0;
             PORTAbits.RA3=0;
        }else if((j1==1)&&(j2==1)){// condicao para voltar ao estagio inicial
            mediador=0;
            j1=0;
            j2=0;
            PORTAbits.RA1=0;
            PORTAbits.RA2=0;
            PORTAbits.RA3=0;
            limpa_lcd();
           
            
        }
       
        INT0IF = 0;
       
        return;
    }
    if (INT1IF) {
        // Atendimento do evento produzido pelo push button ligado
        // ao pino RB1/INT1
        j1=1;// se o botao for pressionado j1 recebe 1 e na main para a contagem
     
        
        INT1IF = 0;
        return;
    }
    if(INT2IF){
        j2=1;
       
        INT2IF = 0;
        return;
    }
    return;
}