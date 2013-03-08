server: server.o trame.o socketmanager.o receivehandler.o serverreceivehandler.o
	g++ -o server server.o trame.o socketmanager.o receivehandler.o serverreceivehandler.o -pthread
	
client: client.o trame.o socketmanager.o receivehandler.o
	g++ -o client client.o trame.o socketmanager.o receivehandler.o -pthread

server.o: server.cpp serverreceivehandler.h socketmanager.h trame.h common.h
	g++ -c -Lpthread server.cpp

trame.o: trame.cpp trame.h common.h
	g++ -c trame.cpp

socketmanager.o: socketmanager.cpp socketmanager.h common.h trame.h
	g++ -c socketmanager.cpp

receivehandler.o: receivehandler.cpp receivehandler.h socketmanager.h common.h trame.h
	g++ -c receivehandler.cpp
	
serverreceivehandler.o: serverreceivehandler.cpp serverreceivehandler.h receivehandler.h
	g++ -c serverreceivehandler.cpp

clean:
	rm -rf *.o client server
