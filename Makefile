client: client.o map.o trame.o
	gcc -o client client.o map.o trame.o -pthread
	
server: server.o map.o trame.o
	gcc -o server server.o map.o trame.o -pthread

server.o: server.c
	gcc -c -std=c99 -Lpthread server.c

client.o: client.c
	gcc -c -std=c99 -Lpthread client.c
	
map.o: map.c map.h
	gcc -c -std=c99 map.c
	
trame.o: trame.c trame.h
	gcc -c -std=c99 trame.c
clean:
	rm -rf *.o client server
