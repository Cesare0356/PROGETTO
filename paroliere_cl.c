#include "includes.h"
#include "dataclient.h"

//Variabili globali utilizzate
int client_socket;
pthread_t send_thread,receive_thread,main_thread;
int registrato = 0;

//Funzione per rendere maiuscolo un carattere
char to_up(char c) {
    if (c>='a' && c<='z') {
        return c-('a'-'A');
    }
    return c; 
}

//Funzione per rendere in maiuscolo una stringa
void to_uppstr(char *str) {
    while (*str != '\0') {
        *str = to_up(*str);
        str++;
    }
}

//Gestore chiusura client digitando fine
void handle_fine() {
    // Invia un segnale al thread principale per gestire la chiusura
    pthread_kill(main_thread, SIGUSR1);
    pthread_exit(NULL);
}

//Gestore chiusura client con SIGINT
void handle_sig(int sig) {
    if (sig == SIGINT) {
        pthread_kill(main_thread, SIGUSR1);
    } else if (sig == SIGUSR1) {
        if(pthread_self() == main_thread) {
            //Invio un messaggio di chiusura al server
            message_info msg;
            msg.type = MSG_CHIUSURA_CLIENT;
            send_message(client_socket,&msg);

            //Cancella e attendi la terminazione dei thread
            pthread_cancel(send_thread);
            pthread_cancel(receive_thread);

            //Termina il processo
            exit(EXIT_SUCCESS);
        }
    }
}

//Thread per l'invio dei messaggi 
void* send_messages(void* arg) {
    free(arg);
    //Per le opzioni da linea di comando
    char command[256];
    //Buffer parola e registrazione
    char buffer[256];
    //Invio messaggi tra client e server
    message_info msg;
    ssize_t e;
    printf("[PROMPT PAROLIERE]-->\n");
    while (1) {
        //Leggo un comando dall'input standard
        SYSC(e,read(STDIN_FILENO,command,256),"nella read");

        //Gestione dei vari comandi disponibili
        if (strcmp(command,"aiuto\n") == 0) {
            //Visualizzazione comandi di aiuto
            printf("Comandi disponibili:\n");
            printf("  aiuto\n");
            printf("  registra_utente <nome utente>\n");
            printf("  matrice\n");
            printf("  p <parola>\n");
            printf("  fine\n");
            printf("  classifica\n");
            printf("[PROMPT PAROLIERE]-->\n");
        } 
        else if (!registrato && sscanf(command,"registra_utente %10s",buffer) == 1) {
            //Comando per registrare un utente
            msg.type = MSG_REGISTRA_UTENTE;
            strcpy(msg.data,buffer);
            msg.length = strlen(msg.data);
            send_message(client_socket,&msg);
            //L'utente richiede subito la matrice in modo da poter giocare
            msg.type = MSG_MATRICE;
            send_message(client_socket,&msg);
        } 
        else if (registrato) {
            if (strcmp(command,"matrice\n") == 0) {
                //Comando per richiedere la matrice
                msg.type = MSG_MATRICE;
                send_message(client_socket,&msg);
            } 
            else if (sscanf(command,"p %16s",buffer) == 1) {
                //Comando per inviare una parola
                msg.type = MSG_PAROLA;
                to_uppstr(buffer);
                strcpy(msg.data,buffer);
                msg.length = strlen(buffer);
                send_message(client_socket,&msg);
            } 
            else if (strcmp(command,"fine\n") == 0) {
                //Termino il programma
                handle_fine();
            } 
            else if (strcmp(command,"classifica\n") == 0) {
                //Comando per richiedere la classifica
                msg.type = MSG_PUNTI_FINALI;
                send_message(client_socket,&msg);
            } 
            else {
                //Se l'utente sbaglia parola
                printf("Digita 'aiuto'\n");
                printf("[PROMPT PAROLIERE]-->\n");
            }
        } 
        else if (strcmp(command,"fine\n") == 0) {
            //Termino il programma
            handle_fine();
        } 
        else {
            //Se si sbaglia l'inserimento
            printf("Registrati per fare comandi\n");
            printf("[PROMPT PAROLIERE]-->\n");
        }
        //Pulisco il buffer dei comandi
        memset(command,0,sizeof(command));
    }
    pthread_exit(NULL);
}

//Thread per ricevere messaggi
void* receive_messages(void* arg) {
    free(arg);
    message_info msg;
    while (1) {
        //Ricevo un messaggio dal server
        receive_message(client_socket,&msg);

        //Gestisco i vari tipi di messaggi ricevuti
        if (msg.type == MSG_OK) {
            //Imposto il flag registrato in modo che possa fare le azioni di un utente registrato
            registrato = 1;
        } 
        else if (msg.type == MSG_ERR) {
            //Stampo i messaggi di errore
            printf("%s\n",msg.data);
            printf("[PROMPT PAROLIERE]-->\n");
        } 
        else if (msg.type == MSG_CHIUSURA_CLIENT) {
            //Termino il programma
            handle_fine();
        } 
        else if (registrato) {
            if (msg.type == MSG_MATRICE) {
                //Stampo la matrice
                printf("%s\n",msg.data);
            } 
            else if (msg.type == MSG_TEMPO_PARTITA) {
                //Stampo il tempo rimanente in una partita
                printf("%s\n",msg.data);
                printf("[PROMPT PAROLIERE]-->\n");
            } 
            else if (msg.type == MSG_TEMPO_ATTESA) {
                //Stampo il tempo di attesa per la nuova partita
                printf("%s\n",msg.data);
                printf("[PROMPT PAROLIERE]-->\n");
            } 
            else if (msg.type == MSG_PUNTI_PAROLA) {
                //Stampo i punti ottenuti dall'inserimento di una parola
                if (atoi(msg.data) == 0)
                    printf("Parola già inserita\n");
                else
                    printf("Punti ottenuti da questa parola: %s\n",msg.data);
                printf("[PROMPT PAROLIERE]-->\n");
            } 
            else if (msg.type == MSG_PUNTI_FINALI) {
                //Mostro i punti finali all'utente
                printf("%s\n",msg.data);
                printf("[PROMPT PAROLIERE]-->\n");
            } 
            else if (msg.type == MSG_CLASSIFICA_PRONTA) {
                //Richiedo la classifica finale
                msg.type = MSG_PUNTI_FINALI;
                send_message(client_socket, &msg);
            }
        }      
    }
    return NULL;
}

//Funzione per impostare il client socket
void client_fun(char* nome_server, int porta_server) {
    int retvalue;
    struct sockaddr_in server_addr;

    //Creo il socket del client
    SYSC(client_socket,socket(AF_INET,SOCK_STREAM,0),"nella socket");

    //Imposto i parametri della socket del server
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(porta_server);
    server_addr.sin_addr.s_addr = inet_addr(nome_server);

    //Effettuo la connessione al server
    SYSC(retvalue,connect(client_socket,(struct sockaddr*)&server_addr,sizeof(server_addr)),"nella connect");
    
    //Creo i thread per inviare e ricevere messaggi
    SYSC(retvalue,pthread_create(&send_thread,NULL,send_messages,NULL),"nella create send");
    SYSC(retvalue,pthread_create(&receive_thread,NULL, receive_messages,NULL),"nella create receive");

    //Attendo la terminazione dei thread
    SYSC(retvalue,pthread_join(send_thread,NULL),"nella join1");
    SYSC(retvalue,pthread_join(receive_thread,NULL),"nella join2");
    return;
}

//Main per prendere i parametri e fare sigaction
int main(int argc, char* argv[]) {
    //Controllo che il numero di argomenti sia sufficiente
    if (argc < 3) {
        fprintf(stderr, "Uso: nome_server porta_server\n");
        exit(EXIT_FAILURE);
    }
    
    //Ottengo il nome del server e la porta dal comando
    char* nome_server = argv[1];
    int porta_server = atoi(argv[2]);
    int retvalue;
    
    //Imposto il gestore dei segnali
    struct sigaction v_SIGINT;
    sigset_t maschera;
    sigemptyset(&maschera);
    SYSC(retvalue,sigaddset(&maschera,SIGINT),"nella sigaction");
    SYSC(retvalue,sigaddset(&maschera,SIGUSR1),"nella sigaction");

    v_SIGINT.sa_handler = handle_sig;
    v_SIGINT.sa_mask = maschera;
    v_SIGINT.sa_flags = SA_RESTART;

    //Maschera SIGINT
    sigaction(SIGINT,&v_SIGINT, NULL);
    //Maschera SIGURS1 per inviare segnale al main per terminare
    sigaction(SIGUSR1,&v_SIGINT, NULL);

    //Prendo il tid del main in modo che possa chiudere i due thread e poi terminare correttamente
    main_thread = pthread_self();
    
    //Avvio la funzione principale del client
    client_fun(nome_server,porta_server);
}

