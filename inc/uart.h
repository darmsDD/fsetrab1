#ifndef _UART_H_
#define _UART_H_


#include <stdio.h>
#include <unistd.h>         //Used for UART
#include <fcntl.h>          //Used for UART
#include <termios.h>        //Used for UART


int valLeitura(int rx_length);
int valEscrita(int count);
int initUart(int option);
void trata_interrupcao_uart(void);
#endif