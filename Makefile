client: client.o map.o trame.o trame_utils.o common.o
	g++ -o client client.o map.o trame.o trame_utils.o common.o -pthread
	
server: server.o map.o trame.o common.o trame_utils.o
	g++ -o server server.o map.o trame.o common.o trame_utils.o -pthread

server.o: server.c
	g++ -c -Lpthread server.c

client.o: client.c
	g++ -c -Lpthread client.c
	
map.o: map.c map.h
	g++ -c -std=c99 map.c
	
trame.o: trame.c trame.h
	g++ -c -std=c99 trame.c

common.o: common.c common.h
	g++ -c -std=c99 common.c

trame_utils.o: trame_utils.c trame_utils.h
	g++ -c trame_utils.c

clean:
	rm -rf *.o client server
