#include <stdlib.h>
#include <stdio.h>

int main(void) {
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
		FILE* file1 = NULL;
		file1 = fopen("FichierB/Spec.pdf","w");
		int longWr = fwrite(buffer,sizeof(char),size,file1);
		printf("longWr : %d\n",longWr);
		fclose(file1);
	}
}
