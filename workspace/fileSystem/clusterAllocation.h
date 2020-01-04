#pragma once
#include "utils.h"
#include "part.h"
#include <cstring>
#include <iostream>
#include <unordered_map>
#include<cstdlib>
#include<windows.h>
#include<ctime>

#define signal(x) ReleaseSemaphore(x,1,NULL)
#define wait(x) WaitForSingleObject(x,INFINITE)

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

    static void setPartition(Partition*);   // TODO: remove all cached data
    static void setBitVector(unsigned, char*);

    static void updateCache(ClusterNo, bool);

    static long freeClustersCount();
private:
    class CacheNode {
    public:
        char* cachedCluster;
        CacheNode *next, *prev;
        ClusterNo cluster;
        bool changed;

        CacheNode(ClusterNo cl) :
            cluster(cl), next(nullptr), prev(nullptr), changed(false) {
            cachedCluster = new char[CLUSTER_SIZE];
        }

        ~CacheNode() {
            delete[] cachedCluster;
        }
    };

    static CacheNode* head, *tail;
    static std::unordered_map<ClusterNo, CacheNode*> *cachedMap;
    static unsigned cacheSize;
    static unsigned maxCacheSize;

    static Partition* partition;
    static char* bitVector;
    static unsigned bitVectorByteSize;
    static unsigned clusterCount;

    static HANDLE allocationMutex;
    static HANDLE clusterMutex;
};

