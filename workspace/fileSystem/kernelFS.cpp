#include "kernelFS.h"
#include <iostream>
// TODO: Check if any readCluster return -1
// TODO: Use bool instead of char
// TODO: Use inline functions
// TODO: Where to close a file?
// TODO: In case of a failed allocation deallocate all the previously allocated clusters.

// initialize static variables
Partition* KernelFS::partition = nullptr;

ClusterNo KernelFS::clusterCount = 0;

char* KernelFS::bitVector = nullptr;
int KernelFS::bitVectorByteSize = 0;
ClusterNo KernelFS::bitVectorClusterSize = 0;
ClusterNo KernelFS::rootDirLvl1Index = 0;

char* KernelFS::clusterBuffer = new char[CLUSTER_SIZE];

Semaphore* KernelFS::mountSem = new Semaphore();
Semaphore* KernelFS::allFilesClosed = new Semaphore();

KernelFS::KernelFS() {
}

// TODO: If other thread tries to mount that same partition, 
//		 what should happen?
char KernelFS::mount(Partition* partition) {
	// other threads get blocked if they attempt
	// to mount some other partition
	mountSem->wait();

	KernelFS::partition = partition;

	clusterCount = partition->getNumOfClusters();
	// determine length of bit vector and allocate it
	bitVectorClusterSize = 1;
	for (; bitVectorClusterSize < clusterCount; bitVectorClusterSize++)
		if (BITS_PER_CLUSTER * bitVectorClusterSize + bitVectorClusterSize > clusterCount)
			break;
	
	bitVectorByteSize = bitVectorClusterSize * CLUSTER_SIZE;
	bitVector = new char[bitVectorByteSize];
	
	// TODO: Use memset here.
	// read bit vector
	for (ClusterNo i = 0; i < bitVectorClusterSize; i++) {
		if (partition->readCluster(i, clusterBuffer) == -1)
			return 0;
		for (int j = 0; j < CLUSTER_SIZE; j++)
			bitVector[i * CLUSTER_SIZE + j] = clusterBuffer[j];
	}
	
	// set root directory index
	rootDirLvl1Index = bitVectorClusterSize;

	return 1;
}

char KernelFS::unmount() {
	// wait until all files are closed
	allFilesClosed->wait();

	partition = nullptr;
	delete[] bitVector;

	// other partition can now be mounted
	mountSem->signal();	
	allFilesClosed->signal();

	return 1;
}

char KernelFS::format() {
	if (!partition)
		return 0;

	// wait until all files are closed
	allFilesClosed->wait();

	// update bit vector in KernelFS object
	unsigned char allSet = 0;
	for (int i = 0; i < 7; i++)
		allSet += 1 << i;

	for (int i = 0; i < bitVectorByteSize; bitVector[i++] = allSet);
	for (int i = 0; i < bitVectorClusterSize; i++)
		markAllocated(i);
	markAllocated(rootDirLvl1Index);

	// update bit vector on disk
	for (int i = 0; i < bitVectorClusterSize; i++) {
		memcpy(clusterBuffer, bitVector + i * CLUSTER_SIZE, CLUSTER_SIZE);
		if (partition->writeCluster(i, clusterBuffer) == -1)
			return 0;
	}

	// update root directory level 1 index
	memset(clusterBuffer, 0, CLUSTER_SIZE);
	if (partition->writeCluster(rootDirLvl1Index, clusterBuffer) == -1)
		return 0;

	allFilesClosed->signal();

	return 1;
}

FileCnt KernelFS::readRootDir() {
	FileCnt fileCnt = 0;

	ClusterNo *lvl1Ptr, *lvl2Ptr;
	rootDirEntry *fileEntry;

	char* lvl1Buffer = new char[CLUSTER_SIZE];
	char* lvl2Buffer = new char[CLUSTER_SIZE];

	if (partition->readCluster(rootDirLvl1Index, lvl1Buffer) == -1)
		return -1;

	for (int i = 0; i < ENTRIES_PER_INDEX; i++) {
		lvl1Ptr = (ClusterNo*)lvl1Buffer + i;

		if (*lvl1Ptr == 0)
			continue;

		if (partition->readCluster((*lvl1Ptr), lvl2Buffer) == -1)
			return -1;

		for (int j = 0; j < ENTRIES_PER_INDEX; j++) {
			lvl2Ptr = (ClusterNo*)lvl2Buffer + j;

			if (*lvl2Ptr == 0)
				continue;

			if (partition->readCluster((*lvl2Ptr), clusterBuffer) == -1)
				return -1;

			for (int k = 0; k < ENTRIES_PER_ROOT_DIR; k++) {
				fileEntry = (rootDirEntry*)clusterBuffer + k;

				if ((*fileEntry[0]) == 0)
					continue;
				else
					fileCnt++;
			}
		}
	}

	delete[] lvl1Buffer;
	delete[] lvl2Buffer;

	return fileCnt;
}

char KernelFS::doesExist(char* fname) {
	int fnameLen = strlen(fname);
	if (fnameLen > 8 || fnameLen == 0)
		return -1;

	char* fileName = nullptr, *extension = nullptr;
	splitFileName(fname, &fileName, &extension);
	if (fileName == nullptr)
		return -1;
	
	ClusterNo* lvl1Ptr, * lvl2Ptr;
	rootDirEntry* fileEntry;

	char* lvl1Buffer = new char[CLUSTER_SIZE];
	char* lvl2Buffer = new char[CLUSTER_SIZE];
	char* fileEntryFileName = new char[9];
	char* fileEntryExtension = new char[4];

	if (partition->readCluster(rootDirLvl1Index, lvl1Buffer) == -1)
		return -1;

	for (int i = 0; i < ENTRIES_PER_INDEX; i++) {
		lvl1Ptr = (ClusterNo*)lvl1Buffer + i;

		if (*lvl1Ptr == 0)
			continue;

		if (partition->readCluster((*lvl1Ptr), lvl2Buffer) == -1)
			return -1;

		for (int j = 0; j < ENTRIES_PER_INDEX; j++) {
			lvl2Ptr = (ClusterNo*)lvl2Buffer + j;

			if (*lvl2Ptr == 0)
				continue;

			if (partition->readCluster((*lvl2Ptr), clusterBuffer) == -1)
				return -1;

			for (int k = 0; k < ENTRIES_PER_ROOT_DIR; k++) {
				fileEntry = (rootDirEntry*)clusterBuffer + k;

				for (int i = 0; i < 8; i++)
					fileEntryFileName[i] = (*fileEntry)[i];
				fileEntryFileName[8] = '\0';

				for (int i = 0; i < 3; i++)
					fileEntryExtension[i] = (*fileEntry)[i + 8];
				fileEntryExtension[3] = '\0';

				if ((strcmp(fileName, fileEntryFileName) == 0) &&
					(strcmp(extension, fileEntryExtension) == 0)) {
					delete[] fileEntryFileName;
					delete[] fileEntryExtension;
					delete[] lvl1Buffer;
					delete[] lvl2Buffer;
					return 1;
				}
			}
		}
	}

	delete[] lvl1Buffer;
	delete[] lvl2Buffer;
	delete[] fileEntryFileName;
	delete[] fileEntryExtension;

	return 0;
}

void KernelFS::splitFileName(char* fname, char** fileName, char** extension) {
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

	std::cout << "File name: " << *fileName << std::endl;
	std::cout << "Extension: " << *extension << std::endl;
}

ClusterNo KernelFS::allocateAndSetDataCluster(char* fileName, char* extension) {
	ClusterNo newRootDirDataCluster = allocateCluster();
	rootDirEntry* entry;

	if (newRootDirDataCluster == 0)
		return 0;

	char* rootDirData = new char[CLUSTER_SIZE];

	memset(rootDirData, 0, CLUSTER_SIZE);

	entry = (rootDirEntry*)rootDirData;

	for (int i = 0; i < strlen(fileName); i++)
		(*entry)[i] = fileName[i];

	for (int i = 0; i < strlen(extension); i++) {
		(*entry)[i + 8] = extension[i];
	}

	partition->writeCluster(newRootDirDataCluster, rootDirData);

	delete[] rootDirData;
	return newRootDirDataCluster;
}

char KernelFS::addEntryToDataDir(ClusterNo dataCluster, char* fileName, char* extension) {
	char* rootDirData = new char[CLUSTER_SIZE];
	rootDirEntry* entry;
	bool finished = false;

	if (partition->readCluster(dataCluster, rootDirData) == -1)
		return 0;

	for (int k = 0; k < ENTRIES_PER_ROOT_DIR; k++) {
		entry = (rootDirEntry*)rootDirData + k;

		if ((*entry)[0] == 0) {
			std::cout << "Saved: " << fileName << std::endl;

			for (int i = 0; i < strlen(fileName); i++)
				(*entry)[i] = fileName[i];

			for (int i = 0; i < strlen(extension); i++) {
				(*entry)[i + 8] = extension[i];
			}
			finished = true;
			break;
		}
	}

	if (finished)
		if (partition->writeCluster(dataCluster, rootDirData) == -1)
			return 0;

	delete[] rootDirData;

	if (finished)
		return 1;
	else
		return 0;
}

ClusterNo KernelFS::allocateAndSetLvl2Cluster(ClusterNo dataCluster) {
	ClusterNo newRootDirLvl2IndexCluster = allocateCluster();
	// check for failed allocation
	if (newRootDirLvl2IndexCluster == 0)
		return 0;

	char* rootDirIndex2 = new char[CLUSTER_SIZE];

	// set all its entries to empty, expect for the first one
	memset(rootDirIndex2, 0, CLUSTER_SIZE);

	ClusterNo* lvl2Entry = (ClusterNo*)rootDirIndex2;
	*lvl2Entry = dataCluster;
	partition->writeCluster(newRootDirLvl2IndexCluster, rootDirIndex2);

	delete[] rootDirIndex2;

	return newRootDirLvl2IndexCluster;
}

File* KernelFS::open(char* fname, char mode) {
	char *fileName = nullptr, *extension = nullptr;
	splitFileName(fname, &fileName, &extension);
	if (fileName == nullptr)
		return nullptr;
	
	if (mode == 'r') {
		// file must exist in reading mode
		if (!doesExist(fname))
			return nullptr;

		// open an existing file and set pointer to the beginning
	}
	if (mode == 'w') {
		// if files exists it is truncated
		// otherwise, new one is created
		if (doesExist(fname))
			deleteFile(fname);

		// buffers for root directory level 1 index and
		// root directory level 2 index
		char* rootDirIndex1 = new char[CLUSTER_SIZE];
		char* rootDirIndex2 = new char[CLUSTER_SIZE];

		// read root directory level 1 index
		if (partition->readCluster(rootDirLvl1Index, rootDirIndex1) == -1)
			return nullptr;

		ClusterNo* rootDirIndex2Ptr, *rootDirDataPtr;
		bool finished = false;

		for (int i = 0; i < ENTRIES_PER_INDEX; i++) {
			// cluster number of root directory level 2 index
			rootDirIndex2Ptr = (ClusterNo*)rootDirIndex1 + i;

			// if it exists, try placing new file entry somewhere in it
			if (*rootDirIndex2Ptr != 0) {
				if (partition->readCluster(*rootDirIndex2Ptr, rootDirIndex2) == -1)
					return nullptr;

				for (int j = 0; j < ENTRIES_PER_INDEX; j++) {
					rootDirDataPtr = (ClusterNo*)rootDirIndex2 + j;

					if (*rootDirDataPtr != 0) {
						finished = addEntryToDataDir(*rootDirDataPtr, fileName, extension);
					}
					else {
						ClusterNo dataClus = allocateAndSetDataCluster(fileName, extension);
						// check for failure
						if (dataClus == 0)
							return nullptr;

						*rootDirDataPtr = dataClus;
						if (partition->writeCluster(*rootDirIndex2Ptr, rootDirIndex2) == -1)
							return nullptr;
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

				ClusterNo lvl2Clus = allocateAndSetLvl2Cluster(dataClus);
				// check for failure
				if (lvl2Clus == 0)
					return nullptr;

				*rootDirIndex2Ptr = lvl2Clus;
				if (partition->writeCluster(rootDirLvl1Index, rootDirIndex1) == -1)
					return nullptr;
				finished = true;
			}

			if (finished)
				break;
		}

		delete[] rootDirIndex1;
		delete[] rootDirIndex2;

		/*File* f = new File();
		KernelFile* kf = new KernelFile(partition, 0, 0, 0, 0, 0);

		openFiles[fname] = f;*/

		return nullptr;

		// create new file
	}
	if (mode == 'a') {
		// file must exist in append mode
		if (!doesExist(fname))
			return nullptr;

		// open an existing file and set pointer to the end
	}

	return nullptr;
}

//TODO: write functions to allocate and deallocate clusters

char KernelFS::deleteFile(char* fname) {
	// truncate it
	// update corresponding root dir entry
	return 0;
}

KernelFS::~KernelFS() {
}

ClusterNo KernelFS::allocateCluster() {
	for (ClusterNo i = 1; i < clusterCount; i++) {
		if (!checkAllocated(i)) {
			markAllocated(i);
			std::cout << "Allocated cluster: " << i << std::endl;
			// system("pause");
			return i;
		}
	}

	return 0;
}

char KernelFS::deallocateCluster(ClusterNo freeClusterIdx) {
	if (checkAllocated(freeClusterIdx))
		return 0;

	markDeallocated(freeClusterIdx);
}

char KernelFS::setLength(ClusterNo rootDirCluster, ClusterNo rootEntry, unsigned size) {
	if (partition->readCluster(rootDirCluster, clusterBuffer) == -1)
		return 0;

	rootDirEntry* entry = (rootDirEntry*)clusterBuffer + rootEntry;

	unsigned* length = (unsigned*)entry + 16;
	*length = size;

	if (partition->writeCluster(rootDirCluster, clusterBuffer) == -1)
		return 0;

	return 1;
}

void KernelFS::markAllocated(ClusterNo clusterId) {
	unsigned correspondingByte = clusterId / 8;
	unsigned correspondingBit = 7 - clusterId % 8;

	bitVector[correspondingByte] &= ~(1 << correspondingBit);
}

void KernelFS::markDeallocated(ClusterNo clusterId) {
	unsigned correspondingByte = clusterId / 8;
	unsigned correspondingBit = 7 - clusterId % 8;

	bitVector[correspondingByte] |= (1 << correspondingBit);
}

char KernelFS::checkAllocated(ClusterNo clusterId) {
	unsigned correspondingByte = clusterId / 8;
	unsigned correspondingBit = 7 - clusterId % 8;

	if (correspondingByte >= bitVectorByteSize)
		return -1;

	return ((bitVector[correspondingByte] & (1 << correspondingBit)) == 0);
}
