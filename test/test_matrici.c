#include "matrix.h"
char matrix[N][N] = {
    {'A', 'T', 'L', 'C'},
    {'I', 'O', 'Q', 'A'},
    {'D', 'V', 'E', 'S'},
    {'I', 'S', 'B', 'I'}
};
int main(int argc, char *argv[]) {
    if(argc < 2) {
        printf("Uso: %s <dizionario>\n",argv[0]);
        exit(EXIT_FAILURE);
    }
    //Definisco parole
    char *parole[] = {"CASI","QUASI","QUASI","AOEI","QUOTA","AIDI"};
    //Stampo parole
    for(int i=0; i<6; i++) 
        printf("Parola:%s\n",parole[i]);
    printf("\n");
    printf("Inizio gioco\n");
    printf("\n");
    //Creo e riempio trie
    TrieNodo *root = crea_tnodo();
    load_dic(root, argv[1]);
    Nodo *head = NULL;
    //Faccio i controlli
    for(int i=0; i<6; i++) {
        if(is_parola_presa(head,parole[i])) {
            printf("Parola presa\n");
        } else if (is_parola_valida(matrix,parole[i])) {
            if(trova_trie(root,parole[i])) {
                aggiungi_parola(&head,parole[i]);
                printf("Parola corretta\n");
            } else {
                printf("Parola non nel dizionario\n");
            }
        } else {
            printf("Parola non corretta\n");
        }
    }  
    //Libero strutture dati
    free(head);
    free(root);
}