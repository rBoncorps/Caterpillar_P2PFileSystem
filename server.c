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
#include "trame_utils.h"
#include "common.h"
 
typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;

Matrice* mapIP;

void* receptionTrame(void* sock) {
	int exitLoop = 0;
	int* tmp = (int*)sock;
	int nouveau_socket_descriptor = *tmp;
	int longueur;
	
	while(!exitLoop) {
		/*char buffer[TAILLE_MAX_TRAME];
		if((longueur=read(nouveau_socket_descriptor,buffer,sizeof(buffer)))<=0) {
			printf("ouch\n");
			return;
		}	
		printf("trame recue\n");
		sleep(5);
		buffer[longueur] = '\0';*/
		
		Trame* revert = receiveTrame(nouveau_socket_descriptor);
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
			name[strlen(name)] = '\0';
			ip[strlen(ip)] = '\0';
			ajouterClient(mapIP,name,ip);
			printf("\n -- Contenu mapIP -- \n");
			afficherMap(mapIP);
			printf(" --			--\n");
			Trame* ackTrame = creationTrame("big-daddy",ACK_CON,0,1,1,"");
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
			int sd = connectTo(name,ip);
			if(sd >= 0) {
				int isConnected = checkConnection(sd);
				if(isConnected == 0) {
					printf("connected !\n");
				}
				else {
					printf("pas connecté 8DDDDDD\n");
				}
			}
			else {
				printf("vraiment pas connecté\n");
			}	
		}
	}
}

int main(){

	pthread_t nouveau_client;

	//creation de la matrice contenant les noms et ip des personnes connectées
	mapIP = newMatrice();
	ajouterClient(mapIP,"_root_","0.0.0.0\0");
	ajouterClient(mapIP,"_admin_\0","0.0.0.0\0");

	int socket_descriptor,
		 nouv_socket_descriptor,
		 longueur_adresse_courante;

	sockaddr_in adresse_locale, adresse_client_courant;
	hostent* ptr_hote;
	servent* ptr_service;
	char machine[TAILLE_MAX_USERNAME+1];

	gethostname(machine,TAILLE_MAX_USERNAME);

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
}
