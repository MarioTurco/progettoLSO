server: server.o lib/boardUtility.o lib/list.o lib/parser.o lib/list.o client.o
	gcc server.o lib/boardUtility.o  lib/list.o lib/parser.o -o server -lpthread
	gcc client.o lib/boardUtility.o lib/parser.o -o client -lpthread



#file da compilare singolarmente
#esegue queste regole quanto c'è un cambiamento al file server.c
#oppure boardUtility.c/.h etcc
list.o: list.c
	gcc -c lib/list.c /lib.h
# -c -> non creare un eseguibile ma solo il file oggetto (.o)
boardUtility.o: boardUtility.c boardUtility.h
	gcc -c lib/boardUtility.c lib/boardUtility.h

parser.o: parser.c parser.h
	gcc -c lib/parser.c

client.o: client.c
	gcc -c client.c lib/boardUtility.c lib/parser.c -lpthread

server.o: server.c 
	gcc -c server.c lib/boardUtility.c lib/parser.c lib/list.c -lpthread
#comando per rimuovere tutti i .o (make clean)
clean:
	rm *.o
