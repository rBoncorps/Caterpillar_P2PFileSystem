#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include "map.h"
#include "trame.h"

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;

Matrice* mapAmi;
Trame* trame;

int main(int argc, char **argv)
{
	
	char* name = "jean paul le poulpe";
	char* ip = "127.0.0.45";
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
	
	trame = creationTrame(CON_SERV,strlen(mesg),mesg);
	
	
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
		perror("erreur : impossilbe de se connecter au serveur.");
		exit(1);
	}

	printf("Connexion établie avec le seveur. \n");

	printf("Authentification... \n");

	if((write(socket_descriptor, (char*)trame, strlen((char*)trame)))<0)
	{
		perror("erreur : impossible d'ecrire le message destine au serveur.");
		exit(1);
	}

	printf("message envoye au serveur. \n");

	while((longueur = read(socket_descriptor, buffer, sizeof(buffer))) > 0)
	{
		printf("reponse du serveur : \n");
		write(1,buffer,longueur);
		break;
	}

	printf("\nfin de la reception. \n");

	sleep(10);
	close(socket_descriptor);

	printf("connexion avec le serveur fermee, fin du programme. \n");	

	exit(0);
}
