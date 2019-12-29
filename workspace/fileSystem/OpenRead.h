#pragma once
#include "OpenFileStrategy.h"
#include "kernelFile.h"

class KernelFile;

class OpenRead :
	public OpenFileStrategy {
public:
	KernelFile* open();
	OpenRead(char*, ClusterNo);
};

