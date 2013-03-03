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

		int tailleFichier = 0;
		
		Trame* revert = receiveTrame(nouveau_socket_descriptor);
		if(revert->nbTrames > 1) {
			FILE* file1 = NULL;
			char* file1Path = "FichierB/test.pdf";
			file1 = fopen(file1Path, "w");
			int nbWaitedTrames = revert->nbTrames;
			int nbReceivedTrames = 0;
			Trame** tabTrames = malloc(nbWaitedTrames*sizeof(Trame*));
			tabTrames[nbReceivedTrames] = revert;
			tailleFichier += revert->taille;
			//fwrite(revert->data,sizeof * revert->data,revert->taille,file1);
			nbReceivedTrames++;
			int exitWaitingLoop = 0;
			while (exitWaitingLoop == 0) {
			printf("recue %d\n",48-nbReceivedTrames);
				Trame* trame = receiveTrame(nouveau_socket_descriptor);
				//check if the received trame was the one we were waiting for
				if(trame->numTrame != (nbReceivedTrames + 1)) {
					//wrong trame received
					printf("wrong number received\n");
					sleep(5);
				}
				else{
					tabTrames[nbReceivedTrames] = trame;
					//fwrite(trame->data,sizeof(char),trame->taille,file1);
					tailleFichier += trame->taille;
					nbReceivedTrames++;
					if(nbReceivedTrames == nbWaitedTrames) {
						exitWaitingLoop = 1;
					}
					
				}
			}
			
			char* mesg = extractMessage(tabTrames,nbWaitedTrames);
			//printf("message : %s\n",mesg);
			printf("taille totale : %d\n",tailleFichier);
			if (file1 != NULL) {
				fwrite(mesg,sizeof(char),tailleFichier,file1);
				//fprintf(file,mesg);
				fclose(file1);
			}
			else {
				printf("error opening");
			}
			
			printf("finito dingo\n");
		}
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
			int sent = sendTrame(ackTrame,nouveau_socket_descriptor);
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
			char* ip = getIP(mapIP,name);
			if(ip == NULL) {
				char errorMessage[2000];
				strcpy(errorMessage,"Error : le serveur ne connait pas \0");
				strcat(errorMessage, name);
				strcat(errorMessage, ".\0");
				printf("%s\n",errorMessage);
				Trame* errorTrame = creationTrame("big-daddy", ERROR, strlen(errorMessage), 1, 1, errorMessage);
				int sent = sendTrame(errorTrame, nouveau_socket_descriptor);
				continue;
			}
			int sd = connectTo(name,ip);
			if(sd >= 0) {
				int isConnected = checkConnection(sd);
				if(isConnected == 0) {
					char* response = (char*)malloc(sizeof(char) * (strlen(ip) + strlen(name) + 2));
					strcpy(response,name);
					strcat(response,":\0");
					strcat(response,ip);
					strcat(response,"\0");
					Trame* ackTrame = creationTrame("big-daddy",ACK,strlen(response),1,1,response);
					int sent = sendTrame(ackTrame,nouveau_socket_descriptor);
					printf("connected !\n");
				}
				else {
					char errorMessage[2000];
					strcpy(errorMessage,"Error : \0");
					strcat(errorMessage, name);
					strcat(errorMessage, " n'est pas connecté.\0");
					printf("%s\n",errorMessage);
					Trame* errorTrame = creationTrame("big-daddy", ERROR, strlen(errorMessage), 1, 1, errorMessage);
					int sent = sendTrame(errorTrame, nouveau_socket_descriptor);
					printf("pas connecté 8DDDDDD\n");
				}
			}
			else {
				char errorMessage[2000];
				strcpy(errorMessage,"Error : \0");
				strcat(errorMessage, name);
				strcat(errorMessage, " n'est pas connecté.\0");
				printf("%s\n",errorMessage);
				Trame* errorTrame = creationTrame("big-daddy", ERROR, strlen(errorMessage), 1, 1, errorMessage);
				int sent = sendTrame(errorTrame, nouveau_socket_descriptor);
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
	socket_descriptor = startListening("big-daddy");
	
	sockaddr_in adresse_locale, adresse_client_courant;
	
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
