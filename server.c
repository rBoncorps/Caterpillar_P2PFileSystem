#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>
#include "map.h"
#include "trame.h"
#include "common.h"
#define TAILLE_MAX_NOM 256
#define TAILLE_MAX_TRAME 256 
typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;

Matrice* mapIP;

void* receptionTrame(void* sock){
	int exitLoop = 0;
	int* tmp = (int*)sock;
	int nouveau_socket_descriptor = *tmp;
	int longueur;
	
	while(!exitLoop) {
		char buffer[TAILLE_MAX_TRAME];
		if((longueur=read(nouveau_socket_descriptor,buffer,sizeof(buffer)))<=0) {
			printf("ouch\n");
			return;
		}	
		printf("trame recue\n");
		sleep(5);
		buffer[longueur] = '\0';
		Trame* revert;
		revert = (Trame*)&buffer;
		if(revert->typeTrame == CON_SERV) {
			printf("Received a CON_SERV trame type\n");
			printf("Datas : %s\n",revert->data);
			char* name;
			char* ip;
			if(extractNameIP(revert->data,&name,&ip) < 0) {
				printf("Error during name & ip extraction\n");
				return;
			}
			printf("extracted infos - name : %s, ip : %s\n",name, ip);
			ajouterClient(mapIP,name,ip);
			printf("\n -- Contenu mapIP -- \n");
			afficherMap(mapIP);
			printf(" --			--\n");
			Trame* ackTrame = creationTrame("big-daddy",ACK_CON,0,"");
			if(write(nouveau_socket_descriptor,(char*)ackTrame,256) < 0) {
				perror("Erreur : impossible d'envoyer l'aquittement de connexion.\n");
				return;
			}
		}
		if(revert->typeTrame == DEM_AMI) {
			printf("Received a DEM_AMI trame type\n");	
			printf("Datas : %s\n",revert->data);
			char* name;
			if(extractAskedFriendName(revert->data,revert->taille,&name) < 0) {
				printf("Error during asked friend name extraction\n");
				return;
			}
			printf("extracted infos - name : %s\n",name);
			printf("trame from : %s\n",revert->nameSrc);
			char* nameIP = getIP(mapIP,name);
			if(nameIP == NULL) {
				printf("Error, the server doesn't know %s\n",name);
				return;
			}
			// Vérification que $name est connecté
			char* ip = getIP(mapIP,name);
			bool isConnected = checkConnection(ip);
			
			//Trame* verifConnectionTrame = creationTrame("big-daddy",CHECK_CON,0,"");
			
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
			// test pour dev local
			if(strcmp(name,"bob") == 0) {
				adresse_other.sin_port = htons(5002);
				printf("port du contact : 5002\n");
			}
			else if(strcmp(name,"alice") == 0) {
				adresse_other.sin_port = htons(5003);
				printf("port du contact : 5003\n");
			}
			else {
				printf("Client inconnu (dev local) \n");
			}
			
			if((otherClientSocketDescriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			{
				printf("erreur : impossible de creer la socket de connexion avec le client %s\n.",name);
				exit(1);
			}
			
			if((connect(otherClientSocketDescriptor, (sockaddr*)(&adresse_other), sizeof(adresse_other))) < 0)
			{
				printf("erreur : impossible de se connecter au client %s\n.",name);
				exit(1);
			}
			
			printf("connexion établie avec %s\n",name);
			if(write(otherClientSocketDescriptor,(char*)verifConnectionTrame,256) < 0) {
				perror("Erreur : impossible d'envoyer l'aquittement de connexion.\n");
				return;
			}
			char otherBuffer[TAILLE_MAX_TRAME];
			int otherLongueur;
			if((otherLongueur=read(otherClientSocketDescriptor,otherBuffer,sizeof(otherBuffer)))<=0) {
				printf("ouch\n");
				return;
			}
			printf("recu quelque chose\n");
			otherBuffer[otherLongueur] = '\0';
			Trame* ackCheckConTrame;
			ackCheckConTrame = (Trame*)&otherBuffer;
			printf("de %s\n",ackCheckConTrame->nameSrc);
			printf("%u\n",ackCheckConTrame->typeTrame);
			if(ackCheckConTrame->typeTrame == DEM_AMI) {
				printf("dem ami ...\n");
			}
			else if(ackCheckConTrame->typeTrame == ACK_CON) {
				printf("ack con ...\n"); 
			}
			printf("%s\n",ackCheckConTrame->data);
			
			if(ackCheckConTrame->typeTrame == ACK_CON) {
				printf("Recu un aquittement de %s\n",ackCheckConTrame->nameSrc);	
			}

			
			/*if(write(nouveau_socket_descriptor,(char*)verifConnectionTrame,256) < 0) {
				perror("Erreur : impossible d'envoyer la vérification de connexion");
			}*/
			/* TODO
			 *	Verifier que $name est connecté
			 * 	Envoi requete demande d'ami a $name de la part de client du socket
			 * 	Attente reponse
			 * 	si OK -> envoi IP de $name pour ajout a la mapAmi du client
			 *  sinon -> envoi trame refus
			*/
		}
	}
}

int main(){

pthread_t nouveau_client;

//creation de la matrice contenant les noms et ip des personnes connectées
mapIP = newMatrice();

int socket_descriptor,
    nouv_socket_descriptor,
    longueur_adresse_courante;

sockaddr_in adresse_locale, adresse_client_courant;
hostent* ptr_hote;
servent* ptr_service;
char machine[TAILLE_MAX_NOM+1];

gethostname(machine,TAILLE_MAX_NOM);

if((ptr_hote=gethostbyname(machine))==NULL){

	perror("Erreur : impossible de trouver le serveur a partir de ce nom \n");
	exit(1);
}

bcopy((char*)ptr_hote->h_addr,(char*)&adresse_locale.sin_addr,ptr_hote->h_length);
adresse_locale.sin_family = ptr_hote->h_addrtype;
adresse_locale.sin_addr.s_addr= INADDR_ANY;
adresse_locale.sin_port = htons(5001);

printf("Numero de port pour la connexion au serveur : %d   \n", ntohs(adresse_locale.sin_port));

if((socket_descriptor = socket(AF_INET , SOCK_STREAM, 0))<0){
	perror("impossible de creer la socket de connexion avec le client .");
	exit(1);
}

if((bind(socket_descriptor,(sockaddr*)(&adresse_locale),sizeof(adresse_locale)))<0){
	perror("impossible de lier la socket à l'adresse de connexion avec le client .");
	exit(1);
}

listen(socket_descriptor,5);

for(;;){
	longueur_adresse_courante = sizeof(adresse_client_courant);
	if((nouv_socket_descriptor= accept(socket_descriptor,(sockaddr*)(&adresse_client_courant),&longueur_adresse_courante))<0){
		perror("impossible d'accepter la connexion avec le client .");
		exit(1);
	}
	if(pthread_create(&nouveau_client, NULL, receptionTrame, (void*)&nouv_socket_descriptor)) {
		perror("Trop de threads hihihihi");
		return 1;
	}
}

close(socket_descriptor);

return 0;
}





