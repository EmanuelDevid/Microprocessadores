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
#pragma config FOSC = XT_XT // Oscillator Selection bits (XT oscillator (XT))
#pragma config WDT = OFF    // Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))
#pragma config PBADEN = OFF // PORTB A/D Enable bit (PORTB<4:0> pins are configured as digital I/O on Reset)
#pragma config MCLRE = ON   // MCLR Pin Enable bit (MCLR pin enabled; RE3 input pin disabled)
#pragma config LVP = OFF    // Single-Supply ICSP Enable bit (Single-Supply ICSP disabled)
unsigned int mediador = 0;  // variável para verificar se o botão do mediador foi pressionado
unsigned int button1 = 0;   // variável para verificar se o botão do jogador 1 foi pressionado
unsigned int button2 = 0;   // variável para verificar se o botão do jogador 2 foi pressionado

// função que escreve caracteres no lcd com a função "printf()"
void putch(char data)
{
    escreve_lcd(data);
}

int temp_player1 = 0; // variÃ¡vel que guarda o tempo do jogador1
int temp_player2 = 0; // variÃ¡vel que guarda o tempo do jogador2

void main(void)
{
    TRISBbits.TRISB0 = 1; // definindo RD0 como entrada do button mediador
    TRISBbits.TRISB1 = 1; // definindo RD1 como entrada do button jogador1
    TRISBbits.TRISB2 = 1; // definindo RD2 como entrada do button jogador2

    TRISAbits.RA3 = 0;    // definindo RA3 omo saida do led do mediador
    TRISAbits.RA0 = 0;    // definindo RA0 como saida do BUZZER
    TRISAbits.RA1 = 0;    // definindo RA1 como saida do led do jogador1
    TRISAbits.RA2 = 0;    // definindo RA2 como saida do led do jogador2
    TRISBbits.TRISB3 = 0; // definindo RD3 como saida ->buzzer

    PORTD = 0; // pinos a serem utilizados para o LCD
    TRISD = 0x00;
    ADCON1 = 0x0F;

    T2CONbits.T2CKPS1 = 0; // Pre scaler 1:4
    T2CONbits.T2CKPS0 = 1; // Pre scaler 1:4
    PR2 = 250;             // 250 Para estourar a cada 1 ms

    // T2OUTPS3 a T2OUTPS0 controlam quantos estouros de timer2 precisam pra levar TMR2IF pra 1
    T2CONbits.T2OUTPS3 = 0;
    T2CONbits.T2OUTPS2 = 0;
    T2CONbits.T2OUTPS1 = 0;
    T2CONbits.T2OUTPS0 = 1;
    INT0IE = 1; // habilita interrupï¿½ï¿½o int0

    PEIE = 1; // habilita interrupicoes dos perifericos
    GIE = 1;  // habilita interrupicoes globalmente

    TMR2 = 0;

    T2CONbits.TMR2ON = 1; // iniciaa a contagem do timer
    int print_tela = 0;   // variavel que controla a impressao (print) na tela

    while (1)
    {
        while (mediador == 1)
        {
            if (TMR2IF){
                TMR2IF = 0;
                if (button1 == 0)
                    temp_player1++;
                if (button2 == 0)
                    temp_player2++;
            }

            if ((button1 == 1) && (button2 == 1)){
                mediador = 0;

                if(temp_player1 < temp_player2)
                    PORTAbits.RA1 = 1;

                else if(temp_player1 > temp_player2)
                    PORTAbits.RA2 = 1;

                print_tela = 1;
            }
        }

        while (print_tela){
            inicializa_lcd(); //inicializando o lcd
            limpa_lcd(); //limpando lcd
            caracter_inicio(1, 1); // define o ponto de inicio da frase na primeira linha
            printf("TEMPO JOGADOR 1: = %d ms", temp_player1); //printando na tela
            __delay_ms(250);       // escreve string no LCD
            __delay_ms(250);       // escreve string no LCD
            caracter_inicio(2, 1); // define o ponto de inicio da frase na segunda linha
            printf("TEMPO JOGADOR 2: = %d ms", temp_player2); //printando na tela
            __delay_ms(250); // escreve string no LCD
            __delay_ms(250); // escreve string no LCD
            print_tela = 0;  // renicia a variavel pra que o print ocorra apenas uma vez
        }
    }
}


//interrupção que é chamada quando o botao do mediador é pressionado
void __interrupt(high_priority) isr()
{
    if (INT0IF){
        if ((button1 == 0) && (button2 == 0)){
            mediador = 1;
            PORTAbits.RA3 = 1; //ascende o LED do mediador
            INT1IE = 1; // habilitando interrupção que trata do botao 1
            INT2IE = 1; // habilitando interrupção que trata do botao 2
            temp_player1 = 0;
            temp_player2 = 0;
            PORTAbits.RA0 = 1;
            __delay_ms(500);
            PORTAbits.RA0 = 0;
            PORTAbits.RA3 = 0;
        }
        else if ((button1 == 1) && (button2 == 1)){
            // condicao para voltar ao estagio inicial
            mediador = 0;
            button1 = 0;
            button2 = 0;
            PORTAbits.RA1 = 0;
            PORTAbits.RA2 = 0;
            PORTAbits.RA3 = 0;
            limpa_lcd();
        }

        INT0IF = 0; //zerando variavel para que o processo ocorra apenas uma vez quando a interrupção é chamada

        return;
    }

    //interrupção que trata do botao do jogador 1 (RB1/INT1)
    if (INT1IF){
        button1 = 1; //se o botao for pressionado button1 recebe 1 e sua incrementação é parada na main
        INT1IF = 0; //zerando variavel para que o processo ocorra apenas uma vez quando a interrupção é chamada
        return;
    }

    //interrupção que trata do botÃ£o do jogador 2 (RB2/INT2)
    if (INT2IF){
        button2 = 1; // se o botao for pressionado button2 recebe 1 e sua incrementação é parada na main
        INT2IF = 0; //zerando variÃ¡vel para que o processo ocorra apenas uma vez quando a interrupção é chamada
        return;
    }
    return;
}