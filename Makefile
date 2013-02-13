client: client.o map.o trame.o
	gcc -o client client.o map.o trame.o -pthread
	
server: server.o map.o trame.o common.o
	gcc -o server server.o map.o trame.o common.o -pthread

server.o: server.c
	gcc -c -Lpthread server.c

client.o: client.c
	gcc -c -Lpthread client.c
	
map.o: map.c map.h
	gcc -c -std=c99 map.c
	
trame.o: trame.c trame.h
	gcc -c -std=c99 trame.c

common.o: common.c common.h
	gcc -c -std=c99 common.c

clean:
	rm -rf *.o client server
