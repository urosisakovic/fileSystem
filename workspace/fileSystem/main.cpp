#include <iostream>
#include "kernelFS.h"
#include "part.h"


int main() {
	std::cout << "Program started executing!" << std::endl;

	Partition* p = new Partition((char *)"p1.ini");

	KernelFS* k = new KernelFS();

	std::cout << "Program finished executing!" << std::endl;
}