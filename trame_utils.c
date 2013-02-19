#include "trame_utils.h"

/* Check 
bool checkConnection(char* ip) {
	Trame* checkTrame = creationTrame("big-daddy",CHECK_CON,0,"");
	int otherClientSocketDescriptor;
	hostent* otherHost;
	servent* otherService;
	sockaddr_in adresse_other;
	char* otherHostIP = getIP(mapIP,name);
	printf("ip du contact : %s\n",otherHostIP);
	if((otherHost = gethostbyname(otherHostIP)) == NULL) {
		printf("Impossible de trouver le client %s\n",name);
	}
	
	bcopy((char*)otherHost->h_addr, (char*)&adresse_other.sin_addr, otherHost->h_length);
	adresse_other.sin_family = AF_INET;

}
