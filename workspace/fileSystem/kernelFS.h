#pragma once
#include <string.h>
#include <unordered_map>
#include <string>
#include <iostream>
#include <vector>
#include "part.h"
#include "utils.h"
#include "file.h"
#include "kernelFile.h"
#include "clusterAllocation.h"
#include "fileptr.h"

class File;
class KernelFile;

class KernelFS {
public:
    static char mount(Partition* partition);
    static char unmount();
    static char format();

    static FileCnt readRootDir();
    static char doesExist(char* fname); 

    static KernelFile* getFile(char* fname);
    static KernelFile* createFile(char* fname);
    static KernelFile* openRead(char* fname);
    static KernelFile* openWrite(char* fname);
    static KernelFile* openAppend(char* fname);
    static File* open(char* fname, char mode);
    
    static char close(char* fname);     
    static char deleteFile(char* fname);    // not finished
private:
    class DataClusterNode {
    public:
        char* smallestKey;
        ClusterNo clusterIdx;
        ClusterNo lvl2IndexEntry;
        unsigned freeEntryCnt;

        DataClusterNode(ClusterNo clusterIdx, ClusterNo lvl1IndexEntry) {
            this->clusterIdx = clusterIdx;
            this->lvl2IndexEntry = lvl1IndexEntry;

            this->smallestKey = nullptr;
            this->freeEntryCnt = ENTRIES_PER_INDEX * ENTRIES_PER_ROOT_DIR;
        }
    };

    class Lvl2IndexNode {
    public:
        char* smallestKey;
        ClusterNo clusterIdx;
        ClusterNo lvl1IndexEntry;
        unsigned freeEntryCnt;
        std::vector<DataClusterNode>* lvl1Nodes;

        Lvl2IndexNode() : Lvl2IndexNode(0, 0) {}

        Lvl2IndexNode(ClusterNo clusterIdx, ClusterNo lvl1IndexEntry) {
            this->clusterIdx = clusterIdx;
            this->lvl1IndexEntry = lvl1IndexEntry;

            this->smallestKey = nullptr;
            this->freeEntryCnt = ENTRIES_PER_INDEX * ENTRIES_PER_ROOT_DIR;
            this->lvl1Nodes = new std::vector<DataClusterNode>();
        }

        ~Lvl2IndexNode() {
            delete lvl1Nodes;
        }

        bool unpopulated() {
            return clusterIdx == 0;
        }

        FileCnt countEntries() {
            return lvl1Nodes->size();
        }
    };

    static Partition* partition;

    static ClusterNo rootDirLvl1Index;
    static char lvl1Index[CLUSTER_SIZE];
    static Lvl2IndexNode lvl2IndexNodes[ENTRIES_PER_INDEX];

    static ClusterNo clusterCount;
    static char *bitVector;
    static unsigned bitVectorByteSize;
    static ClusterNo bitVectorClusterSize;
    
    static char clusterBuffer[CLUSTER_SIZE];

    static std::unordered_map<std::string, File*> *openFiles;


    friend class FileSystemUtils;
};

