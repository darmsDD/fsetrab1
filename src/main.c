#include "main.h"


float tempR=-10.0,tempI=-10.0,tempE=-10.0;
int contador = 0,escreve=1 , contador2=0;
int hot=-2;

struct temperature {
    float tempI2;
    float tempR2;
    float tempE2;
};


int main(){

    signal(SIGALRM,int_trata_alarme);
    signal(SIGINT, trata_interrupcao);
    //signal(SIGTERM, trata_interrupcao);
    //signal(SIGQUIT, trata_interrupcao);
    //signal(SIGHUP, trata_interrupcao);
    //signal(SIGTSTP, trata_interrupcao);
    

    ualarm(500,500);
    sleep(2);
    
    return 0;
}

void* uart () {
    int i=0;
    // get internal temperature
    do{
        tempI = initUart(1);
        printf("%f\n",tempI);
        i++;
    }while(tempI<0 && i<1);
    if(tempI<0){tempI=30.12;}
     // get referencial temperature
    i=0;
    do{
        tempR = initUart(2);
        printf("%f\n",tempR);
        i++;
    }while(tempR<0 && i<1);
    if(tempR<0){tempR=40.12;}
    return NULL;
}

void * i2c(){
    //get external temperature
    tempE = i2c_main();
    return NULL;
}
void * lcd(void * parameters){
    struct temperature * temp = (struct temperature *)parameters;
    float tempI3,tempR3,tempE3;
    tempI3 = temp->tempI2;
    tempR3 = temp->tempR2;
    tempE3 = temp->tempE2;
    printf("%f %f %f\n",tempI3,tempR3,tempE3);
    lcd_main(tempI3,tempR3,tempE3);
    return NULL;
}

void int_trata_alarme(int sig){
    pthread_t thread1_id;
    pthread_t thread2_id;
    pthread_create (&thread1_id, NULL, &uart, NULL);
    pthread_create (&thread2_id, NULL, &i2c, NULL);
    pthread_join (thread1_id, NULL);
    pthread_join (thread2_id, NULL);
    printf("tempI = %f tempR = %f tempE = %f\n",tempI,tempR,tempE);
    pthread_t thread3_id;
    struct temperature temp;
    temp.tempI2 = tempI;
    temp.tempR2 = tempR;
    temp.tempE2 = tempE;
    pthread_create (&thread3_id, NULL, &lcd,&temp);
    pthread_join (thread2_id, NULL);
    
    if(tempI>tempR){
        hot=0;
        gpio(hot);
    }
    else if(tempI<tempR){
        hot=1;
        gpio(hot);
    }
    if(fabs(tempI-tempR)<2 || contador2>=10){
        desliga_resistor_ventoinha(hot);
        hot=-2;
        contador2=0;
    }
    contador2++;
    contador++;
    if(contador==4){
        contador =0;
        FILE *fp2;
        if(escreve==1){
            fp2=fopen("trab1.csv","w");
            fprintf(fp2,"TemperatureI, TemperatureR, TemperatureE, Date and Time\n");
            fclose(fp2);
        }else{
            fp2=fopen("trab1.csv","a");
        }
        escreve=0;
        time_t t = time(NULL);
        struct tm *tm = localtime(&t);
        fprintf(fp2,"%0.2lf deg C, %0.2lf hPa, %0.2lf%%, %s", tempI, tempR, tempE, asctime(tm));
        printf("%0.2lf deg C,  %0.2lf C,  %0.2lf C , %s", tempI, tempR, tempE, asctime(tm));
        fclose(fp2);
    }
    
    
    
}
    
    
void trata_interrupcao(int sinal){
    printf("\nentrei na interrupção\n");
    trata_interrupcao_gpio();
    trata_interrupcao_lcd();
    trata_interrupcao_uart();
    exit(0);
}