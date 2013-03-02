#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include "map.h"
#include "trame.h"
#include "trame_utils.h"

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
		sscanf(action,"%s %s",actionName, parameter);
		
		if(strcmp(actionName,"add_friend") == 0) {
			printf("want to add %s as a friend\n",parameter);
			trame = creationTrame(name,DEM_AMI,strlen(parameter),1,1,parameter);
			int sent = sendTrame(trame, socket_descriptor);
			Trame* response = receiveTrame(socket_descriptor);
			if(response->typeTrame == ERROR) {
				printf("%s\n",response->data);
			}
		}
		
		
		if(strcmp(actionName,"testMulti") == 0) {
			printf("want to test multiTrame\n");
			char* mesg = "Mais cette capacité ne se maîtrise pas sans efforts. Il est necessaire d'avoir des connaissances et des compétences acquise tout au long de sa carrière. A commencer par la formation. A l'université de Nantes, le master ALMA orienté génie logiciel prône la gestion collaborative de projet. Ceci nous apprend à travailler en équipe tout au long de l'année pour réaliser des projets sur des sujets techniques les plus variés. Cette méthode de travail renforce la capacité d'organisation et de rigueur ainsi que la capacité d'adaptation.C'est un cursus qui responsabilise ses élèves en les projetant dans le monde du travail, les poussant a prendre des initiatives.	Ensuite viens l'expérience, avec une vraie confrontation à l'entreprise. Lors du stage de fin d'étude de Licence en informatique, j'ai eu l'occasion d'être intergré dans un processus de développement logiciel. Rejoindre une équipe nécessite d'avoir un tempérament social et parfois même médiateur en cas de conflits. Une volonté d'intéragir avec les membres du groupe et de dissiper les tensions.\0";
			//mesg[strlen(mesg)] = '\0';
			//printf("texte a envoyer : %s , longueur %ld\n",mesg,strlen(mesg));
			int nbTrames;
			Trame** tramesMesg = decoupageTrame(name,ENV_FIC,strlen(mesg), mesg,&nbTrames); 
			int i = 0;
			for(i ; i < nbTrames ; i++) {
				printf("trame %d envoye\n : data -> %s\n",i,tramesMesg[i]->data);
				int retValue = sendTrame(tramesMesg[i],socket_descriptor);
				sleep(2);
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
