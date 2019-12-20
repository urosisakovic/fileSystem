#pragma once
#include <string.h>
#include <unordered_map>
#include <string>
#include "sem.h"
#include "part.h"
#include "utils.h"
#include "file.h"

class File;

class KernelFS {
public:
    // allocate space for clusterBuffer
    KernelFS(); 

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

    // deallocate space for clusterBuffer
    ~KernelFS();

    // returns number of allocated cluster in case of success
    // otherwise, returns 0 (cannot be allocated since bit vector is there)
    static ClusterNo allocateCluster();

    // returns 1 in case of success, 0 otherwise
    static char deallocateCluster(ClusterNo);

    static char setLength(ClusterNo, ClusterNo, unsigned);
private:
    // pointer to a Partition object which abstracts
    // Windows 10 x64 API towards hard disk
    static Partition* partition;

    // number of clusters in partition
    // equal to partition.getNumOfClusters()
    static ClusterNo clusterCount;

    static char *bitVector;
    // byte size of bit vector
    static int bitVectorByteSize;
    // cluster size of bit vector
    static ClusterNo bitVectorClusterSize;

    // sets corresponding bit in bit vector
    static void markAllocated(ClusterNo);
    // clears corresponding bit in bit vector
    static void markDeallocated(ClusterNo);
    // returns 1 if cluster is allocated, 0 if it is free and -1 in case of an error
    static char checkAllocated(ClusterNo);

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

    static ClusterNo allocateAndSetDataCluster(char*, char*);
    static ClusterNo allocateAndSetLvl2Cluster(ClusterNo);
    static char addEntryToDataDir(ClusterNo, char*, char*);
    static void splitFileName(char*, char**, char**);
};

