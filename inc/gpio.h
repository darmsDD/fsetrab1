#ifndef _GPIO_H_
#define _GPIO_H_

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

#define RESISTOR RPI_V2_GPIO_P1_16
#define VENTOINHA RPI_V2_GPIO_P1_18

int gpio(int hot);
void configura_pinos();
void desliga_resistor_ventoinha();
void trata_interrupcao_gpio(void);

#endif