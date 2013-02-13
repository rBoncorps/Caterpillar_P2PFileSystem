#include "trame.h"

typedef struct Trame Trame;

Trame* creationTrame(TypeTrame type,int taille, char* dataT) {
	Trame* trame;
	trame = malloc(sizeof(Trame));
	trame->typeTrame = type;
	trame->taille = taille;
	if (taille > 256) {
		printf("data trop grand pour trame");
		return NULL;
	}
	else {
		strcat(trame->data,dataT);
		trame->data[taille] = '\0';
		int i = 0;
		for (int i = taille + 1 ; i < 255; i++) {
			trame->data[i] = '0';
		}
		trame->data[256] = '\0';
	}
	 return trame;
}
	
