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
	FileSystemUtils::splitFileName(fname, &fileName, &extension);
	if (fileName == nullptr)
		return nullptr;

	// buffers for root directory level 1 index and
	// root directory level 2 index
	char* rootDirIndex1 = new char[CLUSTER_SIZE];
	char* rootDirIndex2 = new char[CLUSTER_SIZE];

	// read root directory level 1 index
	if (ClusterAllocation::readCluster(rootDirLvl1Index, rootDirIndex1) == 0) {
		std::cout << "error in open()" << std::endl;
		exit(1);
	}

	ClusterNo* rootDirIndex2Ptr, * rootDirDataPtr;
	bool finished = false;

	ClusterNo rootDirClusterArg = 0, rootDirEntryArg = 0;

	for (int i = 0; i < ENTRIES_PER_INDEX; i++) {
		// cluster number of root directory level 2 index
		rootDirIndex2Ptr = (ClusterNo*)rootDirIndex1 + i;

		// if it exists, try placing new file entry somewhere in it
		if (*rootDirIndex2Ptr != 0) {
			if (ClusterAllocation::readCluster(*rootDirIndex2Ptr, rootDirIndex2) == 0) {
				std::cout << "error in readCluster()" << std::endl;
				exit(1);
			}

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
					if (ClusterAllocation::writeCluster(*rootDirIndex2Ptr, rootDirIndex2) == 0) {
						std::cout << "error in readCluster()" << std::endl;
						exit(1);
					}

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
			if (ClusterAllocation::writeCluster(rootDirLvl1Index, rootDirIndex1) == 0) {
				std::cout << "error in readCluster()" << std::endl;
				exit(1);
			}
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

	if (newRootDirDataCluster == 0) {
		std::cout << "error in allocateAndSetDataCluster() 1" << std::endl;
		exit(1);
	}

	char* rootDirData = new char[CLUSTER_SIZE];

	memset(rootDirData, 0, CLUSTER_SIZE);

	entry = (rootDirEntry*)rootDirData;

	for (unsigned i = 0; i < strlen(fileName); i++)
		(*entry)[i] = fileName[i];

	for (unsigned i = 0; i < strlen(extension); i++) {
		(*entry)[i + 8] = extension[i];
	}

	if (ClusterAllocation::writeCluster(newRootDirDataCluster, rootDirData) == 0) {
		std::cout << "error in allocateAndSetDataCluster() 2" << std::endl;
		exit(1);
	}

	delete[] rootDirData;
	return newRootDirDataCluster;
}

char OpenFileStrategy::addEntryToDataDir(ClusterNo dataCluster, char* fileName, char* extension, ClusterNo& rootDirEntryArg) {
	char* rootDirData = new char[CLUSTER_SIZE];
	rootDirEntry* entry;
	bool finished = false;

	if (ClusterAllocation::readCluster(dataCluster, rootDirData) == 0) {
		std::cout << "error in addEntryToDataDir() 1" << std::endl;
		exit(1);
	}

	for (int k = 0; k < ENTRIES_PER_ROOT_DIR; k++) {
		entry = (rootDirEntry*)rootDirData + k;
		rootDirEntryArg = k;

		if ((*entry)[0] == 0) {
			for (unsigned i = 0; i < 32; i++)
				(*entry)[i] = 0;

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
		if (ClusterAllocation::writeCluster(dataCluster, rootDirData) == 0) {
			std::cout << "error in addEntryToDataDir() 2" << std::endl;
			exit(1);
		}

	delete[] rootDirData;

	if (finished)
		return 1;
	else
		return 0;
}