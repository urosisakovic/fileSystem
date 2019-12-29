#include "OpenRead.h"

KernelFile* OpenRead::open() {
	return nullptr;
}

OpenRead::OpenRead(char* fname, ClusterNo rootLvl1Index) :
	OpenFileStrategy(fname, rootLvl1Index) {}
