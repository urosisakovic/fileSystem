#include "clusterAllocation.h"
#include <iostream>

Partition* ClusterAllocation::partition = nullptr;
char* ClusterAllocation::bitVector = nullptr;
unsigned ClusterAllocation::bitVectorByteSize = 0;
unsigned ClusterAllocation::clusterCount = 0;

int ClusterAllocation::readCluster(ClusterNo cluster, char* buffer) {
	return partition->readCluster(cluster, buffer);
}

int ClusterAllocation::writeCluster(ClusterNo cluster, const char* buffer) {
	return partition->writeCluster(cluster, buffer);
}

void ClusterAllocation::markAllocated(ClusterNo clusterId) {
	unsigned correspondingByte = clusterId / 8;
	unsigned correspondingBit = 7 - clusterId % 8;

	bitVector[correspondingByte] &= ~(1 << correspondingBit);
}

void ClusterAllocation::markDeallocated(ClusterNo clusterId) {
	unsigned correspondingByte = clusterId / 8;
	unsigned correspondingBit = 7 - clusterId % 8;

	bitVector[correspondingByte] |= (1 << correspondingBit);
}

char ClusterAllocation::checkAllocated(ClusterNo clusterId) {
	unsigned correspondingByte = clusterId / 8;
	unsigned correspondingBit = 7 - clusterId % 8;

	if (correspondingByte >= bitVectorByteSize)
		return -1;

	return ((bitVector[correspondingByte] & (1 << correspondingBit)) == 0);
}

void ClusterAllocation::setPartition(Partition* p) {
	partition = p;
	clusterCount = p->getNumOfClusters();
}

void ClusterAllocation::setBitVector(unsigned bVSize, char* bV) {
	if (bitVector != nullptr)
		delete[] bitVector;

	bitVector = new char[bitVectorByteSize = bVSize];
	memcpy(bitVector, bV, bVSize);
}

ClusterNo ClusterAllocation::allocateCluster() {
	char clusterBuffer[CLUSTER_SIZE];

	for (ClusterNo i = 1; i < clusterCount; i++) {
		if (!checkAllocated(i)) {
			markAllocated(i);
			std::cout << "Allocated cluster: " << i << std::endl;
			memset(clusterBuffer, 0, CLUSTER_SIZE);
			writeCluster(i, clusterBuffer);

			return i;
		}
	}

	return 0;
}

char ClusterAllocation::deallocateCluster(ClusterNo freeClusterIdx) {
	if (checkAllocated(freeClusterIdx))
		return 0;

	markDeallocated(freeClusterIdx);
}