#pragma once
#include <string.h>
#include "sem.h"
#include "part.h"
#include "utils.h"

class File;

class KernelFS {
public:
    KernelFS();
    ~KernelFS();

    static char mount(Partition* partition); //montira particiju   
                // vraca 0 u slucaju neuspeha ili 1 u slucaju uspeha   
    static char unmount();  //demontira particiju
                // vraca 0 u slucaju neuspeha ili 1 u slucaju uspeha
    static char format(); //formatira particiju;               
                // vraca 0 u slucaju neuspeha ili 1 u slucaju uspeha 

    static FileCnt readRootDir();
    // vraca -1 u slucaju neuspeha ili broj fajlova u slucaju uspeha  

    static char doesExist(char* fname); //argument je naziv fajla sa                                        
                                        //apsolutnom putanjom    

    static File* open(char* fname, char mode);
    static char deleteFile(char* fname);
private:
    static Semaphore* mountSem;
    static Semaphore* allFilesClosed;
    static Partition* partition;
    static char *bitVector;
    static int bitVectorSize;
    static ClusterNo rootDirLvl1Index;
    static ClusterNo clusterCount;
    static char* clusterBuffer;
    static ClusterNo bitVectorClusterCount;
};