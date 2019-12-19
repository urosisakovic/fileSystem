#include "kernelFile.h"

KernelFile::KernelFile(Partition *p, ClusterNo lvl1IndexCluster, ClusterNo lvl2IndexCluster, ClusterNo dataCluster) {
	this->part = p;
	
	this->lvl1IndexCluster = lvl1IndexCluster;
	this->lvl2IndexCluster = lvl2IndexCluster;
	this->dataCluster = dataCluster;
	this->position = 0;

	privateBuffer = new char[ClusterSize];
}

KernelFile::~KernelFile() {
	delete[] privateBuffer;
}

char KernelFile::write(BytesCnt bytesCnt, char* buffer) {
	int bytesInCurrentCluster = ClusterSize - position;
	int fullClusterCnt = (bytesCnt - bytesInCurrentCluster) / ClusterSize;
	int bytesInLastCluster = bytesCnt - bytesInCurrentCluster - fullClusterCnt * ClusterSize;

	int readPtr = 0;

	// write first cluster
	part->readCluster(dataCluster, privateBuffer);
	for (; readPtr < bytesInCurrentCluster; readPtr++)
		privateBuffer[position++] = buffer[readPtr];
	part->writeCluster(dataCluster, privateBuffer);

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
	// deallocate all dataClusters, lvl1IndexClusters and lvl2IndexClusters 
	// update corresponding entry in root dir
	return 0;
}
