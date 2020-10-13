#include "gpio.h"
#include <bcm2835.h>
int aberto;
int gpio(int hot)
{
    configura_pinos();
    if(hot==0){
        bcm2835_gpio_write(RESISTOR,0);
        bcm2835_gpio_write(VENTOINHA,1);
    } else { 
        bcm2835_gpio_write(RESISTOR,1);
        bcm2835_gpio_write(VENTOINHA,0);
    }

    return 1;
}

void configura_pinos(){
    // Configura pinos dos LEDs como saídas
    bcm2835_gpio_fsel(RESISTOR, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(VENTOINHA, BCM2835_GPIO_FSEL_OUTP);
}

void trata_interrupcao_gpio(void) {
    bcm2835_gpio_write(RESISTOR,1);
    bcm2835_gpio_write(VENTOINHA,1);
    bcm2835_close();
}

void desliga_resistor_ventoinha(int hot){
    if(hot){
        bcm2835_gpio_write(VENTOINHA,1);
    }
    else{
        bcm2835_gpio_write(RESISTOR,1);
    }
}
