#include "kernelFS.h"

KernelFS::KernelFS() {
	clusterBuffer = new char[ClusterSize];
}

KernelFS::~KernelFS() {
	delete clusterBuffer;
}

char KernelFS::mount(Partition* partition) {
	// other threads get blocked if they attempt
	// to mount some other partition
	mountSem->wait();
	KernelFS::partition = partition;

	clusterCount = partition->getNumOfClusters();
	// determine length of bit vector
	bitVectorClusterCount = 1;
	for (; bitVectorClusterCount < clusterCount; bitVectorClusterCount++)
		if (ClusterSize * 8 * bitVectorClusterCount + bitVectorClusterCount > clusterCount)
			break;
	
	bitVectorSize = bitVectorClusterCount * ClusterSize;
	bitVector = new char[bitVectorSize];
	
	// read bit vector
	for (ClusterNo i = 0; i < bitVectorClusterCount; i++) {
		partition->readCluster(i, clusterBuffer);
		for (int j = 0; j < ClusterSize; j++)
			bitVector[i * ClusterSize + j] = clusterBuffer[j];
	}
	
	// set root directory index
	rootDirLvl1Index = bitVectorClusterCount;
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
	// wait until all files are closed
	allFilesClosed->wait();

	for (int i = 0; i < bitVectorClusterCount; i++)
		bitVector[i] = 0;
	bitVector[rootDirLvl1Index] = 0;
	for (int i = rootDirLvl1Index + 1; i < clusterCount; i++)
		bitVector[i] = 1;

	// set bit vector to all zeros
	for (int i = 0; i < bitVectorClusterCount; i++) {
		partition->writeCluster(i, clusterBuffer + ClusterSize * i);
	}

	for (int i = 0; i < bitVectorClusterCount; i++) {
		if (i == 0)
			clusterBuffer[0] = 0;

		partition->writeCluster(i, clusterBuffer);

		if (i == 0)
			clusterBuffer[0] = 1;
	}

	bitVector[0] = 0;
	bitVector[rootDirLvl1Index] = 0;

	for (int i = 0; i < ClusterSize; clusterBuffer[i++] = 1);
	partition->writeCluster(rootDirLvl1Index, clusterBuffer);

	allFilesClosed->signal();
}

FileCnt KernelFS::readRootDir() {
	FileCnt fileCnt = 0;

	int check = partition->readCluster(rootDirLvl1Index, clusterBuffer);

	if (check == -1)
		return -1;

	int entryPerCluster = (ClusterSize / 8) / sizeof(ClusterNo);
	int rootEntryPerCluster = (ClusterSize / 8) / 20;

	ClusterNo *lvl1Ptr, *lvl2Ptr;
	rootDirEntry *entryPtr;
	for (int i = 0; i < entryPerCluster; i++) {
		lvl1Ptr = (ClusterNo*)clusterBuffer + i;
		if (*lvl1Ptr == 0)
			continue;

		for (int j = 0; j < entryPerCluster; j++) {
			lvl2Ptr = (ClusterNo*)(*lvl1Ptr) + j;
			if (*lvl1Ptr == 0)
				continue;

			for (int k = 0; k < rootEntryPerCluster; k++) {
				entryPtr = (rootDirEntry*)(*lvl2Ptr) + k;

				if (entryPtr[0] == 0)
					continue;

				fileCnt++;
			}
		}
	}

	return fileCnt;
}

char KernelFS::doesExist(char* fname) {
	int check = partition->readCluster(rootDirLvl1Index, clusterBuffer);
	if (check == -1)
		return -1;

	int fnameLen = strlen(fname);
	if (fnameLen > 8 || fnameLen == 0)
		return -1;

	int entryPerCluster = (ClusterSize / 8) / sizeof(ClusterNo);
	int rootEntryPerCluster = (ClusterSize / 8) / 20;

	ClusterNo* lvl1Ptr, * lvl2Ptr;
	rootDirEntry* entryPtr;
	for (int i = 0; i < entryPerCluster; i++) {
		lvl1Ptr = (ClusterNo*)clusterBuffer + i;
		if (*lvl1Ptr == 0)
			continue;

		for (int j = 0; j < entryPerCluster; j++) {
			lvl2Ptr = (ClusterNo*)(*lvl1Ptr) + j;
			if (*lvl1Ptr == 0)
				continue;

			for (int k = 0; k < rootEntryPerCluster; k++) {
				entryPtr = (rootDirEntry*)(*lvl2Ptr) + k;

				if (entryPtr[0] == 0)
					continue;

				if (strcmp(fname, (char*)entryPtr) == 0)
					return 1;
			}
		}
	}

	return 0;
}

File* KernelFS::open(char* fname, char mode) {
	if (!doesExist(fname))
		return nullptr;

	// create new File and KernelFile objects
}

// write functions to allocate and deallocate cluster

char KernelFS::deleteFile(char* fname) {
	// truncate it
	// update corresponding root dir entry
	return 0;
}
