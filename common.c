#include "common.h"

int extractNameIP(char* from, char** p_name, char** p_ip) {
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
		name[i] = from[i];
		i++;
	}
	name[i+1] = '\0';
	i++;
	int j = 0;
	while(from[i] != '\0' && j < IP_SIZE+1) {
		ip[j] = from[i];
		i++;
		j++;
	}
	ip[j+1] = '\0';
	*p_name = name;
	*p_ip = ip;
	return 0;
}
