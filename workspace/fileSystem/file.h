#pragma once
#include "kernelFile.h"
#include "kernelFS.h"

class KernelFile;

class File {
public:   
	~File(); //zatvaranje fajla   
	char write (BytesCnt, char* buffer);    
	BytesCnt read (BytesCnt, char* buffer);   
	char seek (BytesCnt);   
	BytesCnt filePos();   
	char eof ();   
	BytesCnt getFileSize ();   
	char truncate (); 
private: 
	KernelFile* myImpl;
	//objekat fajla se može kreirati samo otvaranjem
	File();

	friend class FS;
	friend class KernelFS;
};