#pragma once
#include "utils.h"
#include "clusterAllocation.h"
#include "kernelFS.h"

class FileSystemUtils {
public:
    static char setLength(ClusterNo, ClusterNo, unsigned);
    static char setLvl1Index(ClusterNo, ClusterNo, ClusterNo);
    static char setLvl2Index(ClusterNo, ClusterNo, ClusterNo);
    static char setDataCluster(ClusterNo, ClusterNo, ClusterNo);
    static BytesCnt readLength(ClusterNo, ClusterNo);
    static char emptyRootDirEntry(ClusterNo, ClusterNo);
    static void splitFileName(char*, char**, char**);
    static void getFileInfo(char*, ClusterNo*, ClusterNo*, ClusterNo*);

    static ClusterNo getLvl1Index(ClusterNo, ClusterNo);
    static ClusterNo getLvl2Index(ClusterNo, ClusterNo);
    static ClusterNo getDataCluster(ClusterNo, ClusterNo);
private:
    static char* clusterBuffer;
};

