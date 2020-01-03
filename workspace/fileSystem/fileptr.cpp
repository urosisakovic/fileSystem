#include "fileptr.h"

FilePointer::FilePointer(ClusterNo rootDirCluster, ClusterNo rootDirEntry) {
	this->rootDirCluster = rootDirCluster;
	this->rootDirEntry = rootDirEntry;

	this->lvl1IndexCluster = FileSystemUtils::getLvl1Index(rootDirCluster, rootDirEntry);
	this->lvl1IndexEntry = 0;

	this->lvl2IndexEntry = 0;
	this->pos = 0;

	if (this->lvl1IndexCluster == 0) {
		this->lvl2IndexCluster = 0;
		this->dataCluster = 0;
	}
	else {
		this->lvl2IndexCluster = FileSystemUtils::getLvl2Index(this->lvl1IndexCluster, this->lvl1IndexEntry);
		this->dataCluster = FileSystemUtils::getDataCluster(this->lvl2IndexCluster, this->lvl2IndexEntry);
	}
	
}

char FilePointer::GoToNextCluster() {
	//std::cout << "lvl2IndexEntry: " << lvl2IndexEntry << std::endl;

	if (lvl2IndexEntry >= ENTRIES_PER_INDEX) {
		std::cout << "FilePointer::GoToNextCluster 1: " << lvl2IndexEntry << std::endl;
		exit(1);
	}
	if (lvl1IndexEntry >= ENTRIES_PER_INDEX) {
		std::cout << "FilePointer::GoToNextCluster 2" << std::endl;
		exit(1);
	}

	char* clusterBuffer = new char[CLUSTER_SIZE];

	if (lvl2IndexEntry < ENTRIES_PER_INDEX - 1) {
		lvl2IndexEntry++;

		if (ClusterAllocation::readCluster(lvl2IndexCluster, clusterBuffer) == 0) {
			std::cout << "error in FilePointer::GoToNextCluster 3" << std::endl;
			exit(1);
		}

		ClusterNo* dataClustPtr = (ClusterNo*)clusterBuffer + lvl2IndexEntry;
		dataCluster = *dataClustPtr;

		pos = 0;

		if (dataCluster == 0) {
			dataCluster = ClusterAllocation::allocateCluster();
			FileSystemUtils::setDataCluster(lvl2IndexCluster, lvl2IndexEntry, dataCluster);
		}

		return 1;
	}

	// lvl2IndexEntry == ENTRIES_PER_INDEX - 1
	lvl1IndexEntry++;

	if (ClusterAllocation::readCluster(lvl1IndexCluster, clusterBuffer) == 0) {
		std::cout << "error in FilePointer::GoToNextCluster 4" << std::endl;
		exit(1);
	}

	ClusterNo* lvl2IndexPtr = (ClusterNo*)clusterBuffer + lvl1IndexEntry;
	lvl2IndexCluster = *lvl2IndexPtr;
	

	if (lvl2IndexCluster == 0) {
		lvl2IndexCluster = ClusterAllocation::allocateCluster();
		lvl2IndexEntry = 0;

		dataCluster = ClusterAllocation::allocateCluster();
		pos = 0;

		FileSystemUtils::setLvl2Index(lvl1IndexCluster, lvl1IndexEntry, lvl2IndexCluster);
		FileSystemUtils::setDataCluster(lvl2IndexCluster, lvl2IndexEntry, dataCluster);
	}

	lvl2IndexEntry = 0;

	if (ClusterAllocation::readCluster(lvl2IndexCluster, clusterBuffer) == 0) {
		std::cout << "error in FilePointer::GoToNextCluster 4" << std::endl;
		exit(1);
	}

	ClusterNo* dataClustPtr = (ClusterNo*)clusterBuffer;
	dataCluster = *dataClustPtr;

	if (dataCluster == 0) {
		std::cout << "FilePointer::GoToNextCluster 3" << std::endl;
		exit(1);
	}

	pos = 0;

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

	FileSystemUtils::setLvl1Index(rootDirCluster, rootDirEntry, lvl1IndexCluster);
	FileSystemUtils::setLvl2Index(lvl1IndexCluster, lvl1IndexEntry, lvl2IndexCluster);
	FileSystemUtils::setDataCluster(lvl2IndexCluster, lvl2IndexEntry, dataCluster);
}

BytesCnt FilePointer::byteOffset() {
	return lvl1IndexEntry * (1 << 20) +
		lvl2IndexEntry * (1 << 11) +
		pos;
}
