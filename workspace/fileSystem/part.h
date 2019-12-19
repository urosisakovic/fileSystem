#pragma once
#include "utils.h"

class PartitionImpl;

class Partition {
public: 
	Partition(char*);   

	virtual ClusterNo getNumOfClusters() const;    

	virtual int readCluster(ClusterNo, char* buffer);  
	virtual int writeCluster(ClusterNo, const char* buffer);

	virtual ~Partition(); 
private:  
	PartitionImpl* myImpl;
};