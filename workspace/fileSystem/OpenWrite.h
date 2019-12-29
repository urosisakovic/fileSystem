#pragma once
#include "OpenFileStrategy.h"
#include "kernelFile.h"

class KernelFile;

class OpenWrite :
	public OpenFileStrategy {
public:
	KernelFile* open();
	OpenWrite(char*, ClusterNo);
};

