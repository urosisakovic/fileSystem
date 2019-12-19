#include "kernelFile.h"

KernelFile::KernelFile(Partition *p, ClusterNo rootDirCluster, ClusterNo rootDirEntry, ClusterNo lvl1IndexCluster, ClusterNo lvl2IndexCluster, ClusterNo dataCluster) {
	this->partition = p;
	this->rootDirCluster = rootDirCluster;

	this->rootDirEntry = rootDirEntry;
	this->lvl1IndexCluster = lvl1IndexCluster;
	this->lvl2IndexCluster = lvl2IndexCluster;
	this->dataCluster = dataCluster;
	this->position = 0;

	clusterBuffer = new char[CLUSTER_SIZE];
}

KernelFile::~KernelFile() {
	delete[] clusterBuffer;
}

char KernelFile::write(BytesCnt bytesCnt, char* buffer) {
	int bytesInCurrentCluster = CLUSTER_SIZE - position;
	int fullClusterCnt = (bytesCnt - bytesInCurrentCluster) / CLUSTER_SIZE;
	int bytesInLastCluster = bytesCnt - bytesInCurrentCluster - fullClusterCnt * CLUSTER_SIZE;

	int readPtr = 0;

	// write first cluster
	partition->readCluster(dataCluster, clusterBuffer);
	for (; readPtr < bytesInCurrentCluster; readPtr++)
		clusterBuffer[position++] = buffer[readPtr];
	partition->writeCluster(dataCluster, clusterBuffer);

	// write full clusters
	for (int i = 0; i < fullClusterCnt; i++) {
		// update lvl2Index, dataCluster and position
		// write to the following cluster
	}

	// write last cluster
	// update lvl2Index, dataCluster and position
	// read last cluster
	// alter that data
	// write it back

	// take care of allocating more memory!!!

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
	return position;
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
}
