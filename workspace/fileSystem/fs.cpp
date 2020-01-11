#include "fs.h"

KernelFS* FS::myImpl = new KernelFS();

FS::~FS() {
	KernelFS::enterCriticSection();
	delete myImpl;
	KernelFS::exitCriticSection();
}

char FS::mount(Partition* partition) {
	KernelFS::enterCriticSection();
	char ret = KernelFS::mount(partition);
	KernelFS::exitCriticSection();

	return ret;
}

char FS::unmount() {
	KernelFS::enterCriticSection();
	char ret = KernelFS::unmount();
	KernelFS::exitCriticSection();

	return ret;
}

char FS::format() {
	KernelFS::enterCriticSection();
	char ret = KernelFS::format();
	KernelFS::exitCriticSection();

	return ret;
}

FileCnt FS::readRootDir() {
	KernelFS::enterCriticSection();
	FileCnt ret = KernelFS::readRootDir();
	KernelFS::exitCriticSection();

	return ret;
}

char FS::doesExist(char* fname) {
	KernelFS::enterCriticSection();
	char ret = KernelFS::doesExist(fname);
	KernelFS::exitCriticSection();

	return ret;
}

File* FS::open(char* fname, char mode) {
	KernelFS::aquireFile(fname);
	KernelFS::enterCriticSection();
	File* ret = KernelFS::open(fname, mode);
	if (ret == nullptr)
		KernelFS::releaseFile(fname);
	KernelFS::exitCriticSection();

	return ret;
}

char FS::deleteFile(char* fname) {
	KernelFS::enterCriticSection();
	char ret = KernelFS::deleteFile(fname);
	KernelFS::exitCriticSection();

	return ret;
}

FS::FS() {
}




