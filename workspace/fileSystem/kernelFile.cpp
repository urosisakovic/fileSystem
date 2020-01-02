#include "kernelFile.h"

KernelFile::KernelFile(ClusterNo rootDirCluster, ClusterNo rootDirEntry, bool canWrite) {
	this->filePtr = new FilePointer(rootDirCluster, rootDirEntry);

	this->clusterBuffer = new char[CLUSTER_SIZE];

	this->canWrite = canWrite;

	this->size = getFileSize();
}

KernelFile::~KernelFile() {
	KernelFS::close(fname);
	delete filePtr;
	delete[] clusterBuffer;
}

char KernelFile::write(BytesCnt bytesCnt, char* buffer) {
	if (!canWrite)
		return 0;
	
	if (bytesCnt == 0)
		return 1;
	
	filePtr->ensureDataCluster();

	unsigned currClusterFree = CLUSTER_SIZE - filePtr->pos;

	if (bytesCnt <= currClusterFree) {
		if (ClusterAllocation::readCluster(filePtr->dataCluster, clusterBuffer) == 0)
			return 0;
		memcpy(clusterBuffer + filePtr->pos, buffer, bytesCnt);
		if (ClusterAllocation::writeCluster(filePtr->dataCluster, clusterBuffer) == 0)
			return 0;

		filePtr->pos += bytesCnt;

		if (filePos() > getFileSize())
			FileSystemUtils::setLength(filePtr->rootDirCluster, filePtr->rootDirEntry, filePos());
		return 1;
	}

	if (ClusterAllocation::readCluster(filePtr->dataCluster, clusterBuffer) == 0)
		return 0;
	memcpy(clusterBuffer + filePtr->pos, buffer, currClusterFree);
	if (ClusterAllocation::writeCluster(filePtr->dataCluster, clusterBuffer) == 0)
		return 0;

	unsigned bufferPtr = currClusterFree;
	while (bytesCnt - bufferPtr >= CLUSTER_SIZE) {
		filePtr->GoToNextCluster();

		memcpy(clusterBuffer, buffer + bufferPtr, CLUSTER_SIZE);
		if (ClusterAllocation::writeCluster(filePtr->dataCluster, clusterBuffer) == 0)
			return 0;

		bufferPtr += CLUSTER_SIZE;
	}

	filePtr->GoToNextCluster();

	if (ClusterAllocation::readCluster(filePtr->dataCluster, clusterBuffer) == 0)
		return 0;
	memcpy(clusterBuffer, buffer + bufferPtr, bytesCnt - bufferPtr);
	if (ClusterAllocation::writeCluster(filePtr->dataCluster, clusterBuffer) == 0)
		return 0;

	filePtr->pos += bytesCnt - bufferPtr;

	std::cout << filePos() << std::endl;

	if (filePos() > getFileSize())
		FileSystemUtils::setLength(filePtr->rootDirCluster, filePtr->rootDirEntry, filePos());
	return 1;
}

BytesCnt KernelFile::read(BytesCnt bytesCnt, char* buffer) {
	if (bytesCnt == 0)
		return 0;

	if (bytesCnt + filePos() > getFileSize())
		bytesCnt = getFileSize() - filePos();

	BytesCnt readBytes = 0;

	unsigned currClusterFree = CLUSTER_SIZE - filePtr->pos;

	if (bytesCnt <= currClusterFree) {
		if (ClusterAllocation::readCluster(filePtr->dataCluster, clusterBuffer) == 0) {
			std::cout << "error read() 1" << std::endl;
			exit(1);
		}

		memcpy(buffer, clusterBuffer + filePtr->pos, bytesCnt);
		readBytes = bytesCnt;
		
		filePtr->pos += bytesCnt;

		return readBytes;
	}

	if (ClusterAllocation::readCluster(filePtr->dataCluster, clusterBuffer) == 0) {
		std::cout << "error read() 2" << std::endl;
		exit(1);
	}
	memcpy(buffer, clusterBuffer + filePtr->pos, currClusterFree);

	readBytes += currClusterFree;

	unsigned bufferPtr = currClusterFree;
	while (bytesCnt - bufferPtr >= CLUSTER_SIZE) {
		filePtr->GoToNextCluster();

		if (ClusterAllocation::readCluster(filePtr->dataCluster, clusterBuffer) == 0)
			return 0;
		memcpy(buffer + bufferPtr, clusterBuffer, CLUSTER_SIZE);

		bufferPtr += CLUSTER_SIZE;
		readBytes += bufferPtr;
	}

	filePtr->GoToNextCluster();
	if (ClusterAllocation::readCluster(filePtr->dataCluster, clusterBuffer) == 0) {
		std::cout << "error read() 3" << std::endl;
		exit(1);
	}

	memcpy(buffer + bufferPtr, clusterBuffer, bytesCnt - bufferPtr);
	readBytes += bytesCnt - bufferPtr;


	filePtr->pos += bytesCnt - bufferPtr;
	
	return readBytes;
}

// TODO: Check if new position is inside the file.
char KernelFile::seek(BytesCnt bytesCnt) {
	if (bytesCnt >= getFileSize())
		return 0;

	filePtr->lvl1IndexEntry = bytesCnt / (1 << 22);

	if (ClusterAllocation::readCluster(filePtr->lvl1IndexCluster, clusterBuffer) == 0) {
		std::cout << "error seek() 1" << std::endl;
		exit(1);
	}

	ClusterNo* lvl2IndexClusterPtr = (ClusterNo*)clusterBuffer + filePtr->lvl1IndexEntry;
	filePtr->lvl2IndexCluster = *lvl2IndexClusterPtr;

	bytesCnt -= filePtr->lvl1IndexEntry * (1 << 22);

	filePtr->lvl2IndexEntry = bytesCnt / (1 << 11);;

	if (ClusterAllocation::readCluster(filePtr->lvl2IndexCluster, clusterBuffer) == 0) {
		std::cout << "error seek() 2" << std::endl;
		exit(1);
	}

	ClusterNo* dataClusterPtr = (ClusterNo*)clusterBuffer + filePtr->lvl2IndexEntry;
	filePtr->dataCluster = *dataClusterPtr;

	bytesCnt -= filePtr->lvl2IndexEntry * (1 << 11);

	filePtr->pos = bytesCnt;

	return 1;
}

BytesCnt KernelFile::filePos() {
	return filePtr->byteOffset();
}

char KernelFile::eof() {
	if (filePos() == getFileSize())
		return 1;
	return 0;
}

BytesCnt KernelFile::getFileSize() {
	return FileSystemUtils::readLength(filePtr->rootDirCluster, filePtr->rootDirEntry);
}

char KernelFile::truncate() {
	if (filePtr->lvl1IndexCluster == 0)
		return 1;

	unsigned truncatedSize = filePos() + 1;

	// delete part of the current data cluster
	char* currDataCluster = new char[CLUSTER_SIZE];
	if (ClusterAllocation::readCluster(filePtr->dataCluster, currDataCluster) == 0) {
		std::cout << "error in truncate() 1" << std::endl;
		exit(1);
	}

	memset(currDataCluster + filePtr->pos + 1, 0, CLUSTER_SIZE - filePtr->pos - 1);

	if (ClusterAllocation::writeCluster(filePtr->dataCluster, currDataCluster) == 0) {
		std::cout << "error in truncate() 2" << std::endl;
		exit(1);
	}

	// deallocate data clusters
	char* lvl2Index = new char[CLUSTER_SIZE];
	if (ClusterAllocation::readCluster(filePtr->lvl2IndexCluster, lvl2Index) == 0) {
		std::cout << "error in truncate() 3" << std::endl;
		exit(1);
	}

	ClusterNo* dataClusterPtr;
	for (int i = filePtr->lvl2IndexEntry + 1; i < ENTRIES_PER_INDEX; i++) {
		dataClusterPtr = (ClusterNo*)lvl2Index + i;

		if (*dataClusterPtr != 0) {
			if (ClusterAllocation::deallocateCluster(*dataClusterPtr) == 0) {
				std::cout << "error in truncate() 4" << std::endl;
				exit(1);
			}
			*dataClusterPtr = 0;
		}
	}

	if (ClusterAllocation::writeCluster(filePtr->lvl2IndexCluster, lvl2Index) == 0) {
		std::cout << "error in truncate() 5" << std::endl;
		exit(1);
	}


	// deallocate lvl2 index clusters
	char* lvl1Index = new char[CLUSTER_SIZE];
	if (ClusterAllocation::readCluster(filePtr->lvl1IndexCluster, lvl1Index) == 0) {
		std::cout << "error in truncate() 6" << std::endl;
		exit(1);
	}

	ClusterNo* lvl2IndexClusterPtr;
	for (int i = filePtr->lvl1IndexEntry + 1; i < ENTRIES_PER_INDEX; i++) {
		lvl2IndexClusterPtr = (ClusterNo*)lvl1Index + i;

		if (*lvl2IndexClusterPtr != 0) {
			if (ClusterAllocation::readCluster(*lvl2IndexClusterPtr, lvl2Index) == 0) {
				std::cout << "error in truncate() 7" << std::endl;
				exit(1);
			}

			for (int i = 0; i < ENTRIES_PER_INDEX; i++) {
				dataClusterPtr = (ClusterNo*)lvl2Index + i;

				if (*dataClusterPtr != 0) {
					if (ClusterAllocation::deallocateCluster(*dataClusterPtr) == 0) {
						std::cout << "error in truncate() 8" << std::endl;
						exit(1);
					}
					*dataClusterPtr = 0;
				}
			}

			if (ClusterAllocation::deallocateCluster(*lvl2IndexClusterPtr) == 0) {
				std::cout << "error in truncate() 9" << std::endl;
				exit(1);
			}
		}

		*lvl2IndexClusterPtr = 0;
	}

	if (ClusterAllocation::writeCluster(filePtr->lvl1IndexCluster, lvl1Index) == 0) {
		std::cout << "error in truncate() 10" << std::endl;
		exit(1);
	}

	// update size of the file in the root directory
	FileSystemUtils::setLength(filePtr->rootDirCluster, filePtr->rootDirEntry, truncatedSize);

	delete[] lvl2Index;
	delete[] lvl1Index;
	delete[] currDataCluster;
	return 1;
}
