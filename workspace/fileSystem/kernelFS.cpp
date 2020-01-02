#include "kernelFS.h"

// initialize static variables
Partition* KernelFS::partition = nullptr;

char* KernelFS::bitVector = nullptr;
unsigned KernelFS::bitVectorByteSize = 0;
ClusterNo KernelFS::bitVectorClusterSize = 0;
ClusterNo KernelFS::clusterCount = 0;

ClusterNo KernelFS::rootDirLvl1Index = 0;

std::unordered_map<std::string, File*>* KernelFS::openFiles = new std::unordered_map<std::string, File*>();


char KernelFS::mount(Partition* partition) {
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
		if (ClusterAllocation::readCluster(i, clusterBuffer) == 0) {
			std::cout << "error in mount()" << std::endl;
			exit(1);
		}
		memcpy(bitVector + i * CLUSTER_SIZE, clusterBuffer, CLUSTER_SIZE);
	}
	ClusterAllocation::setBitVector(bitVectorByteSize, bitVector);
	
	// set root directory index
	rootDirLvl1Index = bitVectorClusterSize;

	return 1;
}

char KernelFS::unmount() {
	if (partition == nullptr)
		return 0;

	partition = nullptr;
	delete[] bitVector;

	return 1;
}

char KernelFS::format() {
	if (!partition)
		return 0;

	// update bit vector in KernelFS object
	unsigned char allSet = 0;
	for (int i = 0; i < 8; i++)
		allSet += 1 << i;

	memset(bitVector, allSet, bitVectorByteSize);
	ClusterAllocation::setBitVector(bitVectorByteSize, bitVector);
	for (unsigned i = 0; i < bitVectorClusterSize; i++)
		ClusterAllocation::markAllocated(i);
	ClusterAllocation::markAllocated(rootDirLvl1Index);

	// update bit vector on disk
	for (unsigned i = 0; i < bitVectorClusterSize; i++) {
		memcpy(clusterBuffer, bitVector + i * CLUSTER_SIZE, CLUSTER_SIZE);
		if (ClusterAllocation::writeCluster(i, clusterBuffer) == 0) {
			std::cout << "error in format() 1" << std::endl;
			exit(1);
		}
	}

	// update root directory level 1 index
	memset(clusterBuffer, 0, CLUSTER_SIZE);
	if (ClusterAllocation::writeCluster(rootDirLvl1Index, clusterBuffer) == 0) {
		std::cout << "error in format() 2" << std::endl;
		exit(1);
	}

	return 1;
}

FileCnt KernelFS::readRootDir() {
	FileCnt fileCnt = 0;

	for (int i = 0; i < ENTRIES_PER_INDEX; i++) {
		if (lvl2IndexNodes[i].unpopulated())
			break;

		fileCnt += lvl2IndexNodes[i].countEntries();
	}

	return fileCnt;
}

char KernelFS::doesExist(char* fname) {
	return (openRead(fname) != nullptr);
}

KernelFile* KernelFS::getFile(char* fname) {
	// no lvl2 index clusters allocated
	if (lvl2IndexNodes[0].unpopulated())
		return 0;

	// pick a candidate cluster which may contain
	// file with the given name
	int currComp, nextComp;
	ClusterNo potentialCluster = 0;
	for (int i = 0; i < ENTRIES_PER_INDEX - 1; i++) {
		if (lvl2IndexNodes[i + 1].unpopulated()) {
			potentialCluster = lvl2IndexNodes[i + 1].clusterIdx;
			break;
		}

		currComp = strcmp(fname, lvl2IndexNodes[i].smallestKey);
		nextComp = strcmp(fname, lvl2IndexNodes[i + 1].smallestKey);

		if (currComp >= 0 && nextComp < 0) {
			potentialCluster = lvl2IndexNodes[i].clusterIdx;
			break;
		}
	}

	// no potential cluster
	if (potentialCluster == 0)
		return 0;

	// read potential cluster
	char clusterBuffer[CLUSTER_SIZE];
	if (ClusterAllocation::readCluster(potentialCluster, clusterBuffer) == 0)
		return 0;

	// search potential cluster for the given key name
	rootDirEntry* fileEntry;
	char fileName[9], extension[4], fullName[13];
	fileName[8] = extension[3] = fullName[12] = '\0';

	for (int k = 0; k < ENTRIES_PER_ROOT_DIR; k++) {
		fileEntry = (rootDirEntry*)clusterBuffer + k;

		memcpy(fileName, (*fileEntry), 8);
		memcpy(fileName, (char*)(*fileEntry) + 8, 3);

		memcpy(fullName, fileName, strlen(fileName));
		fullName[strlen(fileName)] = '.';
		memcpy(fullName + strlen(fileName) + 1, extension, strlen(extension));

		if ((strcmp(fname, fullName) == 0))
			return new KernelFile(potentialCluster, k, false);
	}

	// potential cluster does not contain given key
	return nullptr;
}

KernelFile* KernelFS::createFile(char* fname) {
	return nullptr;
}

KernelFile* KernelFS::openRead(char* fname) {
	return getFile(fname);
}

KernelFile* KernelFS::openWrite(char* fname) {
	KernelFile *kf = getFile(fname);

	if (kf == nullptr) {
		kf = createFile(fname);
	}
	else {
		kf->seek(0);
		kf->truncate();
		kf->enableWriting();
	}

	return kf;
}

KernelFile* KernelFS::openAppend(char* fname) {
	KernelFile *kf = getFile(fname);

	if (kf != nullptr) {
		kf->seek(kf->getFileSize() - 1);
		kf->enableWriting();
	}

	return kf;
}

File* KernelFS::open(char* fname, char mode) {
	KernelFile* kf;
	
	if (mode == 'r')
		kf = openRead(fname);
	else if (mode == 'w')
		kf = openWrite(fname);
	else if (mode == 'a')
		kf = openAppend(fname);
	else
		return nullptr;

	if (kf == nullptr)
		return nullptr;

	File* f = new File();
	// set implementation object of the file
	f->myImpl = kf;

	// set name of the file
	f->myImpl->fname = new char[strlen(fname)];
	memcpy(f->myImpl->fname, fname, strlen(fname) + 1);

	// mark the file as open
	(*openFiles)[fname] = f;

	return f;
}

char KernelFS::close(char* fname) {
	// if file is not open, return error code 0
	if (openFiles->find(fname) == openFiles->end())
		return 0;

	// remove file from the hash table of ordered files
	openFiles->erase(openFiles->find(fname));
	return 1;
}

char KernelFS::deleteFile(char* fname) {
	// check if files exist (and if it does, locate it)
	KernelFile* kf = getFile(fname);
	if (kf == nullptr)
		return 0;

	// delete file data
	if (kf->seek(0) == 0)
		return 0;
	if (kf->truncate() == 0)
		return 0;

	// update internal look-up structure

	return 1;
}
