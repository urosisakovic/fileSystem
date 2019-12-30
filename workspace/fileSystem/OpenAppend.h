#pragma once
#include "OpenFileStrategy.h"

class OpenAppend :
	public OpenFileStrategy {
public:
	KernelFile* open() override;
	OpenAppend(char*, ClusterNo);
};

