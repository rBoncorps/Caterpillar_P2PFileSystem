#ifndef TRAME_H
#define TRAME_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define TAILLE_MAX_USERNAME 100
#define TAILLE_MAX_DATA 256
#define TAILLE_MAX_TRAME 4*sizeof(int) + TAILLE_MAX_DATA + TAILLE_MAX_USERNAME


typedef enum {CON_SERV,ACK_CON,DEM_AMI,DEM_CON_AMI,CMD, DEM_FIC,ENV_FIC,ACK,FIN_CON_AMI,FIN_CON_SERV, ERROR, CHECK_CON} TypeTrame; 
typedef struct Trame {
	TypeTrame typeTrame;
	char nameSrc[TAILLE_MAX_USERNAME];
	//char ipDest[16];
	int numTrame;
	int nbTrames;
	int taille;
	char data[TAILLE_MAX_DATA];
} Trame;

Trame* creationTrame(char* fromName, TypeTrame type, int taille, int numTrame, int nbTrames, char* dataT);
Trame** decoupageTrame(TypeTrame type,int taille, char* dataT, int* nbTrames); 


 
 
 /*
 	TRAME
 		type : connection au serveur, ok connection, demande d'ami, demande de connection avec un utilisateur, commande, demande de fichier,envoi de fichier, accusé de reception, fin de connection avec utilisateur, fin de connection avec le serveur
 		numero de trame : 
 		nombre de trame :
 		
 		
 		
	
 */
 /*
 	TRAME
    Port source : Numéro du port source
    Port destination : Numéro du port destination
    Numéro de séquence : Numéro de séquence du premier octet de ce segment
    Numéro d'acquittement : Numéro de séquence du prochain octet attendu
    Taille de l'en-tête : Longueur de l'en-tête en mots de 32 bits (les options font partie de l'en-tête)
    Drapeaux
        Réservé : Réservé pour un usage futur
        ECN : signale la présence de congestion, voir RFC 3168
        URG : Signale la présence de données URGentes
        ACK : Signale que le paquet est un accusé de réception (ACKnowledgement)
        PSH : Données à envoyer tout de suite (PuSH)
        RST : Rupture anormale de la connexion (ReSeT)
        SYN : Demande de synchronisation (SYN) ou établissement de connexion
        FIN : Demande la FIN de la connexion
    Fenêtre : Taille de fenêtre demandée, c'est-à-dire le nombre d'octets que le récepteur souhaite recevoir sans accusé de réception
    Checksum : Somme de contrôle calculée sur l'ensemble de l'en-tête TCP et des données, mais aussi sur un pseudo en-tête (extrait de l'en-tête IP)
    Pointeur de données urgentes : Position relative des dernières données urgentes
    Options : Facultatives
    Remplissage : Zéros ajoutés pour aligner les champs suivants du paquet sur 32 bits, si nécessaire
    Données : Séquences d'octets transmis par l'application (par exemple : +OK POP3 server ready...)

 	
 
 */
 
 #endif
