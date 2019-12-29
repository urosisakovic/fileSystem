#include <iostream>
#include "kernelFS.h"
#include "part.h"

void test1() {
	std::cout << "Test1 started executing!" << std::endl;

	Partition* p = new Partition((char*)"p1.ini");
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

	k->open((char*)"uros.txt", 'w');
	k->open((char*)"viki.txt", 'w');
	k->open((char*)"mama.txt", 'w');
	k->open((char*)"tata.txt", 'w');
	std::cout << "Counted files: " << k->readRootDir() << std::endl;

	if (k->doesExist((char*)"uros.txt"))
		std::cout << "uros.txt does exist." << std::endl;
	else
		std::cout << "uros.txt does not exist" << std::endl;

	if (k->doesExist((char*)"viki.txt"))
		std::cout << "viki.txt does exist." << std::endl;
	else
		std::cout << "viki.txt does not exist" << std::endl;

	if (k->doesExist((char*)"mama.txt"))
		std::cout << "mama.txt does exist." << std::endl;
	else
		std::cout << "mama.txt does not exist" << std::endl;

	if (k->doesExist((char*)"tata.txt"))
		std::cout << "tata.txt does exist." << std::endl;
	else
		std::cout << "tata.txt does not exist" << std::endl;

	std::cout << "Program finished executing!" << std::endl;

	if (k->doesExist((char*)"lala.txt"))
		std::cout << "lala.txt does exist." << std::endl;
	else
		std::cout << "lala.txt does not exist" << std::endl;

	std::cout << "Program finished executing!" << std::endl;
}


int main() {
	test1();
}