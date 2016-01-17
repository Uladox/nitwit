debug: bin/debug/nitwit_server bin/debug/nitwit_client

bin/debug/nitwit_server: bin/debug bin/debug/ntwt_server.o bin/debug/ntwt_socket.o
	gcc -g -o bin/debug/nitwit_server bin/debug/ntwt_server.o bin/debug/ntwt_socket.o -pthread

bin/debug/ntwt_server.o: bin/debug server/ntwt_server.c
	gcc -g -c -Wall -Werror server/ntwt_server.c -o bin/debug/ntwt_server.o

bin/debug/nitwit_client: bin/debug bin/debug/ntwt_client.o
	gcc -g -o bin/debug/nitwit_client bin/debug/ntwt_client.o bin/debug/ntwt_socket.o

bin/debug/ntwt_client.o: bin/debug client/ntwt_client.c
	gcc -g -c -Wall -Werror client/ntwt_client.c -o bin/debug/ntwt_client.o

bin/debug/ntwt_socket.o: bin/debug shared/socket/ntwt_socket.c shared/socket/ntwt_socket.h
	gcc -g -c -Wall -Werror shared/socket/ntwt_socket.c -o bin/debug/ntwt_socket.o

bin/debug: bin
	mkdir -p bin/debug

bin:
	mkdir -p bin
