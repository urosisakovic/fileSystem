#pragma once
#include "utils.h"
#include "part.h"

class KernelFile{
public:
	KernelFile(Partition*, ClusterNo, ClusterNo, ClusterNo);
	~KernelFile(); //zatvaranje fajla   
	char write(BytesCnt, char*);
	BytesCnt read(BytesCnt, char*);
	char seek(BytesCnt);
	BytesCnt filePos();
	char eof();
	BytesCnt getFileSize();
	char truncate();
private:
	ClusterNo lvl1IndexCluster;
	ClusterNo lvl2IndexCluster;
	ClusterNo dataCluster;
	unsigned position;
	char* privateBuffer;
	Partition *part;
};

