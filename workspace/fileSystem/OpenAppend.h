#pragma once
#include "OpenFileStrategy.h"
#include "kernelFile.h"

class KernelFile;

class OpenAppend :
	public OpenFileStrategy {
public:
	KernelFile* open();
	OpenAppend(char*, ClusterNo);
};

