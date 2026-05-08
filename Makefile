client: client.c capture.c client.h capture.h
	gcc -pthread -o client client.c capture.c -lasound -Wall -Werror
all:
	client