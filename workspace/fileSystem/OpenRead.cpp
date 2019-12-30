#include "OpenRead.h"

KernelFile* OpenRead::open() {
	// if files exists it is truncated
	// otherwise, new one is created
	if (KernelFS::doesExist(fname))
		return nullptr;
	
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

	return new KernelFile(rootDirClusterArg, rootDirEntryArg, false);
}

OpenRead::OpenRead(char* fname, ClusterNo rootLvl1Index) :
	OpenFileStrategy(fname, rootLvl1Index) {}
