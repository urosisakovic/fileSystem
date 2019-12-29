#include "kernelFile.h"

KernelFile::KernelFile(KernelFS *fs, Partition *p, ClusterNo rootDirCluster, ClusterNo rootDirEntry) {
	this->fs = fs;
	this->partition = p;
	this->filePtr = new FilePointer(rootDirCluster, rootDirEntry);

	clusterBuffer = new char[CLUSTER_SIZE];
}

KernelFile::~KernelFile() {
	delete filePtr;
	delete[] clusterBuffer;
}

char KernelFile::write(BytesCnt bytesCnt, char* buffer) {
	if (bytesCnt == 0)
		return 1;
	
	filePtr->ensureDataCluster();

	unsigned currClusterFree = CLUSTER_SIZE - filePtr->pos;
	if (partition->readCluster(filePtr->dataCluster, clusterBuffer) == -1)
		return 0;
	memcpy(clusterBuffer + filePtr->pos, buffer, currClusterFree);
	if (partition->writeCluster(filePtr->dataCluster, clusterBuffer) == -1)
		return 0;

	unsigned bufferPtr = currClusterFree;

	while (bufferPtr >= CLUSTER_SIZE) {

		bufferPtr -= CLUSTER_SIZE;
	}



	return 0;
}

BytesCnt KernelFile::read(BytesCnt bytesCnt, char* buffer) {
	// same stuff as in write()
	return BytesCnt();
}

char KernelFile::seek(BytesCnt bytesCnt) {
	// same stuff as in write and read
	return 0;
}

BytesCnt KernelFile::filePos() {
	return 0;
}

char KernelFile::eof() {
	// position is maxed out, so is dataCluster ptr and lvl2Ptr
	return 0;
}

BytesCnt KernelFile::getFileSize() {
	// sum all lvl1 entry sizes
	// sum all of last lvl2 entry sizes
	// sum all of data size in last entry of last lvl2Index
	return BytesCnt();
}

char KernelFile::truncate() {
	partition->readCluster(lvl1IndexCluster, clusterBuffer);

	char additionalBuffer[CLUSTER_SIZE];

	ClusterNo *lvl1Ptr, *lvl2Ptr;
	for (ClusterNo i = 0; i < ENTRIES_PER_INDEX; i++) {
		lvl1Ptr = (ClusterNo*)clusterBuffer + i;

		if (*lvl1Ptr == 0)
			continue;

		partition->readCluster(*lvl1Ptr, additionalBuffer);

		for (ClusterNo j = 0; j < ENTRIES_PER_INDEX; j++) {
			lvl2Ptr = (ClusterNo*)additionalBuffer + j;

			if (*lvl2Ptr == 0)
				continue;

			KernelFS::deallocateCluster(*lvl2Ptr);
		}

		KernelFS::deallocateCluster(*lvl1Ptr);
		*lvl1Ptr = 0;
	}

	KernelFS::setLength(rootDirCluster, rootDirEntry, 0);

	return 1;
}
