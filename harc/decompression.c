#include "header.h"
char read_bit(FILE* in)
{
	static unsigned char buf = 0, counter = 0;
	if (!counter)
	{
		fread(&buf, sizeof(char), 1, in);
	}
	counter++;
	int bit = buf >> 7;
	buf <<= 1;
	if (counter == 8)
		counter = 0;
	return bit;
}
unsigned char read_char(FILE* in)
{
	unsigned char c = 0;
	for (int i = 0; i < sizeof(char) * 8; i++)
	{
		c <<= 1;
		c |= read_bit(in);
	}
	return c;
}
/*�������������� ������ �� �����*/
Tree *createNode(FILE *inputFile)
{
	unsigned char c;
	Tree *tmp = (Tree*)malloc(sizeof(Tree));
	char bit = read_bit(inputFile);
	if (bit == 0)
	{
		tmp->symbol = -1;
		tmp->left = createNode(inputFile);
		tmp->right = createNode(inputFile);
		return tmp;
	}
	if (bit == 1)
	{
		c = read_char(inputFile);
		tmp->symbol = c;
		tmp->left = tmp->right = NULL;
		return tmp;
	}
}
void decode(FILE *inputFile, FILE *outputFile, unsigned short *crc)
{
	UINT64 dataSize = 0;
	fread(&dataSize, sizeof(UINT64), 1, inputFile);
	Tree *root = createNode(inputFile);
	Tree *tmp = root;
	unsigned char c;
	char tmpBuf[] = {"00000000"};
	/*������������� � ������� ������� �� ������*/
	for (UINT64 i = 0; i < dataSize; i++)
	{
		c = read_bit(inputFile);
		if (c == 0)
		{
			tmp = tmp->left;
		}
		else
		{
			tmp = tmp->right;
		}
		if (tmp->symbol != -1)
		{
			CharToString(tmpBuf, tmp->symbol);
			crc16(tmpBuf, 1, crc);
			if (fwrite(&(tmp->symbol), sizeof(char), 1, outputFile) != 1)
				WRITING_DATA_ERR
			tmp = root;
		}
	}
	deleteNode(root); root = NULL;
	fcloseall;
}