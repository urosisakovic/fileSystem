#include "kernelFS.h"

// TODO: Check if any readCluster return -1

KernelFS::KernelFS() {
	clusterBuffer = new char[CLUSTER_SIZE];
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
		partition->readCluster(i, clusterBuffer);
		for (int j = 0; j < CLUSTER_SIZE; j++)
			bitVector[i * CLUSTER_SIZE + j] = clusterBuffer[j];
	}
	
	// set root directory index
	rootDirLvl1Index = bitVectorClusterSize;
}

char KernelFS::unmount() {
	// wait until all files are closed
	allFilesClosed->wait();

	partition = nullptr;
	delete[] bitVector;

	// other partition can now be mounted
	mountSem->signal();	
	allFilesClosed->signal();
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

	for (int i = 0; i < bitVectorByteSize; bitVector[i] = allSet);
	for (int i = 0; i < bitVectorClusterSize; i++)
		markAllocated(i);
	markAllocated(rootDirLvl1Index);

	// update bit vector on disk
	for (int i = 0; i < bitVectorClusterSize; i++) {
		memcpy(clusterBuffer, bitVector + i * CLUSTER_SIZE, CLUSTER_SIZE);
		partition->writeCluster(i, clusterBuffer);
	}

	// update root directory level 1 index
	for (int i = 0; i < CLUSTER_SIZE; clusterBuffer[i++] = 1);
	partition->writeCluster(rootDirLvl1Index, clusterBuffer);

	allFilesClosed->signal();
}

FileCnt KernelFS::readRootDir() {
	FileCnt fileCnt = 0;

	ClusterNo *lvl1Ptr, *lvl2Ptr;
	rootDirEntry *fileEntry;

	char* lvl1Buffer = new char[CLUSTER_SIZE];
	char* lvl2Buffer = new char[CLUSTER_SIZE];

	if (partition->readCluster(rootDirLvl1Index, lvl1Buffer) == -1)
		return -1;

	int entryPerIndex = CLUSTER_SIZE / sizeof(ClusterNo);
	int entryPerDataDir = CLUSTER_SIZE / sizeof(rootDirEntry);

	for (int i = 0; i < entryPerIndex; i++) {
		lvl1Ptr = (ClusterNo*)lvl1Buffer + i;

		if (*lvl1Ptr == 0)
			continue;

		if (partition->readCluster((*lvl1Ptr), lvl2Buffer) == -1)
			return -1;

		for (int j = 0; j < entryPerIndex; j++) {
			lvl2Ptr = (ClusterNo*)lvl2Buffer + j;

			if (*lvl2Ptr == 0)
				continue;

			if (partition->readCluster((*lvl2Ptr), clusterBuffer) == -1)
				return -1;

			for (int k = 0; k < entryPerDataDir; k++) {
				fileEntry = (rootDirEntry*)(*lvl2Ptr) + k;

				if (fileEntry[0] == 0)
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
	
	ClusterNo* lvl1Ptr, * lvl2Ptr;
	rootDirEntry* fileEntry;

	char* lvl1Buffer = new char[CLUSTER_SIZE];
	char* lvl2Buffer = new char[CLUSTER_SIZE];

	if (partition->readCluster(rootDirLvl1Index, lvl1Buffer) == -1)
		return -1;

	int entryPerIndex = CLUSTER_SIZE / sizeof(ClusterNo);
	int entryPerDataDir = CLUSTER_SIZE / sizeof(rootDirEntry);

	for (int i = 0; i < entryPerIndex; i++) {
		lvl1Ptr = (ClusterNo*)lvl1Buffer + i;

		if (*lvl1Ptr == 0)
			continue;

		if (partition->readCluster((*lvl1Ptr), lvl2Buffer) == -1)
			return -1;

		for (int j = 0; j < entryPerIndex; j++) {
			lvl2Ptr = (ClusterNo*)lvl2Buffer + j;

			if (*lvl2Ptr == 0)
				continue;

			if (partition->readCluster((*lvl2Ptr), clusterBuffer) == -1)
				return -1;

			for (int k = 0; k < entryPerDataDir; k++) {
				fileEntry = (rootDirEntry*)(*lvl2Ptr) + k;

				if (strcmp(fname, (char*)fileEntry) == 0)
					return 1;
			}
		}
	}

	delete[] lvl1Buffer;
	delete[] lvl2Buffer;

	return 0;
}

File* KernelFS::open(char* fname, char mode) {
	if (!doesExist(fname))
		return nullptr;

	// create new File and KernelFile objects
}

//TODO: write functions to allocate and deallocate clusters

char KernelFS::deleteFile(char* fname) {
	// truncate it
	// update corresponding root dir entry
	return 0;
}

KernelFS::~KernelFS() {
	delete clusterBuffer;
}

void KernelFS::markAllocated(ClusterNo clusterId) {
	unsigned correspondingByte = clusterId / 8;
	unsigned correspondingBit = clusterId % 8;

	bitVector[correspondingByte] |= (1 << correspondingBit);
}

void KernelFS::markDeallocated(ClusterNo clusterId) {
	unsigned correspondingByte = clusterId / 8;
	unsigned correspondingBit = clusterId % 8;

	bitVector[correspondingByte] &= ~(1 << correspondingBit);
}
