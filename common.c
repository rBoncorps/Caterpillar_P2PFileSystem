#include "common.h"

int extractNameIP(char* from, char** p_name, char** p_ip) {
	printf("in extractNameIP\n");
	char* name;
	char* ip;
	name = malloc((MAX_NAME_SIZE+1)*sizeof(char));
	// Max IP size is xxx.xxx.xxx.xxx (15) + \0
	ip = malloc((IP_SIZE+1)*sizeof(char));
	int i = 0;
	while(from[i] != ':') {
		if(i > MAX_NAME_SIZE) {
			return -1;
		}
		printf("readed char %c\n",from[i]);
		name[i] = from[i];
		i++;
	}
	name[i] = '\0';
	i++;
	int j = 0;
	while(from[i] != '\0' && j < IP_SIZE+1) {
		ip[j] = from[i];
		i++;
		j++;
	}
	ip[j+1] = '\0';
	printf("extracted name : %s\n",name);
	*p_name = name;
	*p_ip = ip;
	return 0;
}

int extractAskedFriendName(char* from,int dataSize, char** p_name) {
	char* name;
	name = malloc((dataSize+1)*sizeof(char));
	int i = 0;
	while (i < dataSize+1) {
		name[i] = from[i];
		i++;
	}
	name[dataSize+1] = '\0';
	*p_name = name;
	return 0;
}
