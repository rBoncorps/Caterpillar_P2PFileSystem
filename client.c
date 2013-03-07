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
char* name = "bob\0";char* ip = "127.0.0.1\0";
//char* name = "alice\0";char* ip = "127.0.0.1\0";

void* handleTrame(void* sock) {
	printf("in handle trame\n");
	int* tmp = (int*)sock;
	int socketDescriptor = *tmp;
	int longueur;
	char buffer[TAILLE_MAX_TRAME];
	Trame* received = receiveTrame(socketDescriptor);
	printf("%d\n",received->typeTrame);
	if(received->typeTrame == CHECK_CON) {
		printf("CHECK_CON from %s\n",received->nameSrc);
		printf("data : %s\n",received->data);
		Trame* ackCheckConTrame = creationTrame(name,ACK_CON,0,1,1,"");
		printf("writed namesrc : %s\n",name);
		sendTrame(ackCheckConTrame,socketDescriptor);
	}
	else if(received->typeTrame == CMD_CON) {
		printf("Received a CMD_CON trame type\n");
		FILE* test;
		char testB[200];
		char homePath[200];
		if ( (test = popen("echo $HOME", "r")) == NULL ) {  // ouverture
			exit(1); 
		}

		while ( fgets(testB, 200, test) != NULL ) { 
			//fputs(testB, stdout);
			strcpy(homePath,testB);
		}  
		pclose(test); // fermeture	
		int i = 0;
		while(homePath[i] != '\n') {
			i++;
		}
		homePath[i] = '\0';
		strcat(homePath,"/");
		printf("homePath : %s|\n",homePath);	
		Trame* homePathTrame = creationTrame(name,CMD_HOME,strlen(homePath),1,1,homePath);
		int sent = sendTrame(homePathTrame,socketDescriptor);
		if ( (test = popen("cd", "r")) == NULL ) {  // ouverture
			exit(1); 
		}
		
		int exitCmdMode = 0;
		char* currentPath = malloc(200*sizeof(char));
		strcpy(currentPath,homePath);
		while(!exitCmdMode) {
			received = receiveTrame(socketDescriptor);
	
			if (received->typeTrame == CMD) {
				printf("receive a command\n");
				char* cmdData = malloc(received->taille * sizeof(char));
				strcpy(cmdData,received->data);
				int i = 0;
				char* cmdType = malloc(100 * sizeof(char));
				char* cmdParam = malloc(100*sizeof(char));
				//extract command type
				while(cmdData[i] != ' ') {
					cmdType[i] = cmdData[i];
					i++;
				}
				cmdType[i] = '\0';
				// extract command param
				i++;
				int j = 0;
				while(cmdData[i] != '\0') {
					cmdParam[j] = cmdData[i];
					i++;
					j++;
				}
				cmdParam[j] = '\0';
				if(strcmp(cmdType,"cd") == 0) {
					FILE* test;
					char testB[200];
					char cdReturn[200];
					printf("cmdData :%s\n",cmdData);
					printf("homePath:%s\n",homePath);
					printf("cmdParam:%s\n",cmdParam);
					char* realPath = malloc((strlen(homePath)+strlen(cmdParam))*sizeof(char));
					strcpy(realPath,homePath);
					strcat(realPath,cmdParam);
					printf("realPath : %s\n",realPath);
					char command[200];
					strcpy(command,cmdType);
					strcat(command," ");
					strcat(command,realPath);
					printf("command : %s\n",command);
					if ( (test = popen(command, "r")) == NULL ) {  // ouverture
						exit(1); 
					}

					while ( fgets(testB, 200, test) != NULL ) { 
						//fputs(testB, stdout);
						strcpy(cdReturn,testB);
					}  
					pclose(test); // fermeture	
					printf("return of cd : %s\n",cdReturn);
					//cd worked
					strcpy(currentPath,realPath);
					printf("current Path : %s\n",currentPath);
					Trame* cdTrame = creationTrame(name,CD_RET,1,1,1,"1");
					int sent = sendTrame(cdTrame,socketDescriptor);
					
				}
				else if (strcmp(cmdType,"ls") == 0) {
					FILE* test;
					char testB[200];
					char* lsReturn = malloc(1*sizeof(char));
					char command[200];
					strcat(command,cmdType);
					strcat(command," ");
					strcat(command,currentPath);
					if ( (test = popen(command, "r")) == NULL ) {  // ouverture
						exit(1); 
					}

					while ( fgets(testB, 200, test) != NULL ) { 
						//fputs(testB, stdout);
						lsReturn = (char*)realloc(lsReturn,(strlen(lsReturn)+200)*sizeof(char));
						strcat(lsReturn,testB);
					}  
					pclose(test); // fermeture	
					printf("return of the ls : %s\n",lsReturn);
					printf("sending ls result\n");
					int nbTrames;
					Trame** tramesMesg = decoupageTrame(name,LS_RET,strlen(lsReturn), lsReturn,&nbTrames); 
					int i = 0;
					printf("nbTrames : %d\n",nbTrames);
					for(i ; i < nbTrames ; i++) {
						int retValue = sendTrame(tramesMesg[i],socketDescriptor);
						sleep(2);
					}
				}

		
		
				/*FILE* test;
				char testB[200];
				if ( (test = popen("ls -a", "r")) == NULL ) {  // ouverture
					exit(1); 
				}
	
				while ( fgets(testB, 200, test) != NULL ) { 
					//fputs(testB, stdout);
					printf("%s",testB);
				}  

				pclose(test); // fermeture*/
			}
			else if(received->typeTrame == DEM_FIC) {
				printf("want file %s\n",received->data);
				//if(received->typeTrame == DEM_FIC) {
				printf("received a get_file trame\n");
				// try to open the wanted file
				FILE* file = NULL;
				//printf("Try to open %s file\n",received->data);
				printf("Try to open %s file\n",received->data);
				char* filePath = malloc((strlen(currentPath)+received->taille)*sizeof(char));
				strcpy(filePath,currentPath);
				strcat(filePath,received->data);
				//file = fopen(received->data, "r");
				//file = fopen("FichierA/Spec_Communication.pdf","r");
				file = fopen(filePath,"r");
				if(file == NULL) {
					char errorMessage[2000];
					strcpy(errorMessage,"Erreur : impossible d'ouvrir le fichier \0");
					strcat(errorMessage, received->data);
					//strcat(errorMessage, cmdParam);
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
			else if(received->typeTrame == ENV_FIC) {
				printf("got env fic\n");
				// put cmd
				int tailleFichier = 0;
				if(received->nbTrames > 1) {
					FILE* file = NULL;
					char* filePath = "FichierB/zobizobizbozi.pdf"; // for debugging
					file = fopen(filePath, "w");
					if(file == NULL) {
						printf("null file ...\n");
						return;
					}
					printf("pouetpouet\n");
					int nbWaitedTrames = received->nbTrames;
					int nbReceivedTrames = 0;
					Trame* currentTrame = received;
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
						currentTrame = receiveTrame(socketDescriptor);
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
				}
			}
			else if(received->typeTrame == CMD_END) {
				exitCmdMode = 1;
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
	printf("Bienvenue dans Caterpillar p2p file system !\n");
	printf("hello %s\n",name);
	int socket_descriptor,longueur;
	char * host = "127.0.0.1";
	
	//creation socket to dialog with server
	socket_descriptor = connectTo("big-daddy",host);
	
	//map containing friend of the client
	mapAmi = newMatrice();
	
	char* mesg = malloc(strlen(name) + 1 + strlen(ip) + 1);
	strcat(mesg,name);
	strcat(mesg,":");
	strcat(mesg,ip);
	mesg[strlen(mesg)] = '\0';
	
	//sending trame to connect to the server
	trame = creationTrame(name,CON_SERV,strlen(mesg),1,1,mesg);
	int sent = sendTrame(trame, socket_descriptor);
	
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
			//procedure to get the ip of the person to add it to your friend list
			//so you can echange with him
			printf("\twant to add %s as a friend\n",parameter);
			trame = creationTrame(name,DEM_AMI,strlen(parameter),1,1,parameter);
			int sent = sendTrame(trame, socket_descriptor);
			printf("apres envoi\n");
			Trame* response = receiveTrame(socket_descriptor);
			printf("apres reception\n");
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
		/*if(strcmp(actionName,"put_file") == 0) {
			char* contactIP = getIP(mapAmi,parameter);
			if(contactIP == NULL) {
				printf("Le contact %s n'est pas dans votre liste d'amis, veuillez-utiliser <add_friend %s>\n",parameter,parameter);
				continue;
			}
			int contactSocket = connectTo(*/
		
		if(strcmp(actionName,"cmd") == 0) {
			printf("You entered commande mode. allowed command : cd|ls|get_file<file>\n");
			char* contactName = parameter;
			char* contactIP = getIP(mapAmi,contactName);
			char* currentPath = malloc(140 * sizeof(char));
			if(contactIP == NULL) {
				printf("Le contact %s n'est pas dans votre liste d'amis, veuillez-utiliser <add_friend %s>\n",parameter,parameter);
				continue;
			}
			int contactSocket = connectTo(contactName, contactIP);
			//home path on distant client
			trame = creationTrame(name,CMD_CON,0,1,1,"");
			int sent = sendTrame(trame, contactSocket);
			Trame* homePathTrame = receiveTrame(contactSocket);
			if(homePathTrame->typeTrame == ERROR) {
				printf("%s\n",homePathTrame->data);
				continue;
			}
			strncpy(currentPath,homePathTrame->data,homePathTrame->taille);
			printf("current path on distant client : %s\n",currentPath);
			int exitCmdMode = 0;
			while(exitCmdMode == 0) {
				printf("    > ");
				char* command = malloc(100*sizeof(char));
				fgets(command,150,stdin);
				if ((strlen(command)>0) && (command[strlen (command) - 1] == '\n')) {
					command[strlen (command) - 1] = '\0';
				}
				char* commandName = malloc(10*sizeof(char));
				char* cmdParameter = malloc(140*sizeof(char));
				sscanf(command,"%s %s",commandName,cmdParameter);
				if (strcmp(commandName,"exit") == 0) {
					exitCmdMode = 1;	
					Trame* cmdTrame = creationTrame(name,CMD_END,0,1,1,"");
					int sent = sendTrame(cmdTrame,contactSocket);
				}
				else {
					
					if (strcmp(commandName,"cd") == 0) {
						Trame* cmdTrame = creationTrame(name,CMD,strlen(command),1,1,command);
						int sent = sendTrame(cmdTrame,contactSocket);
						Trame* contactResponse = receiveTrame(contactSocket);
						if(contactResponse->typeTrame == ERROR) {
							printf("%s\n",contactResponse->data);
							continue;
						}
					
					}
					if (strcmp(commandName,"ls") == 0) {
						printf("test\n");
						char* msg = malloc(strlen(commandName) + 2);
						strcat(msg,commandName);
						strcat(msg, " ");
						msg[strlen(msg)] = '\0';						
						Trame* cmdTrame = creationTrame(name,CMD,strlen(msg),1,1,msg);
						int sent = sendTrame(cmdTrame,contactSocket);
						printf("sent\n");
						Trame* contactResponse = receiveTrame(contactSocket);
						if(contactResponse->typeTrame == ERROR) {
							printf("%s\n",contactResponse->data);
							continue;
						}
						char* lsPrint = malloc(contactResponse->taille*sizeof(char));
						strncpy(lsPrint,contactResponse->data,contactResponse->taille);
						if(contactResponse->nbTrames > 1) {
							int nbWaitedTrames = contactResponse->nbTrames;
							int nbReceivedTrames = 1;
							Trame* contactTrame;
							while (nbReceivedTrames < nbWaitedTrames) {
								contactTrame = receiveTrame(contactSocket);
								nbReceivedTrames++;
								lsPrint = (char*)realloc(lsPrint,(strlen(lsPrint)+contactTrame->taille)*sizeof(char));
								strncat(lsPrint,contactTrame->data,contactTrame->taille);
							}
							
						}
						printf("ls received : %s\n",lsPrint);
					}
					if(strcmp(commandName,"get_file") == 0) {
						/*char* contactIP = getIP(mapAmi,parameter);
						if(contactIP == NULL) {
							printf("Le contact %s n'est pas dans votre liste d'amis, veuillez-utiliser <add_friend %s>\n",parameter,parameter);
							continue;
						}*/
						printf("want to get file %s from the friend %s(%s)\n",cmdParameter, contactName,contactIP);
						//int contactSocket = connectTo(parameter, contactIP);
						//Trame* demFicTrame = creationTrame(name,DEM_FIC,strlen(cmdParameter),1,1,cmdParameter);
						Trame* demFicTrame = creationTrame(name,DEM_FIC,strlen(cmdParameter),1,1,cmdParameter);
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
						}
					}
					if(strcmp(commandName,"put_file") == 0) {
						FILE* file = NULL;
						//printf("Try to open %s file\n",received->data);
						//printf("Try to open %s file\n",received->data);
						//file = fopen(received->data, "r");
						file = fopen("FichierA/Spec_Communication.pdf","r");
						if(file == NULL) {
							char errorMessage[2000];
							strcpy(errorMessage,"Erreur : impossible d'ouvrir le fichier \0");
							strcat(errorMessage, cmdParameter);
							//strcat(errorMessage, cmdParam);
							strcat(errorMessage, ".\0");
							printf("%s\n",errorMessage);
							//Trame* errorTrame = creationTrame(name,ERROR,strlen(errorMessage),1,1,errorMessage);
							//int sent = sendTrame(errorTrame,socketDescriptor);
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
								int retValue = sendTrame(tramesMesg[i],contactSocket);
								sleep(2);
							}
						}
					}	
				}
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
