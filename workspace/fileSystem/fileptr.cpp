#include "fileptr.h"

FilePointer::FilePointer(ClusterNo rootDirCluster, ClusterNo rootDirEntry) {
	this->rootDirCluster = rootDirCluster;
	this->rootDirEntry = rootDirEntry;

	this->lvl1IndexCluster = 0;
	this->lvl1IndexEntry = 0;
	//this->lvl1IndexSize = ...;

	this->lvl2IndexCluster = 0;
	this->lvl2IndexEntry = 0;
	//this->lvl2IndexSize = ...;

	this->dataCluster = 0;
	this->pos = 0;
}

char FilePointer::GoToNextCluster() {
	if (lvl2IndexEntry >= ENTRIES_PER_INDEX) {
		std::cout << "FilePointer::GoToNextCluster 1" << std::endl;
		exit(1);
	}
	if (lvl1IndexEntry >= ENTRIES_PER_INDEX) {
		std::cout << "FilePointer::GoToNextCluster 2" << std::endl;
		exit(1);
	}

	char* clusterBuffer = new char[CLUSTER_SIZE];

	if (lvl2IndexEntry < ENTRIES_PER_INDEX - 1) {
		lvl2IndexEntry++;

		if (ClusterAllocation::readCluster(lvl2IndexCluster, clusterBuffer) == -1)
			return 0;

		ClusterNo* dataClustPtr = (ClusterNo*)clusterBuffer + lvl2IndexEntry;
		dataCluster = *dataClustPtr;

		pos = 0;

		if (dataCluster == 0) {
			dataCluster = ClusterAllocation::allocateCluster();
			KernelFS::setDataCluster(lvl2IndexCluster, lvl2IndexEntry, dataCluster);
		}

		return 1;
	}

	// lvl2IndexEntry == ENTRIES_PER_INDEX - 1
	lvl1IndexEntry++;

	if (ClusterAllocation::readCluster(lvl1IndexCluster, clusterBuffer) == -1)
		return 0;

	ClusterNo* lvl2IndexPtr = (ClusterNo*)clusterBuffer + lvl1IndexEntry;
	lvl2IndexCluster = *lvl2IndexPtr;

	pos = 0;

	if (lvl2IndexCluster == 0) {
		lvl2IndexCluster = ClusterAllocation::allocateCluster();
		lvl2IndexEntry = 0;

		dataCluster = ClusterAllocation::allocateCluster();
		pos = 0;

		KernelFS::setLvl2Index(lvl1IndexCluster, lvl1IndexEntry, lvl2IndexCluster);
		KernelFS::setDataCluster(lvl2IndexCluster, lvl2IndexEntry, dataCluster);
	}

	return 1;
}

void FilePointer::ensureDataCluster() {
	if (lvl1IndexCluster != 0)
		return;
	
	lvl1IndexCluster = ClusterAllocation::allocateCluster();
	lvl1IndexEntry = 0;

	lvl2IndexCluster = ClusterAllocation::allocateCluster();
	lvl2IndexEntry = 0;

	dataCluster = ClusterAllocation::allocateCluster();
	pos = 0;

	KernelFS::setLvl1Index(rootDirCluster, rootDirEntry, lvl1IndexCluster);
	KernelFS::setLvl2Index(lvl1IndexCluster, lvl1IndexEntry, lvl2IndexCluster);
	KernelFS::setDataCluster(lvl2IndexCluster, lvl2IndexEntry, dataCluster);
}

BytesCnt FilePointer::byteOffset() {
	return lvl1IndexEntry * (1 << 22) +
		lvl2IndexEntry * (1 << 11) +
		pos;
}
