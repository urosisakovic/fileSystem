#include <iostream>
#include "kernelFS.h"
#include "part.h"
#include "clusterAllocation.h"

void check(char *a, char *b, int len) {
	for (int i = 0; i < len; i++)
		if (a[i] != b[i]) {
			std::cout << "NE RADI: " << i << "  " << int(a[i]) << "   " << int(b[i]) << std::endl;
			return;
		}

	std::cout << "RADI" << std::endl;
}

void testWriteRead() {
	std::cout << "testWriteRead started executing!" << std::endl;

	Partition* p = new Partition((char*)"p2.ini");
	std::cout << "Created Partition" << std::endl;

	KernelFS* k = new KernelFS();
	std::cout << "Created KernelFS" << std::endl;

	k->mount(p);
	std::cout << "Mounted partition" << std::endl;

	k->format();
	std::cout << "Formatted partition" << std::endl;

	File *urosFile = k->open((char*)"uros.txt", 'w');

	const int LEN = 5 * int(1e6);

	char* wb = new char[LEN];
	for (int i = 0; i < LEN; wb[i++] = i);

	urosFile->write(LEN, wb);

	urosFile->seek(0);

	char* rb = new char[LEN];
	urosFile->read(LEN, rb);
	
	check(wb, rb, LEN);

	for (int i = 0; i < 20; i++)
		std::cout << int(rb[i]) << " ";
}

void testFileCreationAndDeletion() {
	std::cout << "testFileCreationAndDeletion started executing!" << std::endl;

	Partition* p = new Partition((char*)"p2.ini");
	std::cout << "Created Partition" << std::endl;

	KernelFS* k = new KernelFS();
	std::cout << "Created KernelFS" << std::endl;

	k->mount(p);
	std::cout << "Mounted partition" << std::endl;

	k->format();
	std::cout << "Formatted partition" << std::endl;

	const int LEN = 1000;

	char filepath[12] = { 'u', 'r', 'o', 's', '0', '0', '0', '.', 't', 'x', 't', '\0' };
	for (int i = 0; i < LEN; i++) {
		int s = i / 100;
		int d = (i - s * 100) / 10;
		int j = i % 10;

		char ss = '0' + s;
		char dd = '0' + d;
		char jj = '0' + j;

		filepath[4] = ss;
		filepath[5] = dd;
		filepath[6] = jj;

		if (KernelFS::open(filepath, 'w') == 0) {
			std::cout << "error creating file." << std::endl;
			exit(1);
		}

		std::cout << KernelFS::readRootDir() << std::endl;
	}

	for (int i = 0; i < LEN; i++) {
		int s = i / 100;
		int d = (i - s * 100) / 10;
		int j = i % 10;

		char ss = '0' + s;
		char dd = '0' + d;
		char jj = '0' + j;

		filepath[4] = ss;
		filepath[5] = dd;
		filepath[6] = jj;

		KernelFS::close(filepath);

		if (KernelFS::deleteFile(filepath) == 0)
			std::cout << "error deleting file. " << i << std::endl;

		std::cout << KernelFS::readRootDir() << std::endl;
	}
}

void testDeletion() {
	std::cout << "testDeletion started executing!" << std::endl;

	Partition* p = new Partition((char*)"p2.ini");
	std::cout << "Created Partition" << std::endl;

	KernelFS* k = new KernelFS();
	std::cout << "Created KernelFS" << std::endl;

	k->mount(p);
	std::cout << "Mounted partition" << std::endl;

	k->format();
	std::cout << "Formatted partition" << std::endl;

	File* urosFile = k->open((char*)"uros.txt", 'w');

	std::cout << "Free clusters pre-write: " << ClusterAllocation::freeClustersCount() << std::endl;

	const int LEN = 5 * int(1e6);

	char* wb = new char[LEN];
	for (int i = 0; i < LEN; wb[i++] = i);

	urosFile->write(LEN, wb);

	std::cout << "Free clusters post-write pre-deletion: " << ClusterAllocation::freeClustersCount() << std::endl;

	KernelFS::close((char*)"uros.txt");

	if (KernelFS::deleteFile((char*)"uros.txt") == 0)
		std::cout << "error while deleting" << std::endl;

	std::cout << "Free clusters post-write post-deletion: " << ClusterAllocation::freeClustersCount() << std::endl;
}


void testTruncation() {
	std::cout << "testTruncation started executing!" << std::endl;

	Partition* p = new Partition((char*)"p2.ini");
	std::cout << "Created Partition" << std::endl;

	KernelFS* k = new KernelFS();
	std::cout << "Created KernelFS" << std::endl;

	k->mount(p);
	std::cout << "Mounted partition" << std::endl;

	k->format();
	std::cout << "Formatted partition" << std::endl;

	File* urosFile = k->open((char*)"uros.txt", 'w');

	std::cout << "Free clusters pre-write: " << ClusterAllocation::freeClustersCount() << std::endl;

	const int LEN = 1 * int(1e6);

	char* wb = new char[LEN];
	for (int i = 0; i < LEN; wb[i++] = i);

	urosFile->write(LEN, wb);

	urosFile->seek(1000);

	urosFile->truncate();

	std::cout << urosFile->getFileSize() << std::endl;
}


int main() {
	// testWriteRead();
	// testFileCreationAndDeletion();
	// testDeletion();
	testTruncation();
}