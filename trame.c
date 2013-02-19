#include "trame.h"

typedef struct Trame Trame;

Trame* creationTrame(char* fromName, TypeTrame type,int taille,int numTrame,int nbTrame, char* dataT) {
	Trame* trame;
	trame = malloc(sizeof(Trame));
	trame->typeTrame = type;
	trame->numTrame = 1;
	trame->nbTrames = 1;
	trame->taille = taille;
	
	if (taille > TAILLE_MAX_DATA) {
		printf("data trop grand pour trame");
		return NULL;
	}
	else {
		strcat(trame->data,dataT);
		trame->data[taille] = '\0';
		int i = 0;
		for (int i = taille + 1 ; i < (TAILLE_MAX_DATA - 1); i++) {
			trame->data[i] = '0';
		}
		trame->data[TAILLE_MAX_DATA] = '\0';
	}
	if(strlen(fromName) > TAILLE_MAX_USERNAME) {
		printf("taille du username trop grande");
		return NULL;
	}
	else {
		//strcat(trame->nameSrc,fromName);
		strcpy(trame->nameSrc,fromName);
		trame->nameSrc[strlen(trame->nameSrc)] = '\0';
		for(int i = strlen(trame->nameSrc) + 1 ; i < TAILLE_MAX_USERNAME -1; i++) {
			trame->nameSrc[i] = '0';
		}
		trame->nameSrc[TAILLE_MAX_USERNAME] = '\0';
	}
	trame->numTrame = numTrame;
	trame->nbTrames = nbTrame;
	return trame;
}

Trame** decoupageTrame(char* fromName,TypeTrame type,int taille, char* dataT, int* nbTrames) {
	int divTaille = taille/(TAILLE_MAX_DATA-1);
	int modTaille = taille%(TAILLE_MAX_DATA-1);
	int numTrame = 1;
	int nbTrame = 1;
	
	if(modTaille == 0) {
		nbTrame = divTaille;
	}
	else {
		nbTrame = divTaille + 1;
	}
	*nbTrames = nbTrame;
	
	Trame** tabTrame;
	tabTrame = malloc(nbTrame*sizeof(Trame*));
	int i = 0;
	while ( numTrame <= nbTrame) {
		Trame* trame;
		trame = malloc(sizeof(Trame));
		strcpy(trame->nameSrc,fromName);
		trame->nameSrc[strlen(trame->nameSrc)] = '\0';
		for(int i = strlen(trame->nameSrc) + 1 ; i < TAILLE_MAX_USERNAME -1; i++) {
			trame->nameSrc[i] = '0';
		}
		trame->nameSrc[TAILLE_MAX_USERNAME] = '\0';
		trame->typeTrame = type;
		trame->numTrame = numTrame;
		trame->nbTrames = nbTrame;
		if (numTrame == nbTrame) {
			for (int j = 0; j < modTaille ; j++) {
				trame->data[j] = dataT[i];
				i++;
			}
			trame->data[modTaille] = '\0';
			trame->taille = modTaille;
			for (int j = modTaille + 1; j < (TAILLE_MAX_DATA - 1) ; j++) {
				trame->data[j] = '0';
			}
		}
		else {
			for (int j = 0 ; j < (TAILLE_MAX_DATA - 1) ;j++) {
				trame->data[j] = dataT[i];
				i++;
			}
			trame->taille = TAILLE_MAX_DATA-1;
		}
		trame->data[TAILLE_MAX_DATA] = '\0';
		tabTrame[numTrame-1] = trame;
		numTrame++;
	}
	return tabTrame;
}
	
	
char* extractMessage(Trame** tabTrames,int nbTrames) {
	char* mesg = malloc(nbTrames*TAILLE_MAX_DATA);
	int i = 0;
	int k = 0;
	printf("nbTrames : %d\n",nbTrames);
	
	for (i ; i < nbTrames ; i++) {
		printf("passe la\n");
		Trame* trame = tabTrames[i];
		int j = 0;
		int taille = trame->taille;
		printf("passe : %d\n",taille);
		printf("data: %s\n",trame->data);
		for (j ; j < taille ; j++) {
			mesg[k] = trame->data[j];
			k++;
		}
		
	}
	printf("taille : %ld\n",strlen(mesg));
	mesg[strlen(mesg)] = '\0';
	return mesg;
}
	
	
	
	
	
	
	
