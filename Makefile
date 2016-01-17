debug: bin/debug/nitwit_server bin/debug/nitwit_client

bin/debug/nitwit_server: bin/debug/ntwt_server.o bin/debug/ntwt_socket.o
	gcc -g -o bin/debug/nitwit_server bin/debug/ntwt_server.o bin/debug/ntwt_socket.o -pthread

bin/debug/ntwt_server.o: server/ntwt_server.c
	gcc -g -c -Wall -Werror server/ntwt_server.c -o bin/debug/ntwt_server.o

bin/debug/nitwit_client: bin/debug/ntwt_client.o
	gcc -g -o bin/debug/nitwit_client bin/debug/ntwt_client.o bin/debug/ntwt_socket.o

bin/debug/ntwt_client.o: client/ntwt_client.c
	gcc -g -c -Wall -Werror client/ntwt_client.c -o bin/debug/ntwt_client.o

bin/debug/ntwt_socket.o: shared/socket/ntwt_socket.c
	gcc -g -c -Wall -Werror shared/socket/ntwt_socket.c -o bin/debug/ntwt_socket.o

release: bin/release/nitwit_server bin/release/nitwit_client

bin/release/nitwit_server: bin/release/ntwt_server.o bin/release/ntwt_socket.o
	gcc -Ofast -o bin/release/nitwit_server bin/release/ntwt_server.o bin/release/ntwt_socket.o -pthread

bin/release/ntwt_server.o: server/ntwt_server.c
	gcc -Ofast -c -Wall -Werror server/ntwt_server.c -o bin/release/ntwt_server.o

bin/release/nitwit_client: bin/release/ntwt_client.o
	gcc -Ofast -o bin/release/nitwit_client bin/release/ntwt_client.o bin/release/ntwt_socket.o

bin/release/ntwt_client.o: client/ntwt_client.c
	gcc -Ofast -c -Wall -Werror client/ntwt_client.c -o bin/release/ntwt_client.o

bin/release/ntwt_socket.o: shared/socket/ntwt_socket.c
	gcc -Ofast -c -Wall -Werror shared/socket/ntwt_socket.c -o bin/release/ntwt_socket.o
