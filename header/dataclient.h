#include <pthread.h>
#include <unistd.h>

//Funzioni da implementare
char to_up(char c);
void to_uppstr(char *str);
void handle_fine();
void handle_sig(int sig);
void client_fun(char* nome_server, int porta_server);
