#include "main.h"
#include <bcm2835.h>



float tempR=60.0,tempI=30.12,tempE=-10.0,histerese=4.0;
int  escreve=1,contador=0,priEx=1;
volatile int esperaEsc = 0,esperaLcd=0;
int hot=-2;
FILE *fp2;

volatile int semf1=1,semf2=1,semf3=1,semf4=1,semf5=1,semf6=1;

char escolha[1000];
int cont=0;
int ch,ch2,i=0,option=0,term=0;
int yMax,xMax;
WINDOW * ler, *escrever,* mensagemLeitura;



pthread_t thread1_id,thread2_id,thread3_id,thread4_id,thread5_id,thread6_id;

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
    initNcurs();
    
    
    pthread_create (&thread5_id, NULL, &entradaUser, NULL);
    ualarm(500000,500000);
    while(1){
        sleep(2);
    }
    return 0;
}

void initNcurs(){
    initscr();			
    noecho();
	cbreak();						   
	keypad(stdscr, TRUE);
    curs_set(0);
    getmaxyx(stdscr,yMax,xMax);
	
	int tamX = xMax- xMax/5,posX=2;
    int tamEscrita = 5, yEscrita = yMax-tamEscrita;

    WINDOW * windowLeitura = newwin(tamEscrita,tamX,yEscrita,posX);
    box(windowLeitura, 0, 0);
    WINDOW * windowEscrita = newwin(yEscrita -1 ,tamX,1,posX);
    box(windowEscrita, 0, 0);
    refresh();
    wrefresh(windowLeitura);
    wrefresh(windowEscrita);
    mensagemLeitura =  newwin(1,tamX-2,yEscrita+1,posX+1);
    wprintw(mensagemLeitura, "1 para temperatura no terminal, 0 no potenciometro e 2 para definir histerese\n");
    wrefresh(mensagemLeitura);
    ler = newwin(1,tamX-2,yEscrita+2,posX+1);
    escrever = newwin(yEscrita -3,tamX-2,2,posX+1);
}

void* uart () {
    semf1=0;
    // get internal temperature
    float tp = initUart(1);
    //printf("%f\n",tempI);
    if(tp>0){tempI=tp;}

    // get referencial temperature
    if(term==0){
        tp = initUart(2);
        if(tp>0){tempR=tp;}
    }
    semf1=1;
        return NULL;
    
}

void * i2c_TE(){
    semf2=0;
    //get external temperature
    float a = TE();
    if(a>0){tempE = a;}
    semf2=1;
    return NULL;
}
void * lcd(void * parameters){
    semf3=0;
    struct temperature * temp = (struct temperature *)parameters;
    float tempI3,tempR3,tempE3;
    tempI3 = temp->tempI2;
    tempR3 = temp->tempR2;
    tempE3 = temp->tempE2;
    lcd_main(tempI3,tempR3,tempE3);
    semf3=1;
    return NULL;
}

void * arquivo(){
    semf4=0;
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
    fclose(fp2);
    semf4=1;
    return NULL;
}




void * printTemp(){
    semf6=0;
    int y,x;
    getmaxyx(escrever, y, x);
    if(i>=y){
        wclear(escrever);
        i=0;
    }
    wprintw(escrever,"tempI = %4.2f tempR = %4.2f tempE = %4.2f histerese = %4.2f\n",tempI,tempR,tempE,histerese);
    //printf("tempI = %f tempR = %f tempE = %f\n",tempI,tempR,tempE);
   
    wrefresh(escrever);
    i++;
    semf6=1;
    return NULL;
}
void * entradaUser(){
    while(1){
        while(ch = wgetch(ler),ch!='\n'){
            escolha[cont] = (char)ch;
            wprintw(ler,"%c",(char)ch);
            cont++;
            wrefresh(ler);
        }
        
        wclear(ler);
        wclear(mensagemLeitura);
        escolha[cont]='\0';
        cont=0;


        wprintw(mensagemLeitura, "1 para temperatura no terminal, 0 no potenciometro e 2 para definir histerese\n");
        if(option==1){
            float intermediario = atof(escolha);
            term=1;
            option=0;
            if(intermediario<tempE){
                wclear(mensagemLeitura);
                wprintw(mensagemLeitura, "Temperatura no Terminal\n");
                wprintw(ler,"Entrada Inválida. Digite um tempR > tempE: ");
                option=1;
            }
            else{
                tempR=intermediario;
            }
        }
        else if(option==2){
            histerese = atof(escolha);
            option=0;
        }
        
        else{
            option = atoi(escolha);
            if(option==0){
                term=0;
            }
            else if(option==1){
                wclear(mensagemLeitura);
                wprintw(mensagemLeitura, "Temperatura no Terminal\n");
                wprintw(ler,"Digite a temperatura: ");
            }
            else if(option==2){
                wclear(mensagemLeitura);
                wprintw(mensagemLeitura, "Histerese\n");
                wprintw(ler,"Digite a histerese: ");
            }
            else{
                wprintw(ler,"Entrada Inválida. Digite novamente: ");
            }
            
        }
        wrefresh(mensagemLeitura);
        wrefresh(ler);
    } 
        //printf("%s\n",escolha);
        return NULL;
}


void int_trata_alarme(int sig){
    contador++;

    if(semf1){pthread_create (&thread1_id, NULL, &uart, NULL);}
    if(semf2){pthread_create (&thread2_id, NULL, &i2c_TE, NULL);}
    
    if(semf6){
        int error;
        if(error = pthread_create (&thread6_id, NULL, &printTemp, NULL),error!=0){
            kill(getpid(),SIGINT);
        }
    }
   
   
    pthread_join (thread1_id, NULL);
    pthread_join (thread2_id, NULL);
    
    //printf("tempI = %f tempR = %f tempE = %f\n",tempI,tempR,tempE);
    
    if(contador==4){
        struct temperature temp;
        temp.tempI2 = tempI;
        temp.tempR2 = tempR;
        temp.tempE2 = tempE;
        if(semf3){pthread_create (&thread3_id, NULL, &lcd,&temp);}
        if(semf4){pthread_create(&thread4_id,NULL,&arquivo,NULL);}
        pthread_join(thread3_id,NULL);
        pthread_join(thread4_id,NULL);
        contador = 0;
    }

    if(fabs(tempI-tempR)<(histerese/2)){
        //printf("vou desligar a ventoinha/resistor\n");
        desliga_resistor_ventoinha(hot);
        hot=-2;
    }
    else if(tempI<tempR){
        hot=0;
        gpio(hot);
    }
    else if(tempI>tempR){
        hot=1;
        gpio(hot);
    }
    
    pthread_join (thread6_id, NULL);
    
}


    
    
void trata_interrupcao(int sinal){
    ualarm(0,0);
    pthread_join (thread1_id, NULL);
    pthread_join (thread2_id, NULL);
    pthread_join(thread3_id,NULL);
    pthread_join(thread4_id,NULL);
    pthread_join (thread6_id, NULL);
    pthread_cancel(thread5_id);
    trata_interrupcao_gpio();
    trata_interrupcao_lcd();
    trata_interrupcao_uart();
    endwin();	
    exit(0);
}