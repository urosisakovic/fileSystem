#pragma once
#include "utils.h"
#include "clusterAllocation.h"

class FileSystemUtils {
public:
    static char setLength(ClusterNo, ClusterNo, unsigned);
    static char setLvl1Index(ClusterNo, ClusterNo, ClusterNo);
    static char setLvl2Index(ClusterNo, ClusterNo, ClusterNo);
    static char setDataCluster(ClusterNo, ClusterNo, ClusterNo);
    static BytesCnt readLength(ClusterNo, ClusterNo);
    static char emptyRootDirEntry(ClusterNo, ClusterNo);
private:
    static char* clusterBuffer;
};

