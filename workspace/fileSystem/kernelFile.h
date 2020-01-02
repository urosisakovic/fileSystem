#pragma once
#include "utils.h"
#include "part.h"
#include "kernelFS.h"
#include "fileptr.h"
#include "clusterAllocation.h"
#include "FileSystemUtils.h"

class KernelFS;
class FilePointer;

class KernelFile {
public:
	KernelFile(ClusterNo, ClusterNo, bool);
	~KernelFile(); //zatvaranje fajla   
	char write(BytesCnt, char*);
	BytesCnt read(BytesCnt, char*);
	char seek(BytesCnt);
	BytesCnt filePos();
	char eof();
	BytesCnt getFileSize();
	char truncate();
	void disableWriting() { canWrite = false; }
private:
	FilePointer *filePtr;
	char *clusterBuffer;
	BytesCnt size;
	bool canWrite;
	char* fname;

	friend class KernelFS;
};

