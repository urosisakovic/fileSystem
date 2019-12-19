#include <iostream>
#include "kernelFS.h"
#include "part.h"


int main() {
	std::cout << "Program started executing!" << std::endl;

	Partition* p = new Partition((char *)"p1.ini");
	std::cout << "Created Partition" << std::endl;

	KernelFS* k = new KernelFS();
	std::cout << "Created KernelFS" << std::endl;

	k->mount(p);
	std::cout << "Mounted partition" << std::endl;

	k->format();
	std::cout << "Formatted partition" << std::endl;

	int fileCnt = k->readRootDir();
	std::cout << "Counted files: " << fileCnt << std::endl;

	if (k->doesExist((char*)"uros.txt"))
		std::cout << "uros.txt does exist." << std::endl;
	else
		std::cout << "uros.txt does not exist" << std::endl;

	std::cout << "Program finished executing!" << std::endl;
}