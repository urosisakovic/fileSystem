#pragma once
#include "utils.h"
#include "part.h"
#include "kernelFS.h"
#include "filePointer.h"

class KernelFile {
public:
	KernelFile(KernelFS*, Partition*, ClusterNo, ClusterNo);
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
	char* clusterBuffer;
	Partition* partition;
	KernelFS* fs;

	unsigned size;
};

