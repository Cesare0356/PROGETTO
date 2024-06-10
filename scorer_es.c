#include "scorer.h"
pthread_mutex_t mutex_score = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_score = PTHREAD_COND_INITIALIZER;

//Funzione per inserire un punteggio nella coda
void push_score(score_t **head, char* username, int score) {
    //Alloco memoria per il punteggio
    score_t* s = malloc(sizeof(score_t));
    if (s == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    //Copio i dati
    strcpy(s->username, username);
    s->score = score;
    s->next = NULL;

    //Aggiungo punteggio alla coda
    pthread_mutex_lock(&mutex_score);
    if (head == NULL) {
        *head = s;
    } else {
        score_t* current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = s;
    }
    pthread_cond_signal(&cond_score);
    pthread_mutex_unlock(&mutex_score);
}

//Funzione per estrarre un punteggio dalla coda
score_t* pop_score(score_t **head) {
    pthread_mutex_lock(&mutex_score);

    //Aspetto che ci siano punteggi nella coda
    while (*head == NULL)
        pthread_cond_wait(&cond_score, &mutex_score);
    
    //Estraggo punteggio dalla coda
    score_t* s = *head;
    *head = s->next;
    pthread_mutex_unlock(&mutex_score);
    return s;
}

//Funzione per comparare i risultati nella qsort dello scorer
int compare_scores_final(const void* a, const void* b) {
    score_t* scoreA = *(score_t**)a;
    score_t* scoreB = *(score_t**)b;
    return scoreB->score - scoreA->score;
}
