#include "includes.h"

//Funzione per ricevere i messaggi tra i socket
void receive_message(int socket, message_info* msg){
    int retvalue;
    
    //Ricevo messaggi tramite syscalls
    SYSC(retvalue,read(socket,&msg->length,sizeof(unsigned)),"nella ricezione della lunghezza di messaggio");
    SYSC(retvalue,read(socket,&msg->type,sizeof(char)),"nella ricezione del tipo di messaggio");
    SYSC(retvalue,read(socket,msg->data,sizeof(msg->data)),"nella ricezione del data");

    //Stringa null-terminated
    msg->data[sizeof(msg->data)-1] = '\0';
}

//Funzione per inviare i messaggi tra i socket
void send_message(int socket,message_info* msg){
    int retvalue;
    //Invio il messaggio tramite syscalls
    SYSC(retvalue,write(socket,&msg->length,sizeof(unsigned int)),"nella scrittura della lunghezza del messaggio");
    SYSC(retvalue,write(socket,&msg->type,sizeof(char)),"nella scrittura del tipo di messaggio");
    SYSC(retvalue,write(socket,msg->data,sizeof(msg->data)),"nella scrittura del data");
}