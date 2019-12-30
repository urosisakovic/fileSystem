#include "FileSystemUtils.h"

char* FileSystemUtils::clusterBuffer = new char[CLUSTER_SIZE];

char FileSystemUtils::setLvl2Index(ClusterNo lvl1Index, ClusterNo lvl1IndexEntry, ClusterNo lvl2Index) {
	if (ClusterAllocation::readCluster(lvl1Index, clusterBuffer) == -1)
		return 0;

	ClusterNo* entry = (ClusterNo*)clusterBuffer + lvl1IndexEntry;

	*entry = lvl2Index;

	if (ClusterAllocation::writeCluster(lvl1Index, clusterBuffer) == -1)
		return 0;

	return 1;
}

char FileSystemUtils::setDataCluster(ClusterNo lvl2Index, ClusterNo lvl2IndexEntry, ClusterNo dataCluster) {
	if (ClusterAllocation::readCluster(lvl2Index, clusterBuffer) == -1)
		return 0;

	ClusterNo* entry = (ClusterNo*)clusterBuffer + lvl2IndexEntry;

	*entry = dataCluster;

	if (ClusterAllocation::writeCluster(lvl2Index, clusterBuffer) == -1)
		return 0;

	return 1;
}

BytesCnt FileSystemUtils::readLength(ClusterNo rootDirCluster, ClusterNo rootEntry) {
	if (ClusterAllocation::readCluster(rootDirCluster, clusterBuffer) == -1)
		return 0;

	rootDirEntry* entry = (rootDirEntry*)clusterBuffer + rootEntry;

	BytesCnt* length = (BytesCnt*)entry + 16;

	return *length;
}

char FileSystemUtils::setLength(ClusterNo rootDirCluster, ClusterNo rootEntry, unsigned size) {
	if (ClusterAllocation::readCluster(rootDirCluster, clusterBuffer) == -1)
		return 0;

	rootDirEntry* entry = (rootDirEntry*)clusterBuffer + rootEntry;

	unsigned* length = (unsigned*)entry + 16;
	*length = size;

	if (ClusterAllocation::writeCluster(rootDirCluster, clusterBuffer) == -1)
		return 0;

	return 1;
}

char FileSystemUtils::setLvl1Index(ClusterNo rootDirCluster, ClusterNo rootEntry, ClusterNo lvl1Index) {
	if (ClusterAllocation::readCluster(rootDirCluster, clusterBuffer) == -1)
		return 0;

	rootDirEntry* entry = (rootDirEntry*)clusterBuffer + rootEntry;

	ClusterNo* lvl1 = (ClusterNo*)entry + 12;
	*lvl1 = lvl1Index;

	if (ClusterAllocation::writeCluster(rootDirCluster, clusterBuffer) == -1)
		return 0;

	return 1;
}