server: server.o list.o boardUtility.o parser.o list.o client.o
	gcc server.o boardUtility.o  list.o parser.o -o server -lpthread
	gcc client.o boardUtility.o parser.o -o client -lpthread

#file da compilare singolarmente
#esegue queste regole quanto c'è un cambiamento al file server.c
#oppure boardUtility.c/.h etcc
list.o: list.c list.h
	gcc -c list.c list.h
# -c -> non creare un eseguibile ma solo il file oggetto (.o)
boardUtility.o: boardUtility.c boardUtility.h list.c list.h
	gcc -c boardUtility.c boardUtility.h

parser.o: parser.c parser.h
	gcc -c parser.c

client.o: client.c list.c list.h
	gcc -c client.c boardUtility.c parser.c list.c list.h -lpthread

server.o: server.c list.c list.h
	gcc -c server.c boardUtility.c parser.c list.c list.h -lpthread
#comando per rimuovere tutti i .o (make clean)
clean:
	rm *.o
