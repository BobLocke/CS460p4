all: rag

rag:
	gcc -std=c99 -g rag.c -o scheduler

clean:
	rm rag *.o
