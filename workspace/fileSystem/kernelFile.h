#pragma once
#include "utils.h"
#include "part.h"
#include "kernelFS.h"
#include "fileptr.h"

class KernelFS;
class FilePointer;

class KernelFile {
public:
	KernelFile(ClusterNo, ClusterNo);
	~KernelFile(); //zatvaranje fajla   
	char write(BytesCnt, char*);
	BytesCnt read(BytesCnt, char*);
	char seek(BytesCnt);
	BytesCnt filePos();
	char eof();
	BytesCnt getFileSize();
	char truncate();

private:
	FilePointer *filePtr;
	char *clusterBuffer;
	unsigned size = 0;
};

