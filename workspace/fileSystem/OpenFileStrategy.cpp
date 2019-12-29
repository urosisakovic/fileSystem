#include "OpenFileStrategy.h"

OpenFileStrategy::OpenFileStrategy(char *fname, ClusterNo rootDirLvl1Index) {
	this->fname = fname;
	this->rootDirLvl1Index = rootDirLvl1Index;
}

ClusterNo OpenFileStrategy::allocateAndSetLvl2Cluster(ClusterNo dataCluster) {
	ClusterNo newRootDirLvl2IndexCluster = ClusterAllocation::allocateCluster();
	// check for failed allocation
	if (newRootDirLvl2IndexCluster == 0)
		return 0;

	char* rootDirIndex2 = new char[CLUSTER_SIZE];

	// set all its entries to empty, expect for the first one
	memset(rootDirIndex2, 0, CLUSTER_SIZE);

	ClusterNo* lvl2Entry = (ClusterNo*)rootDirIndex2;
	*lvl2Entry = dataCluster;
	ClusterAllocation::writeCluster(newRootDirLvl2IndexCluster, rootDirIndex2);

	delete[] rootDirIndex2;

	return newRootDirLvl2IndexCluster;
}

ClusterNo OpenFileStrategy::allocateAndSetDataCluster(char* fileName, char* extension) {
	ClusterNo newRootDirDataCluster = ClusterAllocation::allocateCluster();
	rootDirEntry* entry;

	if (newRootDirDataCluster == 0)
		return 0;

	char* rootDirData = new char[CLUSTER_SIZE];

	memset(rootDirData, 0, CLUSTER_SIZE);

	entry = (rootDirEntry*)rootDirData;

	for (unsigned i = 0; i < strlen(fileName); i++)
		(*entry)[i] = fileName[i];

	for (unsigned i = 0; i < strlen(extension); i++) {
		(*entry)[i + 8] = extension[i];
	}

	ClusterAllocation::writeCluster(newRootDirDataCluster, rootDirData);

	delete[] rootDirData;
	return newRootDirDataCluster;
}

char OpenFileStrategy::addEntryToDataDir(ClusterNo dataCluster, char* fileName, char* extension, ClusterNo& rootDirEntryArg) {
	char* rootDirData = new char[CLUSTER_SIZE];
	rootDirEntry* entry;
	bool finished = false;

	if (ClusterAllocation::readCluster(dataCluster, rootDirData) == -1)
		return 0;

	for (int k = 0; k < ENTRIES_PER_ROOT_DIR; k++) {
		entry = (rootDirEntry*)rootDirData + k;
		rootDirEntryArg = k;

		if ((*entry)[0] == 0) {
			for (unsigned i = 0; i < strlen(fileName); i++)
				(*entry)[i] = fileName[i];

			for (unsigned i = 0; i < strlen(extension); i++) {
				(*entry)[i + 8] = extension[i];
			}
			finished = true;
			break;
		}
	}

	if (finished)
		if (ClusterAllocation::writeCluster(dataCluster, rootDirData) == -1)
			return 0;

	delete[] rootDirData;

	if (finished)
		return 1;
	else
		return 0;
}

void OpenFileStrategy::splitFileName(char* fname, char** fileName, char** extension) {
	int fnameLen = strlen(fname);
	int dotPos = -1;
	for (int i = fnameLen - 1; i >= 0; i--) {
		if (fname[i] == '.') {
			dotPos = i;
			break;
		}
	}

	if (dotPos == -1) {
		*fileName = nullptr;
		return;
	}
	if (dotPos == fnameLen - 1) {
		*fileName = nullptr;
		return;
	}
	if (dotPos == 0) {
		*fileName = nullptr;
		return;
	}
	if (dotPos > 8) {
		*fileName = nullptr;
		return;
	}
	if (fnameLen - dotPos - 1 > 3) {
		*fileName = nullptr;
		return;
	}

	*fileName = new char[dotPos + 1];
	*extension = new char[(fnameLen - dotPos - 1) + 1];

	for (int i = 0; i < dotPos; i++)
		(*fileName)[i] = fname[i];
	(*fileName)[dotPos] = '\0';
	for (int i = dotPos + 1; i < fnameLen; i++)
		(*extension)[i - dotPos - 1] = fname[i];
	(*extension)[fnameLen - dotPos - 1] = '\0';
}