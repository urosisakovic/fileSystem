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

KernelFile* OpenFileStrategy::open() {
	char* fileName = nullptr, * extension = nullptr;
	splitFileName(fname, &fileName, &extension);
	if (fileName == nullptr)
		return nullptr;

	// buffers for root directory level 1 index and
	// root directory level 2 index
	char* rootDirIndex1 = new char[CLUSTER_SIZE];
	char* rootDirIndex2 = new char[CLUSTER_SIZE];

	// read root directory level 1 index
	if (ClusterAllocation::readCluster(rootDirLvl1Index, rootDirIndex1) == -1)
		return nullptr;

	ClusterNo* rootDirIndex2Ptr, * rootDirDataPtr;
	bool finished = false;

	ClusterNo rootDirClusterArg = 0, rootDirEntryArg = 0;

	for (int i = 0; i < ENTRIES_PER_INDEX; i++) {
		// cluster number of root directory level 2 index
		rootDirIndex2Ptr = (ClusterNo*)rootDirIndex1 + i;

		// if it exists, try placing new file entry somewhere in it
		if (*rootDirIndex2Ptr != 0) {
			if (ClusterAllocation::readCluster(*rootDirIndex2Ptr, rootDirIndex2) == -1)
				return nullptr;

			for (int j = 0; j < ENTRIES_PER_INDEX; j++) {
				rootDirDataPtr = (ClusterNo*)rootDirIndex2 + j;

				if (*rootDirDataPtr != 0) {
					finished = addEntryToDataDir(*rootDirDataPtr, fileName, extension, rootDirEntryArg);
					if (finished)
						rootDirClusterArg = *rootDirDataPtr;
				}
				else {
					ClusterNo dataClus = allocateAndSetDataCluster(fileName, extension);
					// check for failure
					if (dataClus == 0)
						return nullptr;

					*rootDirDataPtr = dataClus;
					if (ClusterAllocation::writeCluster(*rootDirIndex2Ptr, rootDirIndex2) == -1)
						return nullptr;

					rootDirClusterArg = dataClus;
					rootDirEntryArg = j;

					finished = true;
				}

				if (finished)
					break;
			}
		}
		// if it does not exists, create it
		else {
			ClusterNo dataClus = allocateAndSetDataCluster(fileName, extension);
			// check for failure
			if (dataClus == 0)
				return nullptr;

			rootDirClusterArg = dataClus;
			rootDirEntryArg = 0;

			ClusterNo lvl2Clus = allocateAndSetLvl2Cluster(dataClus);
			// check for failure
			if (lvl2Clus == 0)
				return nullptr;

			*rootDirIndex2Ptr = lvl2Clus;
			if (ClusterAllocation::writeCluster(rootDirLvl1Index, rootDirIndex1) == -1)
				return nullptr;
			finished = true;
		}

		if (finished)
			break;
	}

	delete[] rootDirIndex1;
	delete[] rootDirIndex2;

	return new KernelFile(rootDirClusterArg, rootDirEntryArg, true);
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