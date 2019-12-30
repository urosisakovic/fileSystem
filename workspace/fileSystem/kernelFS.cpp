#include "kernelFS.h"
#include "OpenAppend.h"
#include "OpenRead.h"
#include "OpenWrite.h"

// TODO: Check if any readCluster return -1
// TODO: Use bool instead of char
// TODO: Use inline functions
// TODO: Where to close a file?
// TODO: In case of a failed allocation deallocate all the previously allocated clusters.

// initialize static variables
Partition* KernelFS::partition = nullptr;
ClusterNo KernelFS::clusterCount = 0;
char* KernelFS::bitVector = nullptr;
unsigned KernelFS::bitVectorByteSize = 0;
ClusterNo KernelFS::bitVectorClusterSize = 0;
ClusterNo KernelFS::rootDirLvl1Index = 0;
char* KernelFS::clusterBuffer = new char[CLUSTER_SIZE];
Semaphore* KernelFS::mountSem = new Semaphore();
Semaphore* KernelFS::allFilesClosed = new Semaphore();
OpenFileStrategy* KernelFS::openFile = nullptr;
std::unordered_map<std::string, File*>* KernelFS::openFiles = new std::unordered_map<std::string, File*>();

// TODO: If other thread tries to mount that same partition, 
//		 what should happen?
char KernelFS::mount(Partition* partition) {
	// other threads get blocked if they attempt
	// to mount some other partition
	mountSem->wait();

	KernelFS::partition = partition;

	ClusterAllocation::setPartition(partition);

	clusterCount = partition->getNumOfClusters();
	// determine length of bit vector and allocate it
	bitVectorClusterSize = 1;
	for (; bitVectorClusterSize < clusterCount; bitVectorClusterSize++)
		if (BITS_PER_CLUSTER * bitVectorClusterSize + bitVectorClusterSize > clusterCount)
			break;
	
	bitVectorByteSize = bitVectorClusterSize * CLUSTER_SIZE;
	bitVector = new char[bitVectorByteSize];
	
	// read bit vector
	for (ClusterNo i = 0; i < bitVectorClusterSize; i++) {
		if (ClusterAllocation::readCluster(i, clusterBuffer) == -1)
			return 0;
		memcpy(bitVector + i * CLUSTER_SIZE, clusterBuffer, CLUSTER_SIZE);
	}

	ClusterAllocation::setBitVector(bitVectorByteSize, bitVector);
	
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
	for (int i = 0; i < 8; i++)
		allSet += 1 << i;

	for (unsigned i = 0; i < bitVectorByteSize; bitVector[i++] = allSet);
	for (unsigned i = 0; i < bitVectorClusterSize; i++)
		ClusterAllocation::markAllocated(i);
	ClusterAllocation::markAllocated(rootDirLvl1Index);

	// update bit vector on disk
	for (unsigned i = 0; i < bitVectorClusterSize; i++) {
		memcpy(clusterBuffer, bitVector + i * CLUSTER_SIZE, CLUSTER_SIZE);
		if (ClusterAllocation::writeCluster(i, clusterBuffer) == -1)
			return 0;
	}

	// update root directory level 1 index
	memset(clusterBuffer, 0, CLUSTER_SIZE);
	if (ClusterAllocation::writeCluster(rootDirLvl1Index, clusterBuffer) == -1)
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

	if (ClusterAllocation::readCluster(rootDirLvl1Index, lvl1Buffer) == -1)
		return -1;

	for (int i = 0; i < ENTRIES_PER_INDEX; i++) {
		lvl1Ptr = (ClusterNo*)lvl1Buffer + i;

		if (*lvl1Ptr == 0)
			continue;

		if (ClusterAllocation::readCluster((*lvl1Ptr), lvl2Buffer) == -1)
			return -1;

		for (int j = 0; j < ENTRIES_PER_INDEX; j++) {
			lvl2Ptr = (ClusterNo*)lvl2Buffer + j;

			if (*lvl2Ptr == 0)
				continue;

			if (ClusterAllocation::readCluster((*lvl2Ptr), clusterBuffer) == -1)
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
	//TODO: Fix this design
	(new OpenAppend(fname, 0))->splitFileName(fname, &fileName, &extension);
	if (fileName == nullptr)
		return -1;
	
	ClusterNo* lvl1Ptr, * lvl2Ptr;
	rootDirEntry* fileEntry;

	char* lvl1Buffer = new char[CLUSTER_SIZE];
	char* lvl2Buffer = new char[CLUSTER_SIZE];
	char* fileEntryFileName = new char[9];
	char* fileEntryExtension = new char[4];

	if (ClusterAllocation::readCluster(rootDirLvl1Index, lvl1Buffer) == -1)
		return -1;

	for (int i = 0; i < ENTRIES_PER_INDEX; i++) {
		lvl1Ptr = (ClusterNo*)lvl1Buffer + i;

		if (*lvl1Ptr == 0)
			continue;

		if (ClusterAllocation::readCluster((*lvl1Ptr), lvl2Buffer) == -1)
			return -1;

		for (int j = 0; j < ENTRIES_PER_INDEX; j++) {
			lvl2Ptr = (ClusterNo*)lvl2Buffer + j;

			if (*lvl2Ptr == 0)
				continue;

			if (ClusterAllocation::readCluster((*lvl2Ptr), clusterBuffer) == -1)
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

File* KernelFS::open(char* fname, char mode) {
	if (mode == 'r')
		openFile = new OpenRead(fname, rootDirLvl1Index);
	else if (mode == 'w')
		openFile = new OpenWrite(fname, rootDirLvl1Index);
	else if (mode == 'a')
		openFile = new OpenAppend(fname, rootDirLvl1Index);
	else {
		std::cout << "Invalid mode." << std::endl;
		exit(1);
	}

	File *f = new File();
	f->myImpl = openFile->open();

	return f;
}

char KernelFS::deleteFile(char* fname) {
	if (openFiles->find(fname) == openFiles->end())
		return 0;

	(*openFiles)[fname]->truncate;

	ClusterNo rootDirCluster = (*openFiles)[fname]->myImpl->filePtr->getRootDirCluster();
	ClusterNo rootDirEntry = (*openFiles)[fname]->myImpl->filePtr->getRootDirEntry();

	if (FileSystemUtils::emptyRootDirEntry(rootDirCluster, rootDirEntry) == 0)
		return 0;

	return 1;
}