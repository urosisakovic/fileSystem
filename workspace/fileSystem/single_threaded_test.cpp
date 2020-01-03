#include "testprimer.h"

using namespace std;

//HANDLE nit1, nit2;
//DWORD ThreadID;
//
//HANDLE semMain = CreateSemaphore(NULL, 0, 32, NULL);
//HANDLE sem12 = CreateSemaphore(NULL, 0, 32, NULL);
//HANDLE sem21 = CreateSemaphore(NULL, 0, 32, NULL);
//HANDLE mutex = CreateSemaphore(NULL, 1, 32, NULL);
//
//Partition* partition;
//
//char *ulazBuffer;
//int ulazSize;


int STmain() {
	clock_t startTime, endTime;
	cout << "Pocetak testa!" << endl;
	startTime = clock(); //pocni merenje vremenas

	{//ucitavamo ulazni fajl u bafer, da bi nit 1 i 2 mogle paralelno da citaju
		FILE* f = fopen("ulaz.dat", "rb");
		if (f == 0) {
			cout << "GRESKA: Nije nadjen ulazni fajl 'ulaz.dat' u os domacinu!" << endl;
			system("PAUSE");
			return 0;//exit program
		}
		ulazBuffer = new char[32 * 1024 * 1024];//32MB
		ulazSize = fread(ulazBuffer, 1, 32 * 1024 * 1024, f);
		fclose(f);
	}


	wait(mutex);
	partition = new Partition((char*)"p1.ini");
	signal(mutex);

	wait(mutex);
	cout  << ": Kreirana particija" << endl;
	signal(mutex);

	FS::mount(partition);
	wait(mutex);
	cout << ": Montirana particija" << endl;
	signal(mutex);

	FS::format();
	wait(mutex);
	cout << ": Formatirana particija" << endl;
	signal(mutex);

	wait(mutex);
	cout << ": wait 1" << endl;
	signal(mutex);

	// cout << ClusterAllocation::freeClustersCount() << endl;

	{ //11
		char filepath[] = "/fajl1.dat";

		File* f = FS::open(filepath, 'w');
		wait(mutex);
		cout << ": Kreiran fajl '" << filepath << "'" << endl;
		signal(mutex); 

		f->write(ulazSize, ulazBuffer);
		wait(mutex);
		cout << ": Prepisan sadrzaj 'ulaz.dat' u '" << filepath << "'" << endl;
		signal(mutex);

		delete f;
		wait(mutex);
		cout << ": zatvoren fajl '" << filepath << "'" << endl;
		signal(mutex);
	}

	// cout << ClusterAllocation::freeClustersCount() << endl;

	{ // 21
		File* src, * dst;
		char filepath[] = "/fajl1.dat";

		while ((src = FS::open(filepath, 'r')) == 0) {
			wait(mutex);
			cout << ":Neuspesno otvoren fajl '" << filepath << "'" << endl;
			signal(mutex);
			Sleep(1); // Ceka 1 milisekundu
		}

		wait(mutex);
		cout << ": Otvoren fajl '" << filepath << "'" << endl;
		signal(mutex);

		char filepath1[] = "/fajl2.dat";
		dst = FS::open(filepath1, 'w');

		wait(mutex);
		cout << ": Otvoren fajl '" << filepath1 << "'" << endl;
		signal(mutex);

		char c;
		while (!src->eof()) {
			src->read(1, &c);
			dst->write(1, &c);
		}
		wait(mutex);
		cout << ": Prepisan fajl '" << filepath << "' u '" << filepath1 << "'" << endl;
		signal(mutex);

		delete dst;
		wait(mutex);
		cout << ": Zatvoren fajl '" << filepath1 << "'" << endl;
		signal(mutex);

		delete src;
		wait(mutex);
		cout << ": Zatvoren fajl '" << filepath << "'" << endl;
		signal(mutex);
	}

	// cout << ClusterAllocation::freeClustersCount() << endl;

	{  // 23
		char filepath[] = "/fajl2.dat";
		File* f = FS::open(filepath, 'r');

		wait(mutex);
		cout << ": Otvoren fajl " << filepath << "" << endl;
		signal(mutex);

		ofstream fout("izlaz1.dat", ios::out | ios::binary);

		char* buff = new char[f->getFileSize()];
		f->read(f->getFileSize(), buff);

		fout.write(buff, f->getFileSize());
		wait(mutex);
		cout << ": Upisan '" << filepath << "' u fajl os domacina 'izlaz1.dat'" << endl;
		signal(mutex);

		delete[] buff;
		fout.close();

		delete f;
		wait(mutex);
		cout << ": Zatvoren fajl " << filepath << "" << endl;
		signal(mutex);
	}

	// cout << ClusterAllocation::freeClustersCount() << endl;

	{ //12
		File* src, * dst;
		char filepath[] = "/fajl1.dat";

		src = FS::open(filepath, 'r');
		src->seek(src->getFileSize() / 2); //pozicionira se na pola fajla
		wait(mutex);
		cout << ": Otvoren fajl '" << filepath << "' i pozicionirani smo na polovini" << endl;
		signal(mutex);

		char filepath1[] = "/fajll5.dat";

		dst = FS::open(filepath1, 'w');
		wait(mutex);
		cout << ": Otvoren fajl '" << filepath1 << "'" << endl;
		signal(mutex);

		char c;
		while (!src->eof()) {
			src->read(1, &c);
			dst->write(1, &c);
		}
		wait(mutex);
		cout << ": Prepisana druga polovina '" << filepath << "' u '" << filepath1 << "'" << endl;
		signal(mutex);

		delete dst;
		wait(mutex);
		cout << ": Zatvoren fajl '" << filepath1 << "'" << endl;
		signal(mutex);

		delete src;
		wait(mutex);
		cout << ": Zatvoren fajl '" << filepath << "'" << endl;
		signal(mutex);
	}

	// cout << ClusterAllocation::freeClustersCount() << endl;

	{	// 24
		char copied_filepath[] = "/fajll5.dat";
		File* copy = FS::open(copied_filepath, 'r');

		BytesCnt size = copy->getFileSize();
		wait(mutex);
		cout << ": Otvoren fajl '" << copied_filepath << "' i dohvacena velicina" << endl;
		signal(mutex);

		delete copy;
		wait(mutex);
		cout << ": Zatvoren fajl '" << copied_filepath << "'" << endl;
		signal(mutex);

		File* src, * dst;
		char filepath[] = "/fajl1.dat";
		src = FS::open(filepath, 'r');
		src->seek(0);//pozicionira se na pola fajla

		wait(mutex);
		cout << ": Otvoren fajl '" << filepath << "' i pozicionirani smo na polovini" << endl;
		signal(mutex);

		char filepath1[] = "/fajl25.dat";

		dst = FS::open(filepath1, 'w');
		wait(mutex);
		cout << ": Otvoren fajl '" << filepath1 << "'" << endl;
		signal(mutex);

		char c; BytesCnt cnt = src->getFileSize() - size;
		while (!src->eof() && cnt-- > 0) {
			src->read(1, &c);
			dst->write(1, &c);
		}
		wait(mutex);
		cout << ": Prepisana druga polovina '" << filepath << "' u '" << filepath1 << "'" << endl;
		signal(mutex);

		delete dst;
		wait(mutex);
		cout << ": Zatvoren fajl '" << filepath1 << "'" << endl;
		signal(mutex);

		delete src;
		wait(mutex);
		cout << ": Zatvoren fajl '" << filepath << "'" << endl;
		signal(mutex);
	}

	// cout << ClusterAllocation::freeClustersCount() << endl;

	{ //13
		File* src, * dst;
		char filepath[] = "/fajl25.dat";

		dst = FS::open(filepath, 'a');

		wait(mutex);
		cout << ": Otvoren fajl '" << filepath << "'" << endl;
		signal(mutex);

		char filepath1[] = "/fajll5.dat";
		src = FS::open(filepath1, 'r');

		wait(mutex);
		cout << ": Otvoren fajl '" << filepath1 << "'" << endl;
		signal(mutex);

		char c;
		while (!src->eof()) {
			src->read(1, &c);
			dst->write(1, &c);
		}
		wait(mutex);
		cout << ": Prepisana druga polovina '" << filepath << "' u '" << filepath1 << "'" << endl;
		signal(mutex);

		delete dst;
		wait(mutex);
		cout << ": Zatvoren fajl '" << filepath1 << "'" << endl;
		signal(mutex);

		delete src;
		wait(mutex);
		cout << ": Zatvoren fajl '" << filepath << "'" << endl;
		signal(mutex);
	}

	// cout << ClusterAllocation::freeClustersCount() << endl;

	{	// 25
		char filepath[] = "/fajl25.dat";

		File* f = FS::open(filepath, 'r');
		wait(mutex);
		cout << ": Otvoren fajl " << filepath << "" << endl;
		signal(mutex);

		ofstream fout("izlaz2.dat", ios::out | ios::binary);
		char* buff = new char[f->getFileSize()];
		f->read(f->getFileSize(), buff);
		fout.write(buff, f->getFileSize());
		wait(mutex);
		cout << ": Upisan '" << filepath << "' u fajl os domacina 'izlaz2.dat'" << endl;
		signal(mutex);

		delete[] buff;
		fout.close();
		delete f;
		wait(mutex);
		cout << ": Zatvoren fajl " << filepath << "" << endl;
		signal(mutex);
	}

	// cout << ClusterAllocation::freeClustersCount() << endl;

	{ // 26
		FS::unmount();
		wait(mutex);
		cout << ": Demontirana particija p1" << endl;
		signal(mutex);
	}

	endTime = clock();
	cout << "Kraj test primera!" << endl;
	cout << "Vreme izvrsavanja: " << ((double)(endTime - startTime) / ((double)CLOCKS_PER_SEC / 1000.0)) << "ms!" << endl;

	return 0;
}