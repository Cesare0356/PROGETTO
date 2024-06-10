#include "scorer.h"
char c = 'a';
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
score_t *queue;

//Thread per gestione client
void* client_thread(void* arg) {
    free(arg);
    pthread_mutex_lock(&mutex);
    char user[2];
    int score;
    //Assegno un nome all'utente
    strcpy(user,&c);
    user[1] = '\0';
    c++;
    //Genero uno score casuale
    score = rand()%50;
    //Lo aggiungo nella coda
    push_score(&queue,user,score);
    printlist(queue);
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

int main(void) {
    //Definisco thread
    pthread_t t[4];

    srand(time(NULL));
    
    queue = malloc(sizeof(score_t));

    //Creo i thread
    for(int i=0; i<4; i++)
        pthread_create(&t[i],NULL,client_thread,NULL);
    
    //Attendo terminazione
    for(int i=0; i<4; i++)
        pthread_join(t[i],NULL);

    score_t** scores = malloc(4*sizeof(score_t*));
    if(scores == NULL) {
        perror("Errore malloc");
        exit(EXIT_FAILURE);
    }
    for(int i=0; i<5; i++) 
        scores[i] = pop_score(&queue);

    qsort(scores,5,sizeof(score_t*),compare_scores_final);
    char buffer[1024] = "";

    //Genero la classifica in formato CSV
    strcat(buffer,"Username,Score,Note\n");
    for (int i=0; i<5; i++) {
        if (strcmp(scores[i]->username,"") == 0)
            continue;
        strcat(buffer,scores[i]->username);
        strcat(buffer,",");
        char score_str[16];
        sprintf(score_str,"%d",scores[i]->score);
        strcat(buffer,score_str);
        if (i==0)
            strcat(buffer,",Vincitore");
        strcat(buffer,"\n");
    }
    printf("%s",buffer);
    for(int i=0; i<4; i++)
        free(scores[i]);
    free(scores);

}