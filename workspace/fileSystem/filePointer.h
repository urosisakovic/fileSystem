#pragma once
#include "utils.h"
#include "kernelFile.h"
#include "kernelFS.h"

class FilePointer {
private:
	ClusterNo rootDirCluster;
	ClusterNo rootDirEntry;

	ClusterNo lvl1IndexCluster;
	ClusterNo lvl1IndexEntry;
	ClusterNo lvl1IndexSize;

	ClusterNo lvl2IndexCluster;
	ClusterNo lvl2IndexEntry;
	ClusterNo lvl2IndexSize;

	ClusterNo dataCluster;
	unsigned pos;

	Partition* partition;

public:
	FilePointer() = default;
	FilePointer(Partition *p, ClusterNo rootDirCluster, ClusterNo rootDirEntry);

	char GoToNextCluster();
	void ensureDataCluster();

	friend class KernelFile;
};

