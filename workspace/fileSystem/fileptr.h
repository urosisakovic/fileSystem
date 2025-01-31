#pragma once
#include <iostream>
#include "utils.h"
#include "kernelFile.h"
#include "kernelFS.h"
#include "clusterAllocation.h"

class FilePointer {
public:
	FilePointer() = default;
	FilePointer(ClusterNo rootDirCluster, ClusterNo rootDirEntry);

	char GoToNextCluster();
	void ensureDataCluster();

	BytesCnt byteOffset();

	ClusterNo getRootDirCluster() { return this->rootDirCluster; }
	ClusterNo getRootDirEntry() { return this->rootDirEntry; }

private:
	ClusterNo rootDirCluster;
	ClusterNo rootDirEntry;

	ClusterNo lvl1IndexCluster;
	ClusterNo lvl1IndexEntry;

	ClusterNo lvl2IndexCluster;
	ClusterNo lvl2IndexEntry;

	ClusterNo dataCluster;
	unsigned pos;

	friend class KernelFile;
};

