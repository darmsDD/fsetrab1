#include "main.h"
#include <bcm2835.h>
#include <ncurses.h>
#include <curses.h>


float tempR=60.0,tempI=30.12,tempE=-10.0,histerese=4.0;
int  escreve=1,contador=0;
volatile int esperaEsc = 0,esperaLcd=0;
int hot=-2;
FILE *fp2;

char escolha[1000];
int cont=0;
int ch,ch2,i=0,option=0,term=0;
int yMax,xMax;
WINDOW * ler, *escrever,* mensagemLeitura;



pthread_t thread3_id,thread4_id;

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
    

    ualarm(5000,500000);
    while(1){
        sleep(2);
    }
    return 0;
}

void* uart () {
    int i=0;
    // get internal temperature
    float tp = initUart(1);
    //printf("%f\n",tempI);
    i++;
    if(tp>0){tempI=tp;}

    // get referencial temperature
    if(term==0){
        tp = initUart(2);
        if(tp>0){tempR=tp;}
    }
        return NULL;
    
}

void * i2c_TE(){
    //get external temperature
    int a = TE();
    if(a>0){tempE = a;}
    return NULL;
}
void * lcd(void * parameters){
    struct temperature * temp = (struct temperature *)parameters;
    float tempI3,tempR3,tempE3;
    tempI3 = temp->tempI2;
    tempR3 = temp->tempR2;
    tempE3 = temp->tempE2;
    lcd_main(tempI3,tempR3,tempE3);
    return NULL;
}

void * arquivo(){
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
    return NULL;
}




void * printTemp(){
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
    return NULL;
}
void * entradaUser(){
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
    
    //printf("%s\n",escolha);
    return NULL;
}


void int_trata_alarme(int sig){
    contador++;
    pthread_t thread1_id;
    pthread_t thread2_id;
    pthread_create (&thread1_id, NULL, &uart, NULL);
    pthread_create (&thread2_id, NULL, &i2c_TE, NULL);

    pthread_t thread5_id;
    pthread_t thread6_id;
    pthread_create (&thread5_id, NULL, &entradaUser, NULL);
    pthread_create (&thread6_id, NULL, &printTemp, NULL);
    pthread_join (thread1_id, NULL);
    pthread_join (thread2_id, NULL);
    pthread_join (thread6_id, NULL);
    //printf("tempI = %f tempR = %f tempE = %f\n",tempI,tempR,tempE);
    
    if(contador==4){
        struct temperature temp;
        temp.tempI2 = tempI;
        temp.tempR2 = tempR;
        temp.tempE2 = tempE;
        pthread_create (&thread3_id, NULL, &lcd,&temp);
        pthread_create(&thread4_id,NULL,&arquivo,NULL);
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
    
    
    
}


    
    
void trata_interrupcao(int sinal){
    printf("\nentrei na interrupção\n");
    pthread_join(thread4_id,NULL);
    pthread_join(thread3_id,NULL);
    trata_interrupcao_gpio();
    trata_interrupcao_lcd();
    trata_interrupcao_uart();
    endwin();	
    exit(0);
}