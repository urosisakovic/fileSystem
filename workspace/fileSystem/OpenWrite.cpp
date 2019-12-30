#include "OpenWrite.h"

OpenWrite::OpenWrite(char* fname, ClusterNo rootLvl1Index) :
	OpenFileStrategy(fname, rootLvl1Index) {}


KernelFile* OpenWrite::open() {
	if (KernelFS::doesExist(fname))
		KernelFS::deleteFile(fname);
	
	return OpenFileStrategy::open();
}
