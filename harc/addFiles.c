#include "header.h"

int addFiles(char *archiveName, char **fileNames,int *amountOfFiles, Info **ptrOnStruct)
{
	unsigned short crc = CRC;
	const unsigned int ussd = SIGNATURE;
	struct _stat64 info;
	UINT64 posForWRCRC; //позиция для записи crc

	UINT64 posForWRSize;
	UINT64 realSize;
	char percentCompression;
	unsigned int TMPussd = 0;//signature
	char *data = NULL;
	FILE *fin = NULL, *tmp=NULL, *fout = NULL;
	int makeTmpArchieve;//создавать временный архив или нет(флаг)
	int u = 0; 
	switch (makeTmpArchieve = isEmptyFile(archiveName))
	{
		case 0:
		case 1:
			if ((fout = fopen(archiveName, "a+b")) == NULL)
				OPEN_ERR
			if ((fwrite(&(ussd),sizeof(unsigned int), 1 ,fout))!=1)
				WRITING_DATA_ERR
			fflush(fout);
			if (fclose(fout) == -1)
				CLOSING_FILE_ERR
			makeTmpArchieve = 0;
			break;
		case 2:
			if (checkUssd(archiveName, SIGNATURE) != 0) return 1;
			break;
	}
	//в цикле просто по порядку добавляем в пустой архив
	if ((makeTmpArchieve == 0) || (makeTmpArchieve == 1))
	{
		
		if ((fout = fopen(archiveName, "rb+")) == NULL)
			CREATE_FILE_ERR
		for (u; u < amountOfFiles; u++)
		{
				_stat64(fileNames[u], &info);
				if (accessRights(fileNames[u], READING) != 1) {
					printf("[WARNING:]Файл %s не имеет прав на чтение\n", archiveName);
					return 0;
				}
				if ((fin = fopen(fileNames[u], "rb")) == NULL)
					OPEN_ERR
			//заполнение полей структуры

				(*ptrOnStruct)->lengthName = strlen(shortNameOnly(fileNames[u])); //длина имени файла

				strcpy((*ptrOnStruct)->name, shortNameOnly(fileNames[u]));
				(*ptrOnStruct)->size = info.st_size;
				if (compressOrNot((*ptrOnStruct)->size))
					(*ptrOnStruct)->flags = COMPRESSED;
				else (*ptrOnStruct)->flags = NOTCOMPRESSED;
				(*ptrOnStruct)->compression = 0;
			//запись данных в архив 
					_fseeki64_nolock(fout, 0, SEEK_END);
					 posForWRCRC = _ftelli64_nolock(fout);
					(*ptrOnStruct)->checkSum = crc;//контрольная сумма

					if ((fwrite(&((*ptrOnStruct)->checkSum), SIZE_CHECKSUM, 1, fout)) != 1)
						WRITING_DATA_ERR
					if ((fwrite(&((*ptrOnStruct)->lengthName), SIZE_LENGTHNAME, 1, fout)) != 1)
						WRITING_DATA_ERR
					if ((fwrite(&((*ptrOnStruct)->name), (*ptrOnStruct)->lengthName, 1, fout)) != 1)
						WRITING_DATA_ERR
					if (fwrite(&((*ptrOnStruct)->flags), SIZE_FLAGS, 1, fout) != 1)
						WRITING_DATA_ERR
					if (fwrite(&((*ptrOnStruct)->compression), SIZE_FLAGS, 1, fout) != 1)
						WRITING_DATA_ERR
					posForWRSize = _ftelli64_nolock(fout);
					if (fwrite(&((*ptrOnStruct)->size), SIZE_SIZE, 1, fout) != 1)
						WRITING_DATA_ERR
			//запись файла
					crc = CRC;
					if (compressOrNot((*ptrOnStruct)->size))
						encode(fin, fout, getSize(fin),&crc);
					else {
						if ((data = (char*)malloc(SizeOfBuf)) == NULL)
							ALLOC_MEMORY_ERR
						writeDataToFile(data, fin, fout, &crc, (*ptrOnStruct)->size);
						free(data);
					}
					fflush(fout);
					realSize = _ftelli64_nolock(fout) - posForWRSize - SIZE_SIZE;
					percentCompression = (char)(compressionRatio((double)(*ptrOnStruct)->size, (double)realSize));
			///сдвиг для записи размера
			// -1 для записи процента сжатия

					_fseeki64_nolock(fout,posForWRSize-1, SEEK_SET);
					if ((fwrite(&percentCompression, SIZE_FLAGS, 1, fout)) != 1)
						WRITING_DATA_ERR
					if ((fwrite(&realSize, SIZE_SIZE, 1, fout)) != 1)
						WRITING_DATA_ERR
			//сдвиг обратно для записи контрольной суммы

					_fseeki64_nolock(fout, posForWRCRC, SEEK_SET);
					(*ptrOnStruct)->checkSum = crc;
					if ((fwrite(&((*ptrOnStruct)->checkSum), SIZE_CHECKSUM, 1, fout)) != 1)
						WRITING_DATA_ERR
					_fseeki64_nolock(fout,0,SEEK_END);
		}
		fcloseall;
	}
	
	else //если нужно создавать временный архив

	{
		if ((fin = fopen(archiveName, "rb+")) == NULL)
			OPEN_ERR
		//создание временного файла

		char *tmpArchiveName =NULL;
		tmpArchiveName = uniqName();
		if ((tmp = fopen(tmpArchiveName, "wb+")) == NULL)
			OPEN_ERR
		//запись сигнатуры во временный архив

		if ((fwrite(&(ussd), sizeof(unsigned int), 1, tmp))!= 1)
			WRITING_DATA_ERR fflush(tmp);
		UINT64 endOFFile = getSize(fin);
		//чтение сигнатуры

		if (fread(&TMPussd, SIZE_SIGNATURE, 1 , fin) != 1)
			READING_DATA_ERR
		while ((_ftelli64_nolock(fin)) != endOFFile)
		{
			// флаг = 1 , если совпадёт хоть один файл
			int flag = 0;
			int y;//переменная для цикла for(поиск повторов)

			if ((fread(&((*ptrOnStruct)->checkSum), SIZE_CHECKSUM, 1 , fin)) != 1)
				READING_DATA_ERR
			if ((fread(&((*ptrOnStruct)->lengthName), SIZE_LENGTHNAME , 1 ,fin))!= 1)
				READING_DATA_ERR
			if ((fread(&((*ptrOnStruct)->name), ((*ptrOnStruct)->lengthName), 1, fin)) != 1)
				READING_DATA_ERR
			if ((fread(&((*ptrOnStruct)->flags), SIZE_FLAGS, 1, fin)) != 1)
				READING_DATA_ERR
			if ((fread(&((*ptrOnStruct)->compression), SIZE_FLAGS, 1, fin)) != 1)
				READING_DATA_ERR
			if ((fread(&((*ptrOnStruct)->size), SIZE_SIZE, 1, fin)) != 1)
				READING_DATA_ERR
				y = 0;
				char *TMP2name = NULL;
				if ((TMP2name = (char*)malloc(((*ptrOnStruct)->lengthName) + 1)) == NULL)
					ALLOC_MEMORY_ERR
				strncpy((TMP2name), (*ptrOnStruct)->name, (*ptrOnStruct)->lengthName);
				TMP2name[(*ptrOnStruct)->lengthName] = '\0';
				for (y; y < amountOfFiles; y++)
				{
					if (!strcmp(TMP2name, shortNameOnly(fileNames[y])))
					{
						flag = 1;
						break;
					}
				}
				free(TMP2name);
				//если есть совпадение, то мы просто пропускаем данный файл и смотрим следующий
				if (flag)
				{
					//нужно сдвинуться на размер файла

					_fseeki64_nolock(fin, (*ptrOnStruct)->size, SEEK_CUR);
					flag = 0;
					free(data);
					continue;
				}
				else
				{
				//записываем во временный архив

				_fseeki64_nolock(tmp, 0, SEEK_END);
				if ((fwrite(&((*ptrOnStruct)->checkSum), SIZE_CHECKSUM, 1, tmp)) != 1)
				WRITING_DATA_ERR
				if ((fwrite(&((*ptrOnStruct)->lengthName), SIZE_LENGTHNAME, 1, tmp)) != 1)
				WRITING_DATA_ERR
				if ((fwrite(&((*ptrOnStruct)->name), (*ptrOnStruct)->lengthName, 1, tmp)) != 1)
				WRITING_DATA_ERR
				if (fwrite(&((*ptrOnStruct)->flags), SIZE_FLAGS, 1, tmp) != 1)
				WRITING_DATA_ERR
				if (fwrite(&((*ptrOnStruct)->compression), SIZE_FLAGS, 1, tmp) != 1)
				WRITING_DATA_ERR
				if (fwrite(&((*ptrOnStruct)->size), SIZE_SIZE, 1, tmp) != 1)
				WRITING_DATA_ERR
						//запись самого файла

				if ((data = (char*)malloc(SizeOfBuf)) == NULL)
					ALLOC_MEMORY_ERR
				writeDataToFile(data, fin, tmp, &crc, (*ptrOnStruct)->size);
				free(data);
				fflush(tmp);
			}
	}
		//закрываем наш архив

		if (fclose(fin) == -1)
			CLOSING_FILE_ERR
		//теперь во временный нужно переписать то, что было в основном

			for (int t = 0; t < amountOfFiles; t++)
			{
				_stat64(fileNames[t], &info);
				if ((fin = fopen(fileNames[t], "rb")) == NULL)
					OPEN_ERR
				//заполнение данных

				(*ptrOnStruct)->checkSum = crc;
				(*ptrOnStruct)->lengthName = strlen(shortNameOnly(fileNames[t]));
				strcpy((*ptrOnStruct)->name, shortNameOnly(fileNames[t]));
				(*ptrOnStruct)->size = info.st_size;
				if (compressOrNot((*ptrOnStruct)->size))
					(*ptrOnStruct)->flags = COMPRESSED;
				else (*ptrOnStruct)->flags = NOTCOMPRESSED;
				(*ptrOnStruct)->compression = 0;
				//запись данных

				_fseeki64_nolock(tmp, 0, SEEK_END);
				posForWRCRC = _ftelli64_nolock(tmp);
				if ((fwrite(&((*ptrOnStruct)->checkSum), SIZE_CHECKSUM, 1, tmp)) != 1)
				WRITING_DATA_ERR
				if ((fwrite(&((*ptrOnStruct)->lengthName), SIZE_LENGTHNAME, 1, tmp)) != 1)
				WRITING_DATA_ERR
				if ((fwrite(&((*ptrOnStruct)->name), (*ptrOnStruct)->lengthName, 1, tmp)) != 1)
				WRITING_DATA_ERR
				if (fwrite(&((*ptrOnStruct)->flags), SIZE_FLAGS, 1, tmp) != 1)
				WRITING_DATA_ERR
				if (fwrite(&((*ptrOnStruct)->compression), SIZE_FLAGS, 1, tmp) != 1)
				WRITING_DATA_ERR
				posForWRSize = _ftelli64_nolock(tmp);
				if (fwrite(&((*ptrOnStruct)->size), SIZE_SIZE, 1, tmp) != 1)
				WRITING_DATA_ERR
				//чтение и запись самого файла

				crc = CRC;
				if (compressOrNot((*ptrOnStruct)->size))
					encode(fin, tmp, getSize(fin), &crc);
				else {
					if ((data = (char*)malloc(SizeOfBuf)) == NULL)
						ALLOC_MEMORY_ERR
						writeDataToFile(data, fin, tmp, &crc, (*ptrOnStruct)->size);
					free(data);
				}
				fflush(tmp);
				realSize = _ftelli64_nolock(tmp) - posForWRSize - SIZE_SIZE;
				percentCompression = (char)(compressionRatio((double)(*ptrOnStruct)->size, (double)realSize));
				_fseeki64_nolock(tmp, posForWRSize - 1, SEEK_SET);
				if ((fwrite(&percentCompression, SIZE_FLAGS, 1, tmp)) != 1)
					WRITING_DATA_ERR
				if ((fwrite(&realSize, SIZE_SIZE, 1, tmp)) != 1)
					WRITING_DATA_ERR
				//сдвиг обратно для записи контрольной суммы
				_fseeki64_nolock(tmp, posForWRCRC, SEEK_SET);
				(*ptrOnStruct)->checkSum = crc;//контрольная сумма 
				if ((fwrite(&((*ptrOnStruct)->checkSum), sizeof(unsigned short), 1, tmp)) != 1)
					WRITING_DATA_ERR
				_fseeki64_nolock(tmp, 0, SEEK_END);
				if (fclose(fin) == -1)
					CLOSING_FILE_ERR
			}
		if (fclose(tmp) == -1)
			CLOSING_FILE_ERR
			//удалить архив переименовать temp

			if (remove(archiveName) == -1)
				perror("[ERROR]:Could not delete %s\n",archiveName);
			if (rename(tmpArchiveName, archiveName) == -1)
				printf("[ERROR]:Не удалось переименовать временный архив\n");
	}
	return 0;
}