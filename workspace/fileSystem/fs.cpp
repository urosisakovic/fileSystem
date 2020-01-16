#include "fs.h"

KernelFS* FS::myImpl = new KernelFS();

FS::~FS() {
	KernelFS::enterCS();
	delete myImpl;
	KernelFS::exitCS();
}

char FS::mount(Partition* partition) {
	KernelFS::enterCS();
	char ret = KernelFS::mount(partition);
	KernelFS::exitCS();

	return ret;
}

char FS::unmount() {
	KernelFS::enterCS();
	char ret = KernelFS::unmount();
	KernelFS::exitCS();

	return ret;
}

char FS::format() {
	KernelFS::enterCS();
	char ret = KernelFS::format();
	KernelFS::exitCS();

	return ret;
}


FileCnt FS::readRootDir() {
	KernelFS::enterCS();
	FileCnt ret = KernelFS::readRootDir();
	KernelFS::exitCS();

	return ret;
}

char FS::doesExist(char* fname) {
	KernelFS::enterCS();
	char ret = KernelFS::doesExist(fname);
	KernelFS::exitCS();

	return ret;
}

File* FS::open(char* fname, char mode) {
	KernelFS::aquireFile(fname);

	KernelFS::enterCS();
	File *ret = KernelFS::open(fname, mode);
	if (ret == nullptr)
		KernelFS::releaseFile(fname);
	KernelFS::exitCS();

	return ret;
}

char FS::deleteFile(char* fname) {
	KernelFS::enterCS();
	char ret = KernelFS::deleteFile(fname);
	KernelFS::exitCS();

	return ret;
}

FS::FS() {
}




