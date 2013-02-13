#ifndef MAP_H
#define MAP_H

#include <stdlib.h>
#include <string.h>

typedef struct Matrice Matrice;
typedef struct Pair Pair;

Matrice * newMatrice();
void deleteMatrice(Matrice* map);
char* getIP(Matrice* map, char* name);
Pair* getPair(Matrice* map, char* name);
int ajouterClient(Matrice* map, char* name, char* ip);
void afficherMap(Matrice* map);

#endif

