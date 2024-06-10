#include "includes.h"
#define PORTA 1025
void client() {
    int client_socket, retvalue;
    struct sockaddr_in server_addr;

    //Creo il socket del client
    SYSC(client_socket,socket(AF_INET,SOCK_STREAM,0),"nella socket");

    //Imposto i parametri della socket del server
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORTA);

    //Effettuo la connessione al server
    SYSC(retvalue,connect(client_socket,(struct sockaddr*)&server_addr,sizeof(server_addr)),"nella connect");
    printf("Connessione client stabilita\n");
    message_info msg;
    receive_message(client_socket,&msg);
    printf("Messaggio ricevuto:%s",msg.data);

    //Chiudo socket
    SYSC(retvalue,close(client_socket),"chiusura del client");
}

void server() {
    struct sockaddr_in server_addr,client_addr;
    int client_socket,server_socket,retvalue;
    socklen_t addr_size = sizeof(client_addr);

    //Creo la socket del server
    SYSC(server_socket,socket(AF_INET,SOCK_STREAM,0),"nella socket");

    //Imposto i parametri della socket del server
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORTA);

    //Effettuo il bind della socket del server
    SYSC(retvalue,bind(server_socket,(struct sockaddr *)&server_addr,sizeof(server_addr)),"nella bind");

    //Metto la socket del server in ascolto
    SYSC(retvalue,listen(server_socket,3),"nella listen");
    printf("Connessione server stabilita\n");

    SYSC(client_socket,accept(server_socket,(struct sockaddr *)&client_addr,&addr_size),"nella accept");

    //Invio messaggio
    message_info msg;
    msg.type = MSG_PAROLA;
    strcpy(msg.data,"Ciaoooo\n");
    msg.length = strlen(msg.data);
    send_message(client_socket,&msg);

    //Chiudo i socket
    SYSC(retvalue,close(server_socket),"nella close");
}

int main(void) {
    pid_t pid;
    SYSC(pid,fork(),"nella fork");
    if(pid == 0) {
        client();
        exit(EXIT_SUCCESS);
    }
    server();
    return 0;
}