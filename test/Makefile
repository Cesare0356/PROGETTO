CC = gcc
CFLAGS = -Wall -Wextra -pthread -I../header
SRCS = test_matrici.c test_messaggi.c test_coda.c
OBJS = $(SRCS:.c=.o)
EXEC_MATRICI = test_matrici
EXEC_MESSAGGI = test_messaggi
EXEC_CODA = test_coda
all: $(EXEC_MATRICI) $(EXEC_MESSAGGI) $(EXEC_CODA)
$(EXEC_MATRICI): test_matrici.o ../matrix_es.o 
	$(CC) test_matrici.o ../matrix_es.o -o $(EXEC_MATRICI) $(CFLAGS)

$(EXEC_MESSAGGI): test_messaggi.o ../includes_es.o 
	$(CC) test_messaggi.o ../includes_es.o -o $(EXEC_MESSAGGI) $(CFLAGS)

$(EXEC_CODA): test_coda.o ../scorer_es.o
	$(CC) test_coda.o ../scorer_es.o -o $(EXEC_CODA) $(CFLAGS)

test_matrici.o: test_matrici.c ../header/matrix.h
	$(CC) $(CFLAGS) -c test_matrici.c -o test_matrici.o

test_messaggi.o: test_messaggi.c ../header/includes.h
	$(CC) $(CFLAGS) -c test_messaggi.c -o test_messaggi.o

test_coda.o: test_coda.c ../header/scorer.h
	$(CC) $(CFLAGS) -c test_coda.c -o test_coda.o

clean:
	rm -f $(OBJS) $(EXEC_MATRICI) $(EXEC_MESSAGGI) $(EXEC_CODA)

#Test matrici
matrici-run:
	@./test_matrici ../dictionary_ita.txt
#Test messaggi socket
messaggi-run:
	@./test_messaggi 
#Test coda giocatori
coda-run:
	@./test_coda

.PHONY: clean