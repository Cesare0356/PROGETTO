#include "dataserver.h"
#include "matrix.h"
#include "includes.h"
#include "scorer.h"
//Struttura per contenere i dati del client
typedef struct client_data {
    int socket;
    char username[11];
    int score;
    Nodo* lis; //Lista delle parole inviate dal client
    struct client_data *next; //Puntatore al prossimo nodo della lista
} client_data;

//Puntatore alla testa della lista dei client
client_data *client_list = NULL;

//Gestore sigaction
void handle_sig(int sig) {
    //Gestisco caso server fa SIGINT
    if (sig == SIGINT) {
        handle_sigint();
    } //Gestisco caso in cui finisce una partita o il tempo di attesa tra una partita e l'altra 
    else if (sig == SIGALRM) {
        handle_alarm();
    }
}

//Handler sigint
void handle_sigint() {
    int retvalue;
    message_info msg;

    //Invio un messaggio di chiusura a tutti i client attivi
    msg.type = MSG_CHIUSURA_CLIENT;
    client_data *current = client_list;
    pthread_mutex_lock(&mutex);
    while (current != NULL) {
        send_message(current->socket, &msg);
        current = current->next;
    }
    pthread_mutex_unlock(&mutex);
    //Chiudo il socket del server
    SYSC(retvalue, close(server_socket), "nella close");
    printf("Server chiuso\n");
    exit(EXIT_SUCCESS);
}

//Handler alarm
void handle_alarm() {
    if (curr_state == STATO_INGIOCO) {
        //Cambio lo stato di gioco
        curr_state = STATO_PAUSA;

        fine_partita = time(NULL);
        pthread_t scorer;

        //Creo la coda per i risultati
        score_t *q = malloc(sizeof(score_t));
        if (q == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        pthread_mutex_lock(&mutex);
        client_data *current = client_list;
        while (current != NULL) {
            //Aggiungo client all'interno della coda che userà lo scorer
            push_score(&q,current->username,current->score);

            //Azzero lo score del client
            current->score = 0;

            //Rimuovo le parole prese durante la partita
            while (current->lis != NULL) {
                rimuovi_parola(&(current->lis));
            }
            current = current->next;
        }
        pthread_mutex_unlock(&mutex);

        //Faccio partire lo scorer
        pthread_create(&scorer,NULL,thread_scorer,q);

        //Distacco lo scorer
        pthread_detach(scorer);

        //Imposto il tempo di pausa della partita
        alarm(tempo_attesa);
    } else if (curr_state == STATO_PAUSA) {
        //Cambio lo stato di gioco
        curr_state = STATO_INGIOCO;

        message_info msg;
        char buffer[256];
        genera_matrice(data_filename);

        //Segno il tempo di inizio partita
        inizio_partita = time(NULL);

        //Preparo il messaggio d'invio per la nuova matrice
        msg.type = MSG_MATRICE;
        strcpy(buffer,"\n\n\nNuova partita!\n");
        matrice_tostring(buffer);
        strcpy(msg.data,buffer);
        msg.length = strlen(msg.data);

        //Invio messaggio
        client_data *current = client_list;
        while (current != NULL) {
            send_message(current->socket,&msg);
            current = current->next;
        }

        //Preparo il messaggio d'invio per il tempo partita
        int tempo_rimanente = tempo_rim();
        msg.type = MSG_TEMPO_PARTITA;
        sprintf(buffer,"Tempo rimanente:%d secondi",tempo_rimanente);
        strcpy(msg.data,buffer);
        msg.length = strlen(msg.data);

        //Invio messaggio
        pthread_mutex_lock(&mutex);
        current = client_list;
        while (current != NULL) {
            send_message(current->socket,&msg);
            current = current->next;
        }
        pthread_mutex_unlock(&mutex);

        //Pulisco il buffer della classifica
        memset(classifica,0,sizeof(classifica));

        //Inizio una nuova partita
        alarm(durata);
    }
}

//Main per avviare il server e controllo parametri
int main(int argc, char *argv[]) {
    //Controllo che il numero di argomenti sia sufficiente
    if (argc < 3) {
        fprintf(stderr, "Uso: nome_server porta_server\n");
        exit(EXIT_FAILURE);
    }
    
    //Ottengo il nome del server e la porta dal comando
    char *nome_server = argv[1];
    int porta_server = atoi(argv[2]);

    //Definisco le opzioni per il parsing degli argomenti
    struct option long_options[] = {
        {"matrici", required_argument, 0, 'm'},
        {"durata", required_argument, 0, 'd'},
        {"seed", required_argument, 0, 's'},
        {"diz", required_argument, 0, 'z'},
        {0, 0, 0, 0}
    };
    
    int opt, option_index = 0;

    //Faccio il parsing degli argomenti della riga di comando
    while ((opt = getopt_long(argc, argv, "m:d:s:z:", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'm':
                //Assegno il nome del file delle matrici
                data_filename = optarg;
                break;
            case 'd':
                //Imposto la durata della partita in secondi
                durata = atoi(optarg) * 60;
                break;
            case 's':
                //Imposto il seed per la generazione casuale
                seed = (unsigned int)atoi(optarg);
                break;
            case 'z':
                //Assegno il nome del dizionario
                dizionario = optarg;                
                break;
            default:
                //Se getopt_long ritorna '?' vuol dire che c'è stata un'opzione non riconosciuta
                exit(EXIT_FAILURE);
        }
    }

    //Verifico che non siano utilizzati contemporaneamente il file delle matrici e il seed
    if(strlen(data_filename)>0 && seed!=0) {
        fprintf(stderr, "Non puoi usare data_filename e seed.\n");
        exit(EXIT_FAILURE);
    }

    //Inizializzo il seed per la generazione casuale
    if (seed != 0)
        srand(seed);
    else
        srand(time(NULL));

    //Avvio il server
    server(nome_server, porta_server);
    return 0;
}

//Funzione per inizializzare il server_socket
void server(char *nome_server, int porta_server) {
    struct sockaddr_in server_addr, client_addr;
    int client_socket, retvalue;
    socklen_t addr_size = sizeof(client_addr);

    //Creo la socket del server
    SYSC(server_socket, socket(AF_INET,SOCK_STREAM,0),"nella socket");

    //Imposto i parametri della socket del server
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(nome_server);
    server_addr.sin_port = htons(porta_server);

    //Effettuo il bind della socket del server
    SYSC(retvalue,bind(server_socket,(struct sockaddr*)&server_addr,sizeof(server_addr)),"nella bind");

    //Metto la socket del server in ascolto
    SYSC(retvalue,listen(server_socket,MAX_CLIENTS),"nella listen");
    printf("Connessione server stabilita\n");

    //Creo una struttura sigaction
    struct sigaction sa;
    //Creo una maschera di segnali
    sigset_t maschera;
    //Inizializzo la maschera a un set vuoto
    sigemptyset(&maschera);
    //Aggiungo il segnale SIGINT alla maschera
    SYSC(retvalue, sigaddset(&maschera, SIGINT), "nella sigaction");
    //Aggiungo il segnale SIGALRM alla maschera
    SYSC(retvalue, sigaddset(&maschera, SIGALRM), "nella sigaction");
    //Imposto il gestore del segnale alla funzione handle_sig
    sa.sa_handler = handle_sig;
    //Imposto la maschera di segnali per bloccare SIGINT e SIGALRM durante l'esecuzione del gestore
    sa.sa_mask = maschera;
    //Imposto il flag SA_RESTART per riavviare automaticamente le chiamate di sistema interrotte
    sa.sa_flags = SA_RESTART;
    //Associo la struttura sa al segnale SIGINT
    sigaction(SIGINT, &sa, NULL);
    //Associo la struttura sa al segnale SIGALRM
    sigaction(SIGALRM, &sa, NULL);

    //Imposto i gestori dei segnali SIGINT e SIGALRM
    signal(SIGINT,handle_sigint);
    signal(SIGALRM,handle_alarm);

    //Genero la matrice iniziale
    genera_matrice(data_filename);

    //Entro nel ciclo principale del server per accettare connessioni dei client
    while (1) {
        SYSC(client_socket,accept(server_socket,(struct sockaddr*)&client_addr,&addr_size),"nella accept");
        int *c = malloc(sizeof(int));
        if (c == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        *c = client_socket;

        //Creo un nuovo thread per gestire il client
        pthread_t thread;
        pthread_create(&thread,NULL,thread_client, c);
        //Distacco il thread
        pthread_detach(thread);
    }
}

//Funzione per gestire il client generato
void* thread_client(void* arg) {
    int client_socket = *((int*)arg);
    int registrato = 0;
    message_info msg;

    client_data *client = malloc(sizeof(client_data));
    if (client == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    //Carico il socket client appena connesso
    client->socket = client_socket;
    pthread_mutex_lock(&mutex);
    client->next = client_list;
    client_list = client;
    pthread_mutex_unlock(&mutex);

    //Creo il nodo radice del trie
    TrieNodo *dic = crea_tnodo();

    //Carico il dizionario nel trie
    load_dic(dic, dizionario);

    while (1) {
        receive_message(client_socket, &msg);
        //Gestisco la registrazione del client
        if (msg.type == MSG_REGISTRA_UTENTE) {
            int exist = 0;
            //Verifico se il client è già registrato
            pthread_mutex_lock(&mutex);
            client_data *current = client_list;
            while (current != NULL) {
                if (strcmp(current->username, msg.data) == 0) {
                    exist = 1;
                    break;
                }
                current = current->next;
            }
            pthread_mutex_unlock(&mutex);

            //Registro il client se non è già registrato
            if (!exist) {
                //Aggiungo username e score del client appena registratto all'interno della lista
                pthread_mutex_lock(&mutex);
                client_data *current = client_list;
                while (current != NULL) {
                    if (current->socket == client_socket) {
                        strcpy(current->username,msg.data);
                        current->score = 0;
                        break;
                    }
                    current = current->next;
                }
                pthread_mutex_unlock(&mutex);

                //Invio messaggio di ok
                msg.type = MSG_OK;
                printf("Utente registrato: %s\n",client->username);
                registrato = 1;

                //Se il gioco è in attesa, cambio stato a in gioco
                if (curr_state == STATO_ATTESA) {
                    curr_state = STATO_INGIOCO;
                    inizio_partita = time(NULL);
                    //Faccio partire il gioco
                    alarm(durata);
                }
            } else {
                // Invio un messaggio di errore se l'utente è già registrato
                msg.type = MSG_ERR;
                sprintf(msg.data,"Utente già registrato");
                msg.length = strlen(msg.data);
            }
            send_message(client_socket, &msg);
        } else if (msg.type == MSG_MATRICE && registrato) {
            //Invio la matrice al client
            char buffer[256];

            //Calcolo il tempo rimanente
            int tempo_rimanente = tempo_rim();

            //Se la partita è in gioco invio matrice e tempo rimanente
            if (curr_state == STATO_INGIOCO) {
                matrice_tostring(buffer);
                                
                //Invio la matrice al client
                msg.type = MSG_MATRICE;
                strcpy(msg.data,buffer);
                msg.length = strlen(msg.data);
                send_message(client_socket,&msg);

                //Invio il tempo rimanente al client
                msg.type = MSG_TEMPO_PARTITA;
                sprintf(buffer,"Tempo rimanente:%d secondi",tempo_rimanente);
                strcpy(msg.data, buffer);
                msg.length = strlen(msg.data);
                send_message(client_socket, &msg);
            } else if (curr_state == STATO_PAUSA) {
                //Invio il tempo di attesa al client se la partita è in pausa
                msg.type = MSG_TEMPO_ATTESA;
                sprintf(buffer,"Tempo rimanente inizio nuova partita:%d secondi",tempo_rimanente);
                strcpy(msg.data,buffer);
                msg.length = strlen(msg.data);
                send_message(client_socket,&msg);
            }
            //Pulisco il buffer
            memset(buffer, 0, sizeof(buffer));
        } else if (msg.type == MSG_PAROLA && curr_state != STATO_INGIOCO) {
            //Gestisco l'errore quando il gioco è in pausa e si invia una parola
            msg.type = MSG_ERR;
            sprintf(msg.data, "Gioco in pausa, non puoi inserire parole");
            msg.length = strlen(msg.data);
            send_message(client_socket,&msg);
        } else if (msg.type == MSG_PAROLA && curr_state == STATO_INGIOCO) {
            //Gestisco l'inserimento di una parola valida
            char buffer[16];
            int punteggio = 0;
            msg.type = MSG_PUNTI_PAROLA;

            //Controllo se la parola è già stata presa dal client
            if (is_parola_presa(client->lis,msg.data)) {
                client->score += punteggio;
            } //Controllo se la parola è valida nella matrice 
            else if (is_parola_valida(matrix,msg.data)) {
                //Controllo se la parola è nel dizionario
                if (trova_trie(dic,msg.data)) {
                    aggiungi_parola(&client->lis,msg.data);
                    for (int i=0; i<(int)strlen(msg.data); i++) {
                        if(msg.data[i] == 'Q')
                            continue;    
                        punteggio += 1;
                    }
                    client->score += punteggio;
                } else {
                    //Invio un messaggio di errore se la parola non è nel dizionario
                    msg.type = MSG_ERR;
                    sprintf(msg.data, "Parola non nel dizionario");
                    msg.length = strlen(msg.data);
                    send_message(client_socket,&msg);
                    continue;
                }
            } else {
                //Invio un messaggio di errore se la parola non è valida
                msg.type = MSG_ERR;
                sprintf(msg.data, "Parola non corretta");
                msg.length = strlen(msg.data);
                send_message(client_socket,&msg);
                continue;
            }

            //Aggiorno il punteggio del client
            pthread_mutex_lock(&mutex);
            client_data *current = client_list;
            while (current != NULL) {
                if (current->socket == client_socket) {
                    current->score = client->score;
                    current->lis = client->lis;
                    break;
                }
                current = current->next;
            }
            pthread_mutex_unlock(&mutex);
            
            //Invio il punteggio ottenuto al client
            sprintf(buffer,"%d",punteggio);
            strcpy(msg.data,buffer);
            msg.length = strlen(msg.data);
            send_message(client_socket,&msg);
            memset(buffer,0,sizeof(buffer));
        } else if (msg.type == MSG_PUNTI_FINALI) {
            //Se la partita è in pausa, il client può vedere la classifica
            if(curr_state == STATO_PAUSA) {
                strcpy(msg.data,classifica);
                msg.length = strlen(msg.data);
                send_message(client_socket,&msg);
            } //Se la partita è in corso, il client non può vedere la classifica
            else if (curr_state == STATO_INGIOCO) {
                sprintf(msg.data,"Partita ancora in corso");
                msg.length = strlen(msg.data);
                send_message(client_socket,&msg);
            }
        } 
        else if (msg.type == MSG_CHIUSURA_CLIENT) {
            //Gestisco la chiusura del client
            printf("Chiudo il client\n");
            pthread_mutex_lock(&mutex);
            //Scorro la lista dei client
            client_data *current = client_list;
            client_data *prev = NULL;
            while (current != NULL) {
                //Trovo il socket
                if (current->socket == client_socket) {
                    //Rimuovo tutti i dati del client che si sta disconnettendo
                    if(prev == NULL)
                        client_list = current->next;
                    else
                        prev->next = current->next;
                    while (current->lis != NULL)
                        rimuovi_parola(&(current->lis));
                    free(current);
                    break;
                }
                //Aggiorno la lista dei clients
                prev = current;
                current = current->next;
            }
            pthread_mutex_unlock(&mutex);
            //Se non ci sono client connessi e la partita era in corso, allora metto il server in attesa di nuovi client ai quali generare la partita
            if(client_list == NULL && (curr_state == STATO_INGIOCO || curr_state == STATO_PAUSA)) {
                curr_state = STATO_ATTESA;
                alarm(0);
            }
            return NULL;
        }
    }
}

//Funzione per gestire lo scorer
void *thread_scorer(void *arg) {
    score_t *q = (score_t *)arg;
    message_info msg;

    //Creazione della lista per memorizzare i punteggi
    score_t* scores_head = NULL;
    int count = 0;

    //Elaboro la coda dei punteggi
    while (q != NULL) {
        score_t* score = pop_score(&q);

        //Aggiungo il punteggio alla lista
        score->next = scores_head;
        scores_head = score;
        count++;
    }

    //Converto la lista in un array per ordinarla
    score_t** scores_array = (score_t**)malloc(count*sizeof(score_t*));
    score_t* current_node = scores_head;
    int index = 0;
    while (current_node != NULL) {
        scores_array[index++] = current_node;
        current_node = current_node->next;
    }

    //Ordino i punteggi in ordine decrescente
    qsort(scores_array,count,sizeof(score_t*),compare_scores_final);
    char buffer[1024] = "";

    //Genero la classifica in formato CSV
    strcat(buffer,"Username,Score,Note\n");
    for (int i=0; i<count; i++) {
        if (strcmp(scores_array[i]->username, "") == 0)
            continue;
        strcat(buffer,scores_array[i]->username);
        strcat(buffer,",");
        char score_str[16];
        sprintf(score_str,"%d",scores_array[i]->score);
        strcat(buffer, score_str);
        if (i == 0) {
            strcat(buffer,",Vincitore");
        }
        strcat(buffer,"\n");
    }
    strcpy(classifica,buffer);
    
    //Informo tutti i client che possono richiedere la classifica
    msg.type = MSG_CLASSIFICA_PRONTA;
    client_data *current = client_list;
    pthread_mutex_lock(&mutex);
    while (current != NULL) {
        send_message(current->socket,&msg);
        current = current->next;
    }
    pthread_mutex_unlock(&mutex);

    //Libero memoria allocata
    for (int i=0; i<count; i++)
        free(scores_array[i]);
    free(scores_array);
    free(q);
    pthread_exit(NULL);
}

//Funzione per calcolare il tempo rimanente di gioco o tempo di attesa per una nuova partita
int tempo_rim() {
    //Prendo l'ora corrente
    time_t ora = time(NULL);
    if (curr_state == STATO_INGIOCO) {
        //Calcolo il tempo passato dall'inizio della partita
        int tempo_passato = difftime(ora,inizio_partita);

        //Calcolo il tempo rimanente sottraendo il tempo passato alla durata totale
        int tempo_rimanente = durata-tempo_passato;

        //Restituisco il tempo rimanente
        return tempo_rimanente>0?tempo_rimanente:0;
    } else {
        //Calcolo il tempo rimanente dalla fine della partita più il tempo di attesa
        int tempo_rimanente = difftime(fine_partita+tempo_attesa,ora);
        
        // Restituisco il tempo rimanente
        return tempo_rimanente>0?tempo_rimanente:0;
    }
}


