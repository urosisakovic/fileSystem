#include "OpenRead.h"

KernelFile* OpenRead::open() {
	if (KernelFS::doesExist(fname))
		return nullptr;
	
	KernelFile* kf = OpenFileStrategy::open();
	kf->disableWriting();

	return kf;
}

OpenRead::OpenRead(char* fname, ClusterNo rootLvl1Index) :
	OpenFileStrategy(fname, rootLvl1Index) {}
