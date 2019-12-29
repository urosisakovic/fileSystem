#pragma once
#include "utils.h"
#include "part.h"
#include <cstring>
#include <iostream>

class ClusterAllocation {
public:
    static int readCluster(ClusterNo, char* buffer);
    static int writeCluster(ClusterNo, const char* buffer);
    // returns number of allocated cluster in case of success
    // otherwise, returns 0 (cannot be allocated since bit vector is there)
    static ClusterNo allocateCluster();
    // returns 1 in case of success, 0 otherwise
    static char deallocateCluster(ClusterNo);
    // sets corresponding bit in bit vector
    static void markAllocated(ClusterNo);
    // clears corresponding bit in bit vector
    static void markDeallocated(ClusterNo);
    // returns 1 if cluster is allocated, 0 if it is free and -1 in case of an error
    static char checkAllocated(ClusterNo);

    static void setPartition(Partition*);
    static void setBitVector(unsigned, char*);
private:
    static Partition* partition;
    static char* bitVector;
    static unsigned bitVectorByteSize;
    static unsigned clusterCount;
};

