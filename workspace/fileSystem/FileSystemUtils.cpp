#include "FileSystemUtils.h"

char* FileSystemUtils::clusterBuffer = new char[CLUSTER_SIZE];

char FileSystemUtils::setDataCluster(ClusterNo lvl2Index, ClusterNo lvl2IndexEntry, ClusterNo dataCluster) {
	if (ClusterAllocation::readCluster(lvl2Index, clusterBuffer) == 0)
		return 0;

	ClusterNo* entry = (ClusterNo*)clusterBuffer + lvl2IndexEntry;

	*entry = dataCluster;

	if (ClusterAllocation::writeCluster(lvl2Index, clusterBuffer) == 0)
		return 0;

	return 1;
}

BytesCnt FileSystemUtils::readLength(ClusterNo rootDirCluster, ClusterNo rootEntry) {
	if (ClusterAllocation::readCluster(rootDirCluster, clusterBuffer) == 0)
		return 0;

	rootDirEntry* entry = (rootDirEntry*)clusterBuffer + rootEntry;

	BytesCnt* length = (BytesCnt*)((char*)(*entry) + 16);

	return *length;
}

char FileSystemUtils::emptyRootDirEntry(ClusterNo rootDirCluster, ClusterNo rootEntry) {
	if (ClusterAllocation::readCluster(rootDirCluster, clusterBuffer) == 0)
		return 0;

	rootDirEntry* entry = (rootDirEntry*)clusterBuffer + rootEntry;

	memset(entry, 0, ROOT_DIR_ENTRY_SIZE);

	if (ClusterAllocation::writeCluster(rootDirCluster, clusterBuffer) == 0)
		return 0;

	return 1;
}

void FileSystemUtils::splitFileName(char* fname, char** fileName, char** extension) {
	int fnameLen = strlen(fname);
	int dotPos = -1;
	for (int i = fnameLen - 1; i >= 0; i--) {
		if (fname[i] == '.') {
			dotPos = i;
			break;
		}
	}

	if (dotPos == -1) {
		*fileName = nullptr;
		return;
	}
	if (dotPos == fnameLen - 1) {
		*fileName = nullptr;
		return;
	}
	if (dotPos == 0) {
		*fileName = nullptr;
		return;
	}
	if (dotPos > 8) {
		*fileName = nullptr;
		return;
	}
	if (fnameLen - dotPos - 1 > 3) {
		*fileName = nullptr;
		return;
	}

	*fileName = new char[dotPos + 1];
	*extension = new char[(fnameLen - dotPos - 1) + 1];

	for (int i = 0; i < dotPos; i++)
		(*fileName)[i] = fname[i];
	(*fileName)[dotPos] = '\0';
	for (int i = dotPos + 1; i < fnameLen; i++)
		(*extension)[i - dotPos - 1] = fname[i];
	(*extension)[fnameLen - dotPos - 1] = '\0';
}

void FileSystemUtils::getFileInfo(char* fname, ClusterNo* lvl1IndexCluster, ClusterNo* rootDirCluster, ClusterNo* rootDirEntryNum) {
	*lvl1IndexCluster = *rootDirCluster = *rootDirEntryNum = 0;

	char* fileName = nullptr, * extension = nullptr;
	//TODO: Fix this design
	splitFileName(fname, &fileName, &extension);
	if (fileName == nullptr)
		return;

	ClusterNo* lvl1Ptr, * lvl2Ptr;
	rootDirEntry* fileEntry;

	char* lvl1Buffer = new char[CLUSTER_SIZE];
	char* lvl2Buffer = new char[CLUSTER_SIZE];
	char* fileEntryFileName = new char[9];
	char* fileEntryExtension = new char[4];

	if (ClusterAllocation::readCluster(KernelFS::rootDirLvl1Index, lvl1Buffer) == 0)
		return;

	for (int i = 0; i < ENTRIES_PER_INDEX; i++) {
		lvl1Ptr = (ClusterNo*)lvl1Buffer + i;

		if (*lvl1Ptr == 0)
			continue;

		if (ClusterAllocation::readCluster((*lvl1Ptr), lvl2Buffer) == 0)
			return;

		for (int j = 0; j < ENTRIES_PER_INDEX; j++) {
			lvl2Ptr = (ClusterNo*)lvl2Buffer + j;

			if (*lvl2Ptr == 0)
				continue;

			if (ClusterAllocation::readCluster((*lvl2Ptr), clusterBuffer) == 0)
				return;

			for (int k = 0; k < ENTRIES_PER_ROOT_DIR; k++) {
				fileEntry = (rootDirEntry*)clusterBuffer + k;

				for (int i = 0; i < 8; i++)
					fileEntryFileName[i] = (*fileEntry)[i];
				fileEntryFileName[8] = '\0';

				for (int i = 0; i < 3; i++)
					fileEntryExtension[i] = (*fileEntry)[i + 8];
				fileEntryExtension[3] = '\0';

				if ((strcmp(fileName, fileEntryFileName) == 0) &&
					(strcmp(extension, fileEntryExtension) == 0)) {

					*lvl1IndexCluster = *(ClusterNo*)((char*)(fileEntry) + 12);
					*rootDirCluster = *lvl2Ptr;
					*rootDirEntryNum = k;

					delete[] fileEntryFileName;
					delete[] fileEntryExtension;
					delete[] lvl1Buffer;
					delete[] lvl2Buffer;
					return;
				}
			}
		}
	}

	delete[] lvl1Buffer;
	delete[] lvl2Buffer;
	delete[] fileEntryFileName;
	delete[] fileEntryExtension;

	return;
}

char FileSystemUtils::setLength(ClusterNo rootDirCluster, ClusterNo rootEntry, unsigned size) {
	if (ClusterAllocation::readCluster(rootDirCluster, clusterBuffer) == 0)
		return 0;

	rootDirEntry* entry = (rootDirEntry*)clusterBuffer + rootEntry;

	BytesCnt* length = (BytesCnt*)((char*)(*entry) + 16);
	*length = size;

	if (ClusterAllocation::writeCluster(rootDirCluster, clusterBuffer) == 0)
		return 0;

	return 1;
}

char FileSystemUtils::setLvl1Index(ClusterNo rootDirCluster, ClusterNo rootEntry, ClusterNo lvl1Index) {
	if (ClusterAllocation::readCluster(rootDirCluster, clusterBuffer) == 0)
		return 0;

	rootDirEntry* entry = (rootDirEntry*)clusterBuffer + rootEntry;

	ClusterNo* lvl1 = (ClusterNo*)((char*)entry + 12);
	*lvl1 = lvl1Index;

	if (ClusterAllocation::writeCluster(rootDirCluster, clusterBuffer) == 0)
		return 0;

	return 1;
}

ClusterNo FileSystemUtils::getLvl1Index(ClusterNo rootDirCluster, ClusterNo rootEntry) {
	if (ClusterAllocation::readCluster(rootDirCluster, clusterBuffer) == 0)
		return 0;

	rootDirEntry* entry = (rootDirEntry*)clusterBuffer + rootEntry;

	ClusterNo* lvl1 = (ClusterNo*)((char*)entry + 12);

	return *lvl1;
}

ClusterNo FileSystemUtils::getLvl2Index(ClusterNo lvl1IndexCluster, ClusterNo lvl1IndexEntry) {
	if (ClusterAllocation::readCluster(lvl1IndexCluster, clusterBuffer) == 0)
		return 0;

	ClusterNo* lvl2Entry = (ClusterNo*)clusterBuffer + lvl1IndexEntry;

	return *lvl2Entry;
}

ClusterNo FileSystemUtils::getDataCluster(ClusterNo lvl2IndexCluster, ClusterNo lvl2IndexEntry) {
	if (ClusterAllocation::readCluster(lvl2IndexCluster, clusterBuffer) == 0)
		return 0;

	ClusterNo* dataEntry = (ClusterNo*)clusterBuffer + lvl2IndexEntry;

	return *dataEntry;
}

char FileSystemUtils::setLvl2Index(ClusterNo lvl1Index, ClusterNo lvl1IndexEntry, ClusterNo lvl2Index) {
	if (ClusterAllocation::readCluster(lvl1Index, clusterBuffer) == 0)
		return 0;

	ClusterNo* entry = (ClusterNo*)clusterBuffer + lvl1IndexEntry;

	*entry = lvl2Index;

	if (ClusterAllocation::writeCluster(lvl1Index, clusterBuffer) == 0)
		return 0;

	return 1;
}