#include"fs.h"
#include"file.h"
#include"part.h"
#include<iostream>
#include<fstream>
#include<cstdlib>
#include<windows.h>
#include<ctime>

#define signal(x) ReleaseSemaphore(x,1,NULL)
#define wait(x) WaitForSingleObject(x,INFINITE)

using namespace std;

HANDLE myMutex = CreateSemaphore(NULL, 1, 32, NULL);
HANDLE mySem12 = CreateSemaphore(NULL, 0, 32, NULL);

HANDLE cekajNit1 = CreateSemaphore(NULL, 0, 32, NULL);
HANDLE cekajNit2 = CreateSemaphore(NULL, 0, 32, NULL);

static Partition* partition;

DWORD WINAPI mojaNit1() {
	wait(myMutex);
	partition = new Partition((char*)"p1.ini");
	signal(myMutex);

	wait(myMutex);
	cout << ": Kreirana particija" << endl;
	signal(myMutex);

	FS::mount(partition);
	wait(myMutex);
	cout << ": Montirana particija" << endl;
	signal(myMutex);

	FS::format();
	wait(myMutex);
	cout << ": Formatirana particija" << endl;
	signal(myMutex);

	signal(mySem12);

	Sleep(10000);

	FS::unmount();
	wait(myMutex);
	cout << ": Demontirana particija nit1" << endl;
	signal(myMutex);

	signal(cekajNit1);

	return 0;
}

DWORD WINAPI mojaNit2() {
	wait(mySem12);

	cout << ": Krenula nit2" << endl;
	
	FS::mount(partition);
	wait(myMutex);
	cout << ": Montirana particija nit2" << endl;
	signal(myMutex);

	FS::format();
	wait(myMutex);
	cout << ": Formatirana particija nit2" << endl;
	signal(myMutex);

	signal(cekajNit2);

	return 0;
}

int main() {
	HANDLE nit1,nit2;

	nit1 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)mojaNit1, NULL, 0, NULL);
	nit2 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)mojaNit2, NULL, 0, NULL);

	wait(cekajNit1);
	wait(cekajNit2);

	CloseHandle(myMutex);
	CloseHandle(mySem12);
	CloseHandle(cekajNit1);
	CloseHandle(cekajNit2);
}