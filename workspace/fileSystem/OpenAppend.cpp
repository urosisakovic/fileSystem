#include "OpenAppend.h"

KernelFile* OpenAppend::open() {
	return nullptr;
}

OpenAppend::OpenAppend(char* fname, ClusterNo rootLvl1Index) :
	OpenFileStrategy(fname, rootLvl1Index) {}
