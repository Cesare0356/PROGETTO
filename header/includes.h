#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include <getopt.h>
#define MSG_OK 'K'
#define MSG_ERR 'E'
#define MSG_REGISTRA_UTENTE 'R'
#define MSG_MATRICE 'M'
#define MSG_TEMPO_PARTITA 'T'
#define MSG_TEMPO_ATTESA 'A'
#define MSG_PAROLA 'W'
#define MSG_PUNTI_FINALI 'F'
#define MSG_PUNTI_PAROLA 'P'
#define MSG_CHIUSURA_CLIENT 'C'
#define MSG_CLASSIFICA_PRONTA 'Z'
#define SYSC(v, c, m) \
if ((v = c) == -1) { perror(m); exit(errno); }
#define SYSCN(v, c, m) \
if ((v = c) == NULL) { perror(m); exit(errno); }
//Struttura dati per inviare e ricevere messaggi
typedef struct {
    char type;
    unsigned int length;
    char data[256];
}message_info;

//Funzioni da implementare per inviare messaggi client-server e viceversa
void send_message(int socket, message_info* msg);
void receive_message(int socket, message_info* msg);
