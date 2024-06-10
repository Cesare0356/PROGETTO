#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define N 4
#define MAXCHARS 26
char matrix[N][N];
//Struttura dati parole
typedef struct Nodo {
    char word[16];
    struct Nodo *next;
} Nodo;

//Struttura dati trie
typedef struct TrieNodo {
    struct TrieNodo *children[MAXCHARS];
    int terminal;
} TrieNodo;

//Intero statico per scorrere le linee del file matrici.txt
static int curr_linea = -1;

//Funzioni da implementare
void genera_matrice(char *filename);
void matrice_tostring(char *buffer);
void aggiungi_parola(Nodo** head,char *parola);
void rimuovi_parola(Nodo** head);
int is_parola_presa(Nodo *head,char *parola);
int trova_parola(char matrix[N][N], int visited[N][N], char *parola, int index, int x, int y);
int is_parola_valida(char matrix[N][N], char *parola);
TrieNodo* crea_tnodo(void);
void inserisci_trie(TrieNodo *root, char *parola);
int trova_trie(TrieNodo *node, char *parola);
void load_dic(TrieNodo *root, char *nomefile);