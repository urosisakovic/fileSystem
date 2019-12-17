#pragma once

class FCB;

typedef unsigned BytesCnt;

class KernelFile{
public:
	~KernelFile(); //zatvaranje fajla   
	char write(BytesCnt, char* buffer);
	BytesCnt read(BytesCnt, char* buffer);
	char seek(BytesCnt);
	BytesCnt filePos();
	char eof();
	BytesCnt getFileSize();
	char truncate();
private:
	FCB *fcb;
};

