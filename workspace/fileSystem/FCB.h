#pragma once

class File;

class FCB {
public:
	FCB();
	~FCB();
		
	static void add(FCB*);
	static void remove(FCB*);

private:
	int pos;
	FCB *next, *prev;
	static FCB *head, *tail;
	File *file;
};

