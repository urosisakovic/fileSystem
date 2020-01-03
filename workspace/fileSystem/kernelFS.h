#pragma once
#include <string.h>
#include <unordered_map>
#include<cstdlib>
#include<windows.h>
#include<ctime>
#include <string>
#include <iostream>
#include "part.h"
#include "utils.h"
#include "file.h"
#include "kernelFile.h"
#include "clusterAllocation.h"
#include "fileptr.h"

#define signal(x) ReleaseSemaphore(x,1,NULL)
#define wait(x) WaitForSingleObject(x,INFINITE)

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
    
    static File* open(char* fname, char mode);

    static char close(char* fname);

    static char deleteFile(char* fname);

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

    static std::unordered_map<std::string, File*> *openFiles;

    static OpenFileStrategy* openFile;

    static HANDLE mountSem;
    static HANDLE unmountSem;
    static HANDLE mutex;
    static HANDLE openCriticSection;
    static std::unordered_map<std::string, HANDLE>* fileLocksWrite;
    static std::unordered_map<std::string, HANDLE>* fileLocksRead;

    friend class FileSystemUtils;
};

