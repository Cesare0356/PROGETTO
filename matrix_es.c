#include "matrix.h"

//Funzione per rendere maiuscolo un carattere
char to_upper(char c) {
    if (c >= 'a' && c <= 'z') {
        return c - ('a' - 'A');
    }
    return c; 
}

//Funzione per generare matrice, sia nel caso random che con file passato da command line
void genera_matrice(char* filename) {
    char caratteri[] = "abcdeghijklmnopqrstuvwxyz";
    //Se il nome del file è vuoto, genero la matrice casualmente
    if(strcmp(filename,"") == 0) {
        for(int i=0; i<N; i++) {
            for(int j=0; j<N; j++)
                matrix[i][j] = to_upper(caratteri[rand() % strlen(caratteri)]);
        }
    } else {
        //Apro il file della matrice
        FILE *file;
        file = fopen(filename,"r");
        if(file == NULL) {
            printf("Errore apertura\n");
            exit(EXIT_FAILURE);
        }
        char c;
        int i=0;
        int j=0;
        int linea = 0;
        
        //Avanzo fino alla linea corrente nel file
        while (linea <= curr_linea && (c = fgetc(file)) != EOF) {
            if (c == '\n')
                linea++;
        }

        //Se raggiungo la fine del file, ricomincio dall'inizio
        if(c == EOF) {
            fseek(file, 0, SEEK_SET);
            curr_linea = -1;
            linea = 0;
        }

        //Riempio la matrice leggendo il file
        while ((c = fgetc(file)) != EOF && i < N) {
            if (c == '\n')
                break;
            if (c == 'Q' || c == 'q') {  
                //Salto la 'U' successiva alla 'Q'
                matrix[i][j] = to_upper(c);  
                j++;
                fgetc(file);  
            } else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
                matrix[i][j] = to_upper(c);
                j++;
            } else
                continue;
            if (j == N) {
                j = 0;
                i++;
            }
        }

        //Aggiorno la linea corrente
        curr_linea = linea;

        //Chiudo il file
        fclose(file);
    }
}

//Converto matrice in stringa
void matrice_tostring(char* buffer) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (matrix[i][j] == 'Q')
                //Aggiungo "Qu" e mantengo puntatore posizione corrente
                buffer += sprintf(buffer, "Qu\t");
            else
                //Aggiungo carattere e mantengo puntatore posizione corrente
                buffer += sprintf(buffer, "%c\t",matrix[i][j]);
        }
        //Aggiungo "\n" e mantengo puntatore posizione corrente
        buffer += sprintf(buffer, "\n");
    }
}

//Aggiungo parola nella lista
void aggiungi_parola(Nodo **head, char *parola) {
    Nodo *new_node = (Nodo*)malloc(sizeof(Nodo));
    if(new_node == NULL) {
        perror("errore malloc");
        exit(EXIT_FAILURE);
    }
    strcpy(new_node->word, parola);
    new_node->next = *head;
    *head = new_node;
}

//Rimuovo parola dalla lista
void rimuovi_parola(Nodo **head){
    if (head == NULL) return;
    Nodo* temp = *head;
    *head = (*head)->next;
    free(temp);
}

//Verifico se la parola è stata presa
int is_parola_presa(Nodo *head, char *parola) {
    Nodo *curr = head;
    //Scorro lista per verificare se la parola è già stata inserita
    while (curr != NULL) {
        if (strcmp(curr->word, parola) == 0)
            return 1;
        curr = curr->next;
    }
    return 0;
}

//Funzione per cercare la parola
int trova_parola(char matrix[N][N], int visited[N][N], char *parola, int index, int x, int y) {
    //1 se tutti i caratteri della parola sono stati trovati all'interno della matrice
    if(index >= (int)strlen(parola))
        return 1;

    //Se la posizione non è valida ritorno 0
    if(!(x>=0 && x<N && y>=0 && y<N && !visited[x][y]))
        return 0;
    
    //Direzioni per muoversi nella matrice
    int dx[N];
    dx[0] = -1; dx[1] = 0; dx[2] = 0; dx[3] = 1;
    int dy[N];
    dy[0] = 0; dy[1] = -1; dy[2] = 1; dy[3] = 0; 
    //-1,0 => alto
    //0,-1 => sinistra
    //0,1=> destra
    //1,0 => basso

    //Se la cella corrente contiene 'Q' e la parola contiene "Qu"
    if(matrix[x][y] == 'Q' && parola[index] == 'Q' && parola[index+1] == 'U') {
        //Segno la cella come visitata
        visited[x][y] = 1;
        //Provo a trovare parola nella direzione specificata
        for(int i=0; i<N; i++) {
            //Vado avanti di 2 caratteri
            if(trova_parola(matrix,visited,parola,index+2,x+dx[i],y+dy[i]))
                return 1;
        }
        //Segno cella come non visitata
        visited[x][y] = 0;
    } //Se la cella corrente contiene il carattere atteso nella parola
    else if (matrix[x][y] == parola[index]) {
        //Stesso procedimento di sopra
        visited[x][y] = 1;
        for (int i = 0; i < N; i++) {
            //Vado avanti di due caratteri
            if (trova_parola(matrix,visited,parola,index+1,x+dx[i],y+dy[i]))
                return 1;
        }
        visited[x][y] = 0;
    }

    //La parola non è stata trovata
    return 0;
}

//Funzione per verificare se una parola è valida
int is_parola_valida(char matrix[N][N], char *parola) {
    //Se la parola è più corta di 4 lettere non è valida
    if(strlen(parola) < 4)
        return 0;

    //Controllo che ogni 'Q' sia seguito da 'U'
    for(int i=0; i<(int)strlen(parola); i++) {
        if(parola[i] == 'Q')
            if(i+1 >= (int)strlen(parola) || parola[i+1] != 'U')
                return 0;
    }

    //Inizializzo matrice dei visitati a 0
    int visited[N][N];
    for (int i=0; i<N; i++) 
        for (int j=0; j<N; j++) 
            visited[i][j] = 0;
    
    //Cerco parola nella matrice partendo da ogni cella
    for(int i=0; i<N; i++) {
        for(int j=0; j<N; j++) {
            if(trova_parola(matrix,visited,parola,0,i,j)) {
                return 1;
            }
        }
    }

    //Non ho trovato la parola
    return 0;
}

//Ottengo indice di un carattere A-Z
int get_index(char c) {
    return c - 'A';
}

//Creo un nuovo nodo nel trie
TrieNodo* crea_tnodo(void) {
    TrieNodo *node = (TrieNodo *)malloc(sizeof(TrieNodo));
    if (node == NULL) {
        printf("Errore allocazione TNODO");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < MAXCHARS; i++) {
        node->children[i] = NULL;
    }
    node->terminal = 0;
    return node;
}

//Inserisco nel trie
void inserisci_trie(TrieNodo *root, char *parola) {
    TrieNodo *node = root;
    while(*parola != '\0') {
        if(*parola >= 'A' && *parola <= 'Z') {
            //Calcolo indice del carattere
            int i = get_index(*parola);
            if(!node->children[i]) {
                //Creo un nuovo nodo se non esiste
                node->children[i] = crea_tnodo();
            }
            node = node->children[i];
        }
        //Salto la 'U' successiva alla 'Q'
        if(*parola == 'Q' && *(parola+1) == 'U')
            parola++;
        parola++;
    }
    //Imposto il flag terminale a 1
    node->terminal = 1;
}

//Funzione per trovare la parola nel trie
int trova_trie(TrieNodo *node, char *parola) {
    //Ritorno il flag terminale se raggiungo la fine della parola
    if(*parola == '\0')
        return node->terminal;
    if(*parola >= 'A' && *parola <= 'Z') {
        int i = get_index(*parola);
        //Se il figlio non esiste ritorno 0
        if(node->children[i] == NULL)
            return 0;
        //Salto la 'U' successiva alla 'Q'
        if(*parola == 'Q' && *(parola+1)  == 'U')
            return trova_trie(node->children[i],parola+2);
        return trova_trie(node->children[i],parola+1);
    }
    //Parola non valida restituisco 0
    return 0;
}

//Funzione per caricare il dizionario nel trie
void load_dic(TrieNodo *root, char *nomefile) {
    //Apro il dizionario
    FILE *file = fopen(nomefile,"r");
    if(file == NULL) {
        printf("Errore apertura dizionario\n");
        exit(EXIT_FAILURE);
    }
    char parola[256];
    //Leggo ogni parola del file e la inserisco nel trie
    while (fscanf(file,"%s",parola) != EOF) {
        //Converto parola in maiuscolo
        for(int i=0; parola[i]; i++)
            parola[i] = to_upper(parola[i]);
        inserisci_trie(root,parola);
    }
    //Chiudo il file
    fclose(file);
}