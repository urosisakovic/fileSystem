#include "kernelFS.h"

char KernelFS::mount(Partition* partition) {
	mountSem->wait();
	p = partition;
}

char KernelFS::unmount() {
	allFilesClosed->wait();
	p = nullptr;
	mountSem->signal();
	allFilesClosed->signal();
}

char KernelFS::format() {
	allFilesClosed->wait();

	// set bit vector to all zeros
	char zeroBlock[ClusterSize];
	for (int i = 0; i < ClusterSize; zeroBlock[i++] = 0);

	// set root directory level 1 index to cluster 1
	// ...

	allFilesClosed->signal();
}

FileCnt KernelFS::readRootDir() {
	return FileCnt();
}

char KernelFS::doesExist(char* fname) {
	bool exists = false;
	// search directory index for it
	// ...

	return exists;
}

File* KernelFS::open(char* fname, char mode) {
	if (!doesExist(fname))
		return nullptr;
}

char KernelFS::deleteFile(char* fname) {
	return 0;
}
