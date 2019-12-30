#pragma once
#include "OpenFileStrategy.h"

class OpenWrite :
	public OpenFileStrategy {
public:
	KernelFile* open() override;
	OpenWrite(char*, ClusterNo);
};

