#include "uart.h"
int uart0_filestream = -1;
float initUart(int option){

    uart0_filestream = open("/dev/serial0", O_RDWR | O_NOCTTY | O_NDELAY);    //Open in non blocking read/write mode
    if (uart0_filestream == -1)
    {
        printf("Erro - Não foi possível iniciar a UART.\n");
    }
    else
    {
        //printf("UART inicializada!\n");
    }    
    struct termios options;
    tcgetattr(uart0_filestream, &options);
    options.c_cflag = B115200 | CS8 | CLOCAL | CREAD;     //<Set baud rate
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(uart0_filestream, TCIFLUSH);
    tcsetattr(uart0_filestream, TCSANOW, &options);

    unsigned char b[100];

    unsigned char * a;
    a = &b[0]; 
    if(option==1){
        *a++ = 0XA1;
        
    }
    else{
        *a++ = 0XA2;
    }
        int rx_length,count;
        float vari;
    *a++ = 3;
    *a++ = 2;
    *a++ = 7;
    *a++ = 8;
    
    if (uart0_filestream != -1)
    {   
    
        //printf("Escrevendo caracteres na UART ...\n");
        // pegando temperatura interna
    
        
        count = write(uart0_filestream, &b[0], 5); 
        if(!valEscrita(count)){
            close(uart0_filestream);
            return -1;
        }
        usleep(50000);
        rx_length = read(uart0_filestream, &vari, 4);
        if(valLeitura(rx_length)){
            //char s;
            //s = (option==1)?'I':'R';
            //printf("temp%c = %f\n",s,vari);
        }
        else{
            close(uart0_filestream);
            return -1;
        }
    }
    close(uart0_filestream);
    return vari;
}

int valLeitura(int rx_length){
    if (rx_length < 0)
    {   
        //printf("Erro na leitura.\n"); //An error occured (will occur if there are no bytes)
        return 0;
    }
    else if (rx_length == 0)
    {
        //printf("Nenhum dado disponível.\n"); //No data waiting
        return 0;
    }
   
    //Bytes received
    //printf("%i Bytes lidos\n", rx_length);
    
   return 1;

}
int valEscrita(int count){
    if (count < 0)
        {
            //printf("UART TX error\n");
            return 0;
        }
    else
    {   
        //printf("escrito.\n");
        return 1;
    }
}

void trata_interrupcao_uart(){
    if(uart0_filestream!=-1){
        close(uart0_filestream);
    }
}
