#include "main.h"
#include <bcm2835.h>


float tempR=28,tempI=30.12,tempE=-10.0;
int  escreve=1,contador=0;
volatile int esperaEsc = 0,esperaLcd=0;
int hot=-2;
FILE *fp2;

struct temperature {
    float tempI2;
    float tempR2;
    float tempE2;
};


int main(){

    signal(SIGALRM,int_trata_alarme);
    signal(SIGINT, trata_interrupcao);
    signal(SIGTERM, trata_interrupcao);
    signal(SIGQUIT, trata_interrupcao);
    signal(SIGHUP, trata_interrupcao);
    signal(SIGTSTP, trata_interrupcao);
    bcm2835_init();
    

    ualarm(500,500);
    sleep(2);
    
    return 0;
}

void* uart () {
    int i=0;
    // get internal temperature
    int tp = initUart(1);
    printf("%f\n",tempI);
    i++;
    if(tp>0){tempI=tp;}

    // get referencial temperature
    tp = initUart(2);
    if(tp>0){tempR=tp;}
    return NULL;
}

void * i2c_TE(){
    //get external temperature
    int a = TE();
    if(a>0){tempE = a;}
    return NULL;
}
void * lcd(void * parameters){
    esperaLcd=1;
    struct temperature * temp = (struct temperature *)parameters;
    float tempI3,tempR3,tempE3;
    tempI3 = temp->tempI2;
    tempR3 = temp->tempR2;
    tempE3 = temp->tempE2;
    printf("%f %f %f\n",tempI3,tempR3,tempE3);
    lcd_main(tempI3,tempR3,tempE3);
    sleep(2);
    esperaLcd=0;
    return NULL;
}

void * arquivo(){
    esperaEsc=1;
    if(escreve==1){
        fp2=fopen("trab1.csv","w");
        fprintf(fp2,"TemperatureI, TemperatureR, TemperatureE, Date and Time\n");
        fclose(fp2);
    }
    fp2=fopen("trab1.csv","a");
    escreve=0;
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    fprintf(fp2,"%4.2f deg C, %4.2f deg C, %4.2f degC, %s", tempI, tempR, tempE, asctime(tm));
    //printf("%0.2lf deg C,  %0.2lf C,  %0.2lf C , %s", tempI, tempR, tempE, asctime(tm));
    fclose(fp2);
    sleep(2);
    esperaEsc=0;
    return NULL;
}

void int_trata_alarme(int sig){
    contador++;
    pthread_t thread1_id;
    pthread_t thread2_id;
    pthread_create (&thread1_id, NULL, &uart, NULL);
    pthread_create (&thread2_id, NULL, &i2c_TE, NULL);
    pthread_join (thread1_id, NULL);
    pthread_join (thread2_id, NULL);
    printf("tempI = %f tempR = %f tempE = %f\n",tempI,tempR,tempE);
    pthread_t thread3_id;
    struct temperature temp;
    temp.tempI2 = tempI;
    temp.tempR2 = tempR;
    temp.tempE2 = tempE;
    
    if(!esperaEsc){
        pthread_t thread4_id;
        pthread_create(&thread4_id,NULL,&arquivo,NULL);
        contador=0;
    }
    if(!esperaLcd){
        pthread_create (&thread3_id, NULL, &lcd,&temp);
    }
    while(tempR<tempE){
        printf("Por favor usuário selecione outra temperatura\n");
        scanf("%f",&tempR);
    }
    if(tempI<tempR){
        hot=0;
        gpio(hot);
    }
    else if(tempI>tempR){
        hot=1;
        gpio(hot);
    }
    if(fabs(tempI-tempR)<2){
        printf("vou desligar a ventoinha/resistor\n");
        desliga_resistor_ventoinha(hot);
        hot=-2;
    }
    
    
}
    
    
void trata_interrupcao(int sinal){
    printf("\nentrei na interrupção\n");
    trata_interrupcao_gpio();
    trata_interrupcao_lcd();
    trata_interrupcao_uart();
    if(fp2!=NULL){
        fclose(fp2);
    }
    exit(0);
}