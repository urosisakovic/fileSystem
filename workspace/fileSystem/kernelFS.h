#pragma once
#include <string.h>
#include <unordered_map>
#include <string>
#include <iostream>
#include "sem.h"
#include "part.h"
#include "utils.h"
#include "file.h"
#include "kernelFile.h"
#include "clusterAllocation.h"
#include "OpenAppend.h"
#include "OpenRead.h"
#include "OpenWrite.h"

class File;
class KernelFile;
class OpenFileStrategy;

class KernelFS {
public:
    // returns 1 for success and 0 for failure
    static char mount(Partition* partition); 
    // returns 1 for success and 0 for failure
    static char unmount();  
    // returns 1 for success and 0 for failure
    static char format(); 

    // returns number of files for success and -1 for failure
    static FileCnt readRootDir();
    // fname is aboslute filepath of a file
    // returns 1 if such a file exists, 0 otherwise
    static char doesExist(char* fname); 
    
    // description
    static File* open(char* fname, char mode);
    // description
    static char deleteFile(char* fname);

    static char setLength(ClusterNo, ClusterNo, unsigned);
    static char setLvl1Index(ClusterNo, ClusterNo, ClusterNo);
    static char setLvl2Index(ClusterNo, ClusterNo, ClusterNo);
    static char setDataCluster(ClusterNo, ClusterNo, ClusterNo);
    static BytesCnt readLength(ClusterNo, ClusterNo);
private:
    // pointer to a Partition object which abstracts
    // Windows 10 x64 API towards hard disk
    static Partition* partition;

    // number of clusters in partition
    // equal to partition.getNumOfClusters()
    static ClusterNo clusterCount;

    static char *bitVector;
    // byte size of bit vector
    static unsigned bitVectorByteSize;
    // cluster size of bit vector
    static ClusterNo bitVectorClusterSize;

    // number of cluster containing level 1 index of root directory
    // equal to bitVectorClusterCount
    static ClusterNo rootDirLvl1Index;
    
    // buffer with the size equal to one cluster
    // used to read a cluster into it or write a cluster from it
    static char* clusterBuffer;
    
    // blocks all thread which try to mount a partition while there is
    // already one mounted
    static Semaphore* mountSem;
    // blocks unmounting and formatting if there are open files
    static Semaphore* allFilesClosed;

    static std::unordered_map<std::string, File*> openFiles;

    static OpenFileStrategy* openFile;
};

