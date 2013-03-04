#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include "map.h"
#include "trame.h"
#include "trame_utils.h"
#include "common.h"

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;

Matrice* mapAmi;
Trame* trame;

// test bob = 5002, alice = 5003
//char* name = "bob\0";
//char* ip = "127.0.0.1\0";
char* name = "alice\0";
char* ip = "127.0.0.1\0";

void* handleTrame(void* sock) {
	printf("in handle trame\n");
	int* tmp = (int*)sock;
	int socketDescriptor = *tmp;
	int longueur;
	char buffer[TAILLE_MAX_TRAME];
	Trame* received = receiveTrame(socketDescriptor);
	if(received->typeTrame == CHECK_CON) {
		printf("CHECK_CON from %s\n",received->nameSrc);
		printf("data : %s\n",received->data);
		Trame* ackCheckConTrame = creationTrame(name,ACK_CON,0,1,1,"");
		printf("writed namesrc : %s\n",name);
		sendTrame(ackCheckConTrame,socketDescriptor);
	}
	else if(received->typeTrame == DEM_FIC) {
		printf("received a DEM_FIC trame\n");
		// try to open the wanted file
		FILE* file = NULL;
		printf("Try to open %s file\n",received->data);
		//file = fopen(received->data, "r");
		file = fopen("FichierA/Spec_Communication.pdf","r");
		if(file == NULL) {
			char errorMessage[2000];
			strcpy(errorMessage,"Erreur : impossible d'ouvrir le fichier \0");
			strcat(errorMessage, received->data);
			strcat(errorMessage, ".\0");
			printf("%s\n",errorMessage);
			Trame* errorTrame = creationTrame(name,ERROR,strlen(errorMessage),1,1,errorMessage);
			int sent = sendTrame(errorTrame,socketDescriptor);
		}
		else {
			// here is the core of the application
			int pos = fseek(file,0,SEEK_END);
			int size = ftell(file);
			rewind(file);
			char* buffer = malloc((size) * sizeof(char));
			printf("size : %d\n",size);
			int longLu = 0;
			longLu = fread( buffer , sizeof(char) , size , file );
			sleep(2);
			fclose(file);
			int nbTrames;
			sleep(2);
			Trame** tramesMesg = decoupageTrame(name,ENV_FIC,size, buffer,&nbTrames); 
			int i = 0;
			printf("nbTrames : %d\n",nbTrames);
			for(i ; i < nbTrames ; i++) {
				int retValue = sendTrame(tramesMesg[i],socketDescriptor);
				sleep(2);
			}
		}
	}
	printf("side client trame recue\n");
}

void* boucleReception() {
	//création d'un socket descriptor
	int socket_descriptor, nouveau_socket_descriptor, longueur_adresse_courante;
	sockaddr_in adresse_server, adresse_client_courant;
	socket_descriptor = startListening(name);
	
	pthread_t handleTrame_th;
	for(;;) {
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
	char * host = "127.0.0.1";
	
	socket_descriptor = connectTo("big-daddy",host);
	
	mapAmi = newMatrice();
	
	char* mesg = malloc(strlen(name) + 1 + strlen(ip) + 1);
	strcat(mesg,name);
	strcat(mesg,":");
	strcat(mesg,ip);
	mesg[strlen(mesg)] = '\0';
	
	trame = creationTrame(name,CON_SERV,strlen(mesg),1,1,mesg);
	int sent = sendTrame(trame, socket_descriptor);
	
	printf("Bienvenue dans Caterpillar p2p file system !\n");
	
	Trame* ackTrame = receiveTrame(socket_descriptor);
	if(ackTrame->typeTrame == ACK_CON) {
		printf("Recu un aquittement de connexion.\n");
	}
	
	// Launch the thread handeling incoming messages
	pthread_t boucle_reception;
	if(pthread_create(&boucle_reception, NULL, boucleReception, NULL)) {
		perror("Impossible de créer un nouveau thread.\n");
		return 1;
	}
	
	// User main loop. Contains the command interpreter and call
	// related actions.
	char* action = malloc(100*sizeof(char));
	while(strcmp(action,"exit") != 0) {
		printf("Enter a command or type help\n");
		fgets(action,100,stdin);
		if ((strlen(action)>0) && (action[strlen (action) - 1] == '\n')) {
        	action[strlen (action) - 1] = '\0';
        }
		char* actionName = malloc(100*sizeof(char));
		char* parameter = malloc(100*sizeof(char));
		char* parameter2 = malloc(100*sizeof(char));
		sscanf(action,"%s %s %s",actionName, parameter, parameter2);
		
		if(strcmp(actionName,"add_friend") == 0) {
			printf("want to add %s as a friend\n",parameter);
			trame = creationTrame(name,DEM_AMI,strlen(parameter),1,1,parameter);
			int sent = sendTrame(trame, socket_descriptor);
			Trame* response = receiveTrame(socket_descriptor);
			if(response->typeTrame == ERROR) {
				printf("%s\n",response->data);
			}
			else if(response->typeTrame == ACK) {
				char* responseName;
				char* responseIP;
				extractNameIP(response->data,&responseName,&responseIP);
				printf("ACK for DEM_AMI. %s has the adress %s\n",responseName,responseIP);
				ajouterClient(mapAmi,responseName, responseIP);
				afficherMap(mapAmi);
			}
		}
		if(strcmp(actionName,"get_file") == 0) {
			char* contactIP = getIP(mapAmi,parameter);
			if(contactIP == NULL) {
				printf("Le contact %s n'est pas dans votre liste d'amis, veuillez-utiliser <add_friend %s>\n",parameter,parameter);
				continue;
			}
			printf("want to get file %s from the friend %s(%s)\n",parameter2, parameter,contactIP);
			int contactSocket = connectTo(parameter, contactIP);
			Trame* demFicTrame = creationTrame(name,DEM_FIC,strlen(parameter2),1,1,parameter2);
			int sent = sendTrame(demFicTrame,contactSocket);
			Trame* contactResponse = receiveTrame(contactSocket);
			if(contactResponse->typeTrame == ERROR) {
				printf("%s\n",contactResponse->data);
				continue;
			}
			
			int tailleFichier = 0;
			if(contactResponse->nbTrames > 1) {
				FILE* file = NULL;
				char* filePath = "FichierB/tagada.pdf"; // for debugging
				file = fopen(filePath, "w");
				int nbWaitedTrames = contactResponse->nbTrames;
				int nbReceivedTrames = 0;
				Trame* currentTrame = contactResponse;
				//Trame** tabTrames = malloc(nbWaitedTrames*sizeof(Trame*));
				//tabTrames[nbReceivedTrames] = contactResponse;
				tailleFichier += currentTrame->taille;
				fwrite(currentTrame->data,sizeof(char),currentTrame->taille,file);
				//fwrite(revert->data,sizeof * revert->data,revert->taille,file1);
				nbReceivedTrames++;
				int exitWaitingLoop = 0;
				while (exitWaitingLoop == 0) {
					printf("recue %d\n",nbReceivedTrames);
					//Trame* trame = receiveTrame(contactSocket);
					currentTrame = receiveTrame(contactSocket);
					//check if the received trame was the one we were waiting for
					if(currentTrame->numTrame != (nbReceivedTrames + 1)) {
					//if(trame->numTrame != (nbReceivedTrames + 1)) {
						//wrong trame received
						printf("wrong number received\n");
						sleep(5);
					}
					else{
						//tabTrames[nbReceivedTrames] = trame;
						fwrite(currentTrame->data,sizeof(char),currentTrame->taille,file);
						//tailleFichier += trame->taille;
						tailleFichier += trame->taille;
						nbReceivedTrames++;
						if(nbReceivedTrames == nbWaitedTrames) {
							exitWaitingLoop = 1;
						}
					
					}
				}
				fclose(file);
			
				/*char* mesg = extractMessage(tabTrames,nbWaitedTrames);
				//printf("message : %s\n",mesg);
				printf("taille totale : %d\n",tailleFichier);
				if (file != NULL) {
					fwrite(mesg,sizeof(char),tailleFichier,file);
					//fprintf(file,mesg);
					fclose(file);
				}
				else {
					printf("error opening");
				}*/
			}
		}

		if(strcmp(actionName,"testFichier") == 0) {
			printf("want to test file Trame\n");
			FILE* file = NULL;
			char* filePath = "FichierA/Spec_Communication.pdf";
			file = fopen(filePath, "r");
			char* contenu = NULL;
			int longueurTotale = 0;
			int firstTime = 0;
			
			if (file != NULL)
			{
				// On peut lire et écrire dans le fichier
				int pos = fseek(file,0,SEEK_END);
				int size = ftell(file);
				rewind(file);
				char* buffer = malloc((size) * sizeof(char));
				printf("size : %d\n",size);
				int longLu = 0;
				longLu = fread( buffer , sizeof(char) , size , file );
				sleep(2);
				fclose(file);
				//while((longLu = fread( buffer , sizeof(char) , size , file )) > 0) {
					/*longueurTotale += longLu;
					printf("longLu: %d\n",longLu);
					if(firstTime == 0) {
						contenu = malloc(longLu*sizeof(char));
						firstTime = 1;
					}
					else {
						char* tmp = (char*)realloc(contenu,longueurTotale*sizeof(char));
						if (tmp = NULL) {
							contenu = tmp;
						}
					}
					strncat(contenu,buffer,longLu);
					*/
					
					/*realloc(contenu,(longueurTotale)*sizeof(char));
					contenu[longueurTotale] = '\0';
					printf("contenu : %s\n",contenu);
					strcat(contenu,buffer);
					*/
				//}
				//printf("contenu : %s\n",contenu);
				
				int nbTrames;
				sleep(2);
				Trame** tramesMesg = decoupageTrame(name,ENV_FIC,size, buffer,&nbTrames); 
				int i = 0;
				printf("nbTrames : %d\n",nbTrames);
				FILE* file1 = NULL;
				file1 = fopen("FichierB/ccccdelamerde.pdf","w");
				for(i ; i < nbTrames ; i++) {
					//printf("trame %d envoye\n : data -> %s\n",i,tramesMesg[i]->data);
					int retValue = sendTrame(tramesMesg[i],socket_descriptor);
					fwrite(tramesMesg[i]->data,sizeof(char),TAILLE_MAX_DATA-1,file1);
					sleep(2);
				}
				fclose(file1);
			}
			else
			{
				// On affiche un message d'erreur si on veut
				printf("Impossible d'ouvrir le fichier");
			}
			
		}
	}

	printf("\nfin de la reception. \n");
	close(socket_descriptor);

	printf("connexion avec le serveur fermee, fin du programme. \n");	

	exit(0);
}
