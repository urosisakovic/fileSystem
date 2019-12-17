#pragma once
#include "sem.h"
#include "part.h"

class File;

typedef long FileCnt;
typedef unsigned long BytesCnt;
const unsigned int FNAMELEN = 8;
const unsigned int FEXTLEN = 3;

class KernelFS {
public:
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
    static Partition* p;
};