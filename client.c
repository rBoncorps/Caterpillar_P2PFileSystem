#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include "map.h"
#include "trame.h"

#define TAILLE_MAX_TRAME 256 
#define TAILLE_MAX_NOM 256

#define PORT_SERVEUR_LOCAL 5002
//#define PORT_SERVEUR_LOCAL 5003

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;

Matrice* mapAmi;
Trame* trame;

// test bob = 5002, alice = 5003
char* name = "bob\0";
char* ip = "127.0.0.1\0";
//char* name = "alice\0";
//char* ip = "127.0.0.1\0";

void* handleTrame(void* sock) {
	printf("in handle trame\n");
	int* tmp = (int*)sock;
	int nsocket_descriptor = *tmp;
	int longueur;
	char buffer[TAILLE_MAX_TRAME];
	if((longueur=read(nsocket_descriptor,buffer,sizeof(buffer)))<=0) {
		printf("ouch\n");
		return;
	}	
	Trame* revert;
	revert = (Trame*)&buffer;
	if(revert->typeTrame == CHECK_CON) {
		printf("CHECK_CON from %s\n",revert->nameSrc);
		printf("data : %s\n",revert->data);
		Trame* ackCheckConTrame = creationTrame(name,ACK_CON,0,"");
		printf("writed namesrc : %s\n",name);
		if(write(nsocket_descriptor,(char*)ackCheckConTrame,256) < 0) {
			printf("%s : impossible d'envoyer l'aquittement.\n",name);
		}
		printf("%s : aquittement envoyé\n",name);
	}
	printf("side client trame recue\n");
}

void* boucleReception() {
	//création d'un socket descriptor
	int socket_descriptor, nouveau_socket_descriptor, longueur_adresse_courante;
	hostent* serverHost;
	servent* serverService;
	sockaddr_in adresse_server, adresse_client_courant;
	char machine[TAILLE_MAX_NOM+1];

	gethostname(machine,TAILLE_MAX_NOM);

	if((serverHost=gethostbyname(machine))==NULL) {
		perror("Erreur : impossible de trouver le serveur a partir de ce nom \n");
		exit(1);
	}
	bcopy((char*)serverHost->h_addr,(char*)&adresse_server.sin_addr,serverHost->h_length);
	adresse_server.sin_family = serverHost->h_addrtype;
	adresse_server.sin_addr.s_addr= INADDR_ANY;
	adresse_server.sin_port = htons(PORT_SERVEUR_LOCAL);
	
	if((socket_descriptor = socket(AF_INET , SOCK_STREAM, 0))<0){
		perror("impossible de creer la socket de d'écoute cote client .");
		exit(1);
	}
	
	if((bind(socket_descriptor,(sockaddr*)(&adresse_server),sizeof(adresse_server)))<0){
		perror("impossible de lier la socket à l'adresse de d'écoute cote client .");
		exit(1);
	}
	printf("listen serveur local lancé\n");
	listen(socket_descriptor,5);
	pthread_t handleTrame_th;
	for(;;){
		longueur_adresse_courante = sizeof(adresse_client_courant);
		if((nouveau_socket_descriptor= accept(socket_descriptor,(sockaddr*)(&adresse_client_courant),&longueur_adresse_courante))<0){
			perror("impossible d'accepter la connexion.");
			exit(1);
		}
		printf("%s : sur le point de créer un thread pour le message\n",name);
		if(pthread_create(&handleTrame_th, NULL, handleTrame, (void*)&nouveau_socket_descriptor)) {
			perror("Trop de threads hihihihi");
			return;
		}
	}
	return;
}


int main(int argc, char **argv)
{
	printf("hello %s\n",name);
	int socket_descriptor,longueur;
	sockaddr_in adresse_locale;
	hostent *ptr_host;
	servent *ptr_service;
	
	char buffer[256];
	char * host = "127.0.0.1";
	
	mapAmi = newMatrice();
	
	char* mesg = malloc(strlen(name) + 1 + strlen(ip) + 1);
	strcat(mesg,name);
	strcat(mesg,":");
	strcat(mesg,ip);
	mesg[strlen(mesg)] = '\0';
	
	trame = creationTrame(name,CON_SERV,strlen(mesg),mesg);
	
	printf("Bienvenue dans Caterpillar p2p file system !\n");
	
	if((ptr_host = gethostbyname(host)) == NULL)
	{
		perror("erreur : impossible de trouver le serveur vérifiez votre connexion.");
		exit(1);
	}

	bcopy((char*)ptr_host->h_addr, (char*)&adresse_locale.sin_addr, ptr_host->h_length);
	adresse_locale.sin_family = AF_INET;

	/*if((ptr_service = getservbyname("irc","tcp")) == NULL)
	{
		perror("erreur : impossible de récuperer le numero de port du service desire.");
		exit(1);
	}	
	adresse_locale.sin_port = htons(ptr_service->s_port);
	*/
	
	adresse_locale.sin_port = htons(5001);

	printf("numero de port pour la connexion au serveur : %d \n", ntohs(adresse_locale.sin_port));

	if((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("erreur : impossible de creer la socket de connexion avec le serveur.");
		exit(1);
	}

	if((connect(socket_descriptor, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0)
	{
		perror("erreur : impossible de se connecter au serveur.");
		exit(1);
	}

	printf("Connexion établie avec le seveur. \n");

	printf("Authentification... \n");

	if((write(socket_descriptor, (char*)trame, 256))<0)
	{
		perror("erreur : impossible d'ecrire le message destine au serveur.");
		exit(1);
	}

	printf("message envoye au serveur. \n");

	if((longueur = read(socket_descriptor, buffer, sizeof(buffer))) > 0)
	{
		printf("reponse du serveur : \n");
		Trame* revert = (Trame*)&buffer;
		if(revert->typeTrame == ACK_CON) {
			printf("Recu aquittement de connexion.\n");
		}
	}
	
	// launch the thread handeling messages
	// passer le socket_descriptor direct ? peut être un nouveau socket descriptor
	// ou simplement faire un read comme précédemment
	pthread_t boucle_reception;
	if(pthread_create(&boucle_reception, NULL, boucleReception, NULL)) {
		perror("Trop de threads hihihihi");
		return 1;
	}
	
	// main loop for the client
	char* action = malloc(100*sizeof(char));
	while(strcmp(action,"exit") != 0) {
		printf("Enter a command or type help\n");
		fgets(action,100,stdin);
		if ((strlen(action)>0) && (action[strlen (action) - 1] == '\n')) {
        	action[strlen (action) - 1] = '\0';
        }
		char* actionName = malloc(100*sizeof(char));
		char* parameter = malloc(100*sizeof(char));
		sscanf(action,"%s %s",actionName, parameter);
		if(strcmp(actionName,"add_friend") == 0) {
			printf("want to add %s as a friend\n",parameter);
			trame = creationTrame(name,DEM_AMI,strlen(parameter),parameter);
			if((write(socket_descriptor, (char*)trame, 256))<0)
			{
				perror("erreur : impossible d'ecrire le message destine au serveur.");
				exit(1);
			}
			else {
				printf("trame DEM_AMI envoyée\n");
			}
		}
	}

	printf("\nfin de la reception. \n");
	close(socket_descriptor);

	printf("connexion avec le serveur fermee, fin du programme. \n");	

	exit(0);
}
