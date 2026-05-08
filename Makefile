client: client.c capture.c config.c client.h capture.h config.h
	gcc -pthread -o client client.c capture.c config.c -lasound -Wall -Werror
all:
	client