#include <stdio.h>
#include "map.h"

typedef struct Matrice Matrice;
typedef struct Pair Pair;

struct Pair {
	char* name;
	char* ip;
};

struct Matrice {
	unsigned int size;
	Pair* pairs;
};

Matrice* newMatrice() {
	Matrice* mat;
	mat = malloc(sizeof(Matrice));
	if (mat == NULL) {
		return NULL;
	}
	
	mat->size = 0;
	mat->pairs = NULL;
	
	return mat;
}

void deleteMatrice(Matrice* map) {
	Pair* pair;
	
	if(map == NULL) {
		return;
	}
	int i = 0;	
	pair = map->pairs;
	while (i < map->size) {
		free(pair->name);
		free(pair->ip);
		pair++;
		i++;
	}
	free(map->pairs);
	free(map);
}

char* getIP(Matrice* map, char* name) {
	Pair* pair = map->pairs;
	int i  = 0;
	
	if (map == NULL) {
		return NULL;
	}
	if (name == NULL) {
		return NULL;
	}
	while (i < map->size) {
		if( strcmp(name,pair->name) == 0){
			return pair->ip;
		}
		pair++;
		i++;
	}
	return NULL;
}

Pair* getPair(Matrice* map, char* name) {
	Pair* pair = map->pairs;
	int i  = 0;
	if (map == NULL) {
		return NULL;
	}
	if (map->size == 0) {
		return NULL;
	}
	while (i < map->size) {
		if( strcmp(name,pair->name) == 0){
			return pair;
		}
		pair++;
		i++;
	}
	return NULL;
}
	

int ajouterClient(Matrice* map, char* name, char* ip) {
	unsigned int name_len, ip_len;
	Pair *tmp_pairs, *pair;
	char *tmp_ip;
	char *new_name, *new_ip;
	//printf("coucou\n");
	if (map == NULL) {
		return 0;
	}
	if(name == NULL || ip == NULL) {
		return 0;
	}
	//printf("pouetpouet\n");
	name_len = strlen(name);
	//printf("bim\n");
	ip_len = strlen(ip);
	//printf("paf\n");
	//printf("strlen(name) = %u\nstrlen(ip) = %u\n",name_len,ip_len);
	
	new_name = malloc((name_len + 1) * sizeof(char));
	if (new_name == NULL) {
		return 0;
	}
	new_ip = malloc((ip_len + 1) * sizeof(char));
	if (new_ip == NULL) {
		free(new_name);
		return 0;
	}
	
	if (map->size == 0) {
		map->pairs = malloc(sizeof(Pair));
		if (map->pairs == NULL) {
			free(new_name);
			free(new_ip);
			return 0;
		}
		printf("%x\n",map->pairs);
		map->size = 1;
	}
	else {
		if ((pair = getPair(map,name)) != NULL) {
			if (strcmp(ip,pair->ip) == 0 ) {
				return 1;
			}
			else {
				if(strlen(pair->ip) < strlen(ip)) {
					//mise a jour de l'ip du client
					tmp_ip = realloc(pair->ip,(ip_len + 1) * sizeof(char));
					if (tmp_ip == NULL) {
						free(new_name);
						free(new_ip);
						return 0;
					}
					pair->ip = tmp_ip;
				}
				strcpy(pair->ip,ip);
				return 1;
			}
		}
		else {
			printf("la\n");
			printf("%d\n",map->size);
			printf("%x\n",map->pairs);
			printf("sizeof char* : %d\n",sizeof(char*));
			printf("sizeof pair : %d\n",sizeof(Pair));
			printf("%d\n",((map->size) + 1) * sizeof(Pair));
			printf("length of ip / name : %d / %d \n",strlen(name),strlen(ip));
			tmp_pairs = (Pair*)realloc((map->pairs), ((map->size) + 1) * sizeof(Pair));
			printf("pouet\n");
			if (tmp_pairs == NULL) {
				printf("bim\n");
				free(new_name);
				free(new_ip);
				printf("boum\n");
				return 0;
			}
			printf("icici\n");
			map->pairs = tmp_pairs;
			map->size++;
		}
	}
	pair = &(map->pairs[map->size - 1]);
	pair->name = new_name;
	pair->ip = new_ip;
	
	strcpy(pair->name,name);
	strcpy(pair->ip,ip);
	return 1;
}


void afficherMap(Matrice* map) {
	Pair* pair;
	
	int i = 0;
	pair = map->pairs;
	while (i < map->size) {
		printf("pair %d : name=%s , ip=%s\n",i,pair->name,pair->ip);
		pair++;
		i++;
	}
}
/*
int main(int argc, char* argv[]) {

Matrice* map = newMatrice();
printf("nouvelle matrice créée\n");

ajouterClient(map,"bob","127.0.0.1");
ajouterClient(map,"lolilol","127.0.0.1");

afficherMap(map);

ajouterClient(map,"bob","157.5.20.0");

afficherMap(map);

deleteMatrice(map);
printf("matrice delete\n");

printf("essai : %d", map->size);
return 0;


}
*/
