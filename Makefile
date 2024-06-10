CC = gcc
CFLAGS = -Wall -Wextra -pthread -Iheader
SRCS = includes_es.c matrix_es.c paroliere_cl.c paroliere_srv.c scorer_es.c
OBJS = $(SRCS:.c=.o)
EXEC_CL = paroliere_cl
EXEC_SRV = paroliere_srv

all: $(EXEC_CL) $(EXEC_SRV)
$(EXEC_CL): paroliere_cl.o includes_es.o matrix_es.o scorer_es.o
	$(CC) paroliere_cl.o includes_es.o matrix_es.o scorer_es.o -o $(EXEC_CL) $(CFLAGS)

$(EXEC_SRV): paroliere_srv.o includes_es.o matrix_es.o scorer_es.o
	$(CC) paroliere_srv.o includes_es.o matrix_es.o scorer_es.o -o $(EXEC_SRV) $(CFLAGS)

includes_es.o: includes_es.c header/includes.h
	$(CC) $(CFLAGS) -c includes_es.c -o includes_es.o

matrix_es.o: matrix_es.c header/matrix.h
	$(CC) $(CFLAGS) -c matrix_es.c -o matrix_es.o

paroliere_cl.o: paroliere_cl.c header/includes.h header/dataclient.h
	$(CC) $(CFLAGS) -c paroliere_cl.c -o paroliere_cl.o

paroliere_srv.o: paroliere_srv.c header/dataserver.h header/matrix.h header/includes.h header/scorer.h
	$(CC) $(CFLAGS) -c paroliere_srv.c -o paroliere_srv.o

scorer_es.o: scorer_es.c header/scorer.h
	$(CC) $(CFLAGS) -c scorer_es.c -o scorer_es.o

clean:
	rm -f $(OBJS) $(EXEC_CL) $(EXEC_SRV)

#Avvio programma nei vari modi

client-run:
	@./paroliere_cl 127.0.0.1 1024
#Test con solo parametri obbligatori
server-run-noparams:
	@./paroliere_srv 127.0.0.1 1024
#Test con --matrici
server-run-matrici:
	@./paroliere_srv 127.0.0.1 1024 --matrici matrici.txt
#Test con --durata
server-run-durata:
	@./paroliere_srv 127.0.0.1 1024 --durata 1
#Test con --seed
server-run-seed:
	@./paroliere_srv 127.0.0.1 1024 --seed 12345
#Test con --diz
server-run-diz:
	@./paroliere_srv 127.0.0.1 1024 --diz dic.txt
#Test con --matrici e --durata
server-run-matrici-durata:
	@./paroliere_srv 127.0.0.1 1024 --matrici matrici.txt --durata 1
#Test con --matrici e --diz
server-run-matrici-diz:
	@./paroliere_srv 127.0.0.1 1024 --matrici matrici.txt --diz dic.txt
#Test con --durata e --diz
server-run-durata-diz:
	@./paroliere_srv 127.0.0.1 1024 --durata 1 --diz dic.txt
#Test con --seed e --durata
server-run-seed-durata:
	@./paroliere_srv 127.0.0.1 1024 --seed 12345 --durata 1
#Test con --seed e --diz
server-run-seed-diz:
	@./paroliere_srv 127.0.0.1 1024 --seed 12345 --diz dic.txt
#Test con --durata, --diz e --seed
server-run-durata-diz-seed:
	@./paroliere_srv 127.0.0.1 1024 --durata 1 --diz dic.txt --seed 12345
#Test con --matrici, --durata e --diz
server-run-matrici-durata-diz:
	@./paroliere_srv 127.0.0.1 1024 --matrici matrici.txt --durata 1 --diz dic.txt
#Test con --matrici e --seed
server-run-matrici-seed:
	@./paroliere_srv 127.0.0.1 1024 --matrici matrici.txt --seed 12345

.PHONY: clean