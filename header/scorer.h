#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
//Nodo scorer
typedef struct score_t {
    char username[11];
    int score;
    struct score_t* next;
} score_t;

//Funzioni da implementare
void push_score(score_t** head, char* username, int score);
score_t* pop_score(score_t** head);
int compare_scores_final(const void* a, const void* b);
