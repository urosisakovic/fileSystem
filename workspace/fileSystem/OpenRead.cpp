#include "OpenRead.h"

KernelFile* OpenRead::open() {
	if (!KernelFS::doesExist(fname))
		return nullptr;
	
	ClusterNo lvl1IndexCluster, rootDirCluster, rootDirEntry;
	FileSystemUtils::getFileInfo(fname, &lvl1IndexCluster, &rootDirCluster, &rootDirEntry);

	KernelFile *kf = new KernelFile(rootDirCluster, rootDirEntry, false);

	return kf;
}

OpenRead::OpenRead(char* fname, ClusterNo rootLvl1Index) :
	OpenFileStrategy(fname, rootLvl1Index) {}
