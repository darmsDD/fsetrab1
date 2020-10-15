#ifndef _main_H_
#define _main_H_

#include "uart.h"
#include "bme280.h"
#include "gpio.h"
#include "lcd.h"
#include<pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>

void * uart ();
void * arquivo();
void * i2c_TE();
void * printTemp();
void * entradaUser();
void * lcd(void * parameters);
void int_trata_alarme(int sig);
void trata_interrupcao(int sinal);
void initNcurs();


#endif