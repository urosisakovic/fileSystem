#include "OpenAppend.h"

KernelFile* OpenAppend::open() {
	if (KernelFS::doesExist(fname))
		return nullptr;

	KernelFile *kf = OpenFileStrategy::open();
	kf->seek(kf->getFileSize() - 1);

	return kf;
}

OpenAppend::OpenAppend(char* fname, ClusterNo rootLvl1Index) :
	OpenFileStrategy(fname, rootLvl1Index) {}
