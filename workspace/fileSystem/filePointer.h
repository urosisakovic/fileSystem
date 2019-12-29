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

	ClusterNo lvl2IndexCluster;
	ClusterNo lvl2IndexEntry;

	ClusterNo dataCluster;
	unsigned pos;

public:
	FilePointer() = default;
	FilePointer(ClusterNo rootDirCluster, ClusterNo rootDirEntry);

	char GoToNextCluster();
	void ensureDataCluster();

	friend class KernelFile;
};

