#include "header.h";

int fileExists(char * filename)
{
	return (access(filename, 0) == 0);
}
/*
1 - ���� ������ �� ������ � ������ ����
����� ���� ���
*/
int accessRights(char *fileName)
{
	//if (fileExists(fileName))
	//{
		return (access(fileName, 6) == 0);
	//}
//	else {
		//printf("���� %s �� ����������", fileName);
	//}

}
char *uniqName() {
	unsigned char randChislo;
	char *path = NULL;
	char str[LENGTH_TMP_NAME];
	UINT64 size;
	int k = 0;
	if ((path = (char*)malloc(k = (strlen(str) + strlen(EXTENTION)))) == NULL)
		ALLOC_MEMORY_ERR
		while (1)
		{
			str[0] = 'T'; str[1] = 'M'; str[2] = 'P';
			for (int i = 3; i < LENGTH_TMP_NAME - 1; i++) {
				randChislo = 97 + rand() % 25;
				str[i] = randChislo;
			}
			str[LENGTH_TMP_NAME - 1] = '\0';
			strcpy(path, str); strcat(path, EXTENTION);
			if (!fileExists(path))
			{
				return path;
			}
		}
}