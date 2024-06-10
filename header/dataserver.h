#include <pthread.h>
#define MAX_CLIENTS 10
//Enum per gli stati della partita
typedef enum {
    STATO_ATTESA,
    STATO_INGIOCO,
    STATO_PAUSA
} game_state;
game_state curr_state = STATO_ATTESA;

//Mutex per sincronizzazione
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

//Seed
unsigned int seed = 0;
//Socket del server
int server_socket;
//Dizionario standard
char *dizionario = "dictionary_ita.txt";
//File vuoto se non passo --matrici, altrimenti genera random
char *data_filename="";
//Tempo di attesa fine partita
int tempo_attesa = 10;  
//Durata di una partita
int durata = 10;
//Timer per segnare l'inizio e la fine della partita
time_t inizio_partita;
time_t fine_partita;
//Buffer globale che invia la classifica all'utente quando richiesta durante la pausa
char classifica[256];

//Funzioni da implementare
void *thread_client(void *arg);
void server(char *nome_server, int porta);
void handle_sigint();
void handle_alarm();
int tempo_rim();
void *thread_scorer(void *arg);
