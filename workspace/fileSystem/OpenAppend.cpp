#include "OpenAppend.h"

KernelFile* OpenAppend::open() {
	if (!KernelFS::doesExist(fname))
		return nullptr;

	ClusterNo lvl1IndexCluster, rootDirCluster, rootDirEntry;
	FileSystemUtils::getFileInfo(fname, &lvl1IndexCluster, &rootDirCluster, &rootDirEntry);

	KernelFile* kf = new KernelFile(rootDirCluster, rootDirEntry, true);
	kf->seek(kf->getFileSize() - 1);

	return kf;
}

OpenAppend::OpenAppend(char* fname, ClusterNo rootLvl1Index) :
	OpenFileStrategy(fname, rootLvl1Index) {}
