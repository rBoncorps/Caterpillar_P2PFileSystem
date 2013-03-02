#include "trame_utils.h"

/*! \brief Try to set a connexion to the given ip.
	\return The opened socket descriptor or -1 if an error occured.
*/
int connectTo(char* name, char* ip) {
	int clientSocketDescriptor;
	hostent* clientHost;
	servent* clientService;
	sockaddr_in clientAdress;
	/* Debug */
	printf("[trame_utils::connect] IP à connecter : %s.\n",ip);
	/* /Debug */
	if((clientHost = gethostbyname(ip)) == NULL) {
		printf("[trame_utils::connect] Impossible de trouver le client %s.",ip);
		return -1;
	}
	bcopy((char*)clientHost->h_addr, (char*)&clientAdress.sin_addr, clientHost->h_length);
	clientAdress.sin_family = AF_INET;
	/* Debug : local dev */
	if(strcmp(name,"bob") == 0) {
		clientAdress.sin_port = htons(5002);
		printf("[trame_utils::connect] Port du contact : 5002.\n",ip);
	}
	else if(strcmp(name,"alice") == 0) {
		clientAdress.sin_port = htons(5003);
		printf("[trame_utils::connect] Port du contact : 5003.\n",ip);
	}
	else {
		clientAdress.sin_port = htons(5001);
		printf("[trame_utils::connect] Port du contact : 5001.\n",ip);
	}
	/* /Debug : local dev */
	if((clientSocketDescriptor = socket(AF_INET,SOCK_STREAM,0)) < 0) {
		printf("[trame_utils::connect] Impossible de créer la socket de connexion avec le client.\n");
		return -1;
	}
	if((connect(clientSocketDescriptor, (sockaddr*)(&clientAdress), sizeof(clientAdress))) < 0) {
		printf("[trame_utils::connect] Impossible de se connecter au client %s.\n",ip);
		return -1;
	}
	return clientSocketDescriptor;
}

/*! \brief Try to open a listening connexion.
	 \return The opened socket descriptor or -1 if an error occured.
*/
int startListening(char* name) {
	int localSocketDescriptor;
	hostent* localHost;
	servent* localService;
	sockaddr_in localAdress;
	char machine[TAILLE_MAX_USERNAME+1];
	gethostname(machine,TAILLE_MAX_USERNAME);
	printf("[trame_utils::startListening] Host Name : %s\n",machine);
	if((localHost=gethostbyname(machine))==NULL){
		perror("[trame_utils::startListening] Impossible de lancer le serveur.\n");
		return -1;
	}
	bcopy((char*)localHost->h_addr,(char*)&localAdress.sin_addr,localHost->h_length);
	localAdress.sin_family = localHost->h_addrtype;
	localAdress.sin_addr.s_addr= INADDR_ANY;
	if(strcmp(name,"bob") == 0) {
		localAdress.sin_port = htons(5002);
		printf("[trame_utils::startListening] Port du contact : 5002.\n");
	}
	else if(strcmp(name,"alice") == 0) {
		localAdress.sin_port = htons(5003);
		printf("[trame_utils::startListening] Port du contact : 5003.\n");
	}
	else {
		localAdress.sin_port = htons(5001);
		printf("[trame_utils::startListening] Port du contact : 5001.\n");
	}
	
	if((localSocketDescriptor = socket(AF_INET , SOCK_STREAM, 0))<0){
		perror("[trame_utils::startListening] Impossible de creer la socket d'écoute.\n");
		return -1;
	}
	if((bind(localSocketDescriptor,(sockaddr*)(&localAdress),sizeof(localAdress)))<0){
		perror("[trame_utils::startListening] Impossible de lier le socket d'écoute.\n");
		return -1;
	}
	listen(localSocketDescriptor,5);
	return localSocketDescriptor;
}

/*! \brief Try to send Trame trame in the socketDescriptor.
    \return 0 if the trame has been sent, -1 if an error occured.
*/
int sendTrame(Trame* trame, int socketDescriptor) {
	if(write(socketDescriptor,(char*)trame,TAILLE_MAX_TRAME) < 0) {
		printf("[trame_utils::sendTrame] Impossible d'envoyer la trame dans la socket %d.\n",socketDescriptor);
		return -1;
	}
	return 0;
}

/*! \brief Read a Trame from the given socketDescriptor.
	\return The received Trame or NULL if an error occured.
	\error Blocks the runtime if no Trame is sent.
*/
Trame* receiveTrame(int socketDescriptor) {
	
	char buffer[TAILLE_MAX_TRAME];
	int bufferSize;
	if((bufferSize=read(socketDescriptor,buffer,TAILLE_MAX_TRAME)) <= 0) {
		printf("[trame_utils::reveiveTrame] Une erreur est survenue à la lecture du socket %d.\n",socketDescriptor);
		return NULL;
	}
	buffer[bufferSize] = '\0';
	Trame* t = (Trame*)&buffer;
	if(t->typeTrame == ENV_FIC) {
		FILE* file1 = NULL;
		file1 = fopen("FichierB/icalsurlapistedumarsupilami.pdf","a");
		fwrite(t->data,sizeof(char),TAILLE_MAX_DATA-1,file1);
		fclose(file1);
	}
	//printf("trame->taille : %d\n",t->taille);
	char* fromName = &(t->nameSrc[0]);
	//char* dataT = &(t->data[0]);
	//return creationTrame(fromName,t->typeTrame,t->taille,t->numTrame,t->nbTrames,t->data);
	Trame* newTrame = (Trame*)malloc(sizeof(Trame));
	newTrame->typeTrame = t->typeTrame;
	strcpy(newTrame->nameSrc,t->nameSrc);
	newTrame->numTrame = t->numTrame;
	newTrame->nbTrames = t->nbTrames;
	newTrame->taille = t->taille;
	//strcpy(newTrame->data, t->data);
	bcopy(t->data,newTrame->data,TAILLE_MAX_DATA);
	//newTrame->data[TAILLE_MAX_DATA] = '\0';
	printf(" ********* Debug ************ \n");
	printf("%s\n\n-------------------------\n\n%s\n",t->data, newTrame->data);
	if(t->typeTrame == ENV_FIC) {
		FILE* file2 = NULL;
		file2 = fopen("FichierB/oubaoubahop.pdf","a");
		fwrite(newTrame->data,sizeof(char),TAILLE_MAX_DATA-1,file2);
		fclose(file2);
	}
	return newTrame;
	//return t;
}


/*! \brief Check if the given is able to answer a request.
	\error Blocks the runtime if the client doesn't answer.
*/
int checkConnection(int socketDescriptor) {
	Trame* checkTrame = creationTrame("big-daddy",CHECK_CON,0,1,1,"");
	sendTrame(checkTrame,socketDescriptor);
	Trame* t = receiveTrame(socketDescriptor);
	if(t->typeTrame == ACK_CON) {
		return 0;
	}
	return -1;
}
