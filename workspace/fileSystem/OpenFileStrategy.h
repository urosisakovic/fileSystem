#pragma once
#include "utils.h"
#include "kernelFile.h"
#include "clusterAllocation.h"

class KernelFile;

class OpenFileStrategy {
public:
	virtual KernelFile* open();

	static ClusterNo allocateAndSetDataCluster(char*, char*);
	static ClusterNo allocateAndSetLvl2Cluster(ClusterNo);
	static char addEntryToDataDir(ClusterNo, char*, char*, ClusterNo&);

	OpenFileStrategy(char*, ClusterNo);
protected:
	char* fname;
	ClusterNo rootDirLvl1Index;
};

