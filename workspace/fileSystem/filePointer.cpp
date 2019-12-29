#include "filePointer.h"

FilePointer::FilePointer(ClusterNo rootDirCluster, ClusterNo rootDirEntry) {
	this->rootDirCluster = rootDirCluster;
	this->rootDirEntry = rootDirEntry;

	this->lvl1IndexCluster = 0;
	this->lvl1IndexEntry = 0;

	this->lvl2IndexCluster = 0;
	this->lvl2IndexEntry = 0;

	this->dataCluster = 0;
	this->pos = 0;
}

char FilePointer::GoToNextCluster() {
	return 0;
}

void FilePointer::ensureDataCluster() {
	if (lvl1IndexCluster != 0)
		return;
	
	lvl1IndexCluster = KernelFS::allocateCluster();
	lvl1IndexEntry = 0;

	lvl2IndexCluster = KernelFS::allocateCluster();
	lvl2IndexEntry = 0;

	dataCluster = KernelFS::allocateCluster();
	pos = 0;

	KernelFS::setLvl1Index(rootDirCluster, rootDirEntry, lvl1IndexCluster);
	KernelFS::setLvl2Index(lvl1IndexCluster, lvl1IndexEntry, lvl2IndexCluster);
	KernelFS::setDataCluster(lvl2IndexCluster, lvl2IndexEntry, dataCluster);
}
