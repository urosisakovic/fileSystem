#pragma once
#include "OpenFileStrategy.h"

class OpenRead :
	public OpenFileStrategy {
public:
	KernelFile* open() override;
	OpenRead(char*, ClusterNo);
};

