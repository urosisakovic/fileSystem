#include "kernelFile.h"

KernelFile::KernelFile(ClusterNo rootDirCluster, ClusterNo rootDirEntry, bool canWrite) {
	this->filePtr = new FilePointer(rootDirCluster, rootDirEntry);

	this->clusterBuffer = new char[CLUSTER_SIZE];

	this->canWrite = canWrite;

	this->size = getFileSize();
}

KernelFile::~KernelFile() {
	delete filePtr;
	delete[] clusterBuffer;
}

char KernelFile::write(BytesCnt bytesCnt, char* buffer) {
	if (!canWrite)
		return 0;
	
	if (bytesCnt == 0)
		return 1;
	
	filePtr->ensureDataCluster();

	if (filePos() > getFileSize())
		FileSystemUtils::setLength(filePtr->rootDirCluster, filePtr->rootDirEntry, filePos());

	unsigned currClusterFree = CLUSTER_SIZE - filePtr->pos;

	if (bytesCnt <= currClusterFree) {
		if (ClusterAllocation::readCluster(filePtr->dataCluster, clusterBuffer) == -1)
			return 0;
		memcpy(clusterBuffer + filePtr->pos, buffer, bytesCnt);
		if (ClusterAllocation::writeCluster(filePtr->dataCluster, clusterBuffer) == -1)
			return 0;

		filePtr->pos += bytesCnt;

		if (filePos() > getFileSize())
			FileSystemUtils::setLength(filePtr->rootDirCluster, filePtr->rootDirEntry, filePos());
		return 1;
	}

	if (ClusterAllocation::readCluster(filePtr->dataCluster, clusterBuffer) == -1)
		return 0;
	memcpy(clusterBuffer + filePtr->pos, buffer, currClusterFree);
	if (ClusterAllocation::writeCluster(filePtr->dataCluster, clusterBuffer) == -1)
		return 0;

	unsigned bufferPtr = currClusterFree;
	while (bytesCnt - bufferPtr >= CLUSTER_SIZE) {
		filePtr->GoToNextCluster();

		if (ClusterAllocation::readCluster(filePtr->dataCluster, clusterBuffer) == -1)
			return 0;
		memcpy(clusterBuffer, buffer + bufferPtr, CLUSTER_SIZE);
		if (ClusterAllocation::writeCluster(filePtr->dataCluster, clusterBuffer) == -1)
			return 0;

		bufferPtr += CLUSTER_SIZE;
	}

	filePtr->GoToNextCluster();
	if (ClusterAllocation::readCluster(filePtr->dataCluster, clusterBuffer) == -1)
		return 0;
	memcpy(clusterBuffer, buffer + bufferPtr, bytesCnt - bufferPtr);
	if (ClusterAllocation::writeCluster(filePtr->dataCluster, clusterBuffer) == -1)
		return 0;

	filePtr->pos += bytesCnt - bufferPtr;

	if (filePos() > getFileSize())
		FileSystemUtils::setLength(filePtr->rootDirCluster, filePtr->rootDirEntry, filePos());
	return 1;
}

BytesCnt KernelFile::read(BytesCnt bytesCnt, char* buffer) {
	if (bytesCnt == 0)
		return 1;

	unsigned currClusterFree = CLUSTER_SIZE - filePtr->pos;

	if (bytesCnt <= currClusterFree) {
		if (ClusterAllocation::readCluster(filePtr->dataCluster, clusterBuffer) == -1)
			return 0;
		memcpy(buffer, clusterBuffer + filePtr->pos, bytesCnt);
		
		filePtr->pos += bytesCnt;

		return 1;
	}

	if (ClusterAllocation::readCluster(filePtr->dataCluster, clusterBuffer) == -1)
		return 0;
	memcpy(buffer, clusterBuffer + filePtr->pos, currClusterFree);

	unsigned bufferPtr = currClusterFree;
	while (bytesCnt - bufferPtr >= CLUSTER_SIZE) {
		filePtr->GoToNextCluster();

		if (ClusterAllocation::readCluster(filePtr->dataCluster, clusterBuffer) == -1)
			return 0;
		memcpy(buffer + bufferPtr, clusterBuffer, CLUSTER_SIZE);

		bufferPtr += CLUSTER_SIZE;
	}

	filePtr->GoToNextCluster();
	if (ClusterAllocation::readCluster(filePtr->dataCluster, clusterBuffer) == -1)
		return 0;
	memcpy(buffer + bufferPtr, clusterBuffer, bytesCnt - bufferPtr);

	filePtr->pos += bytesCnt - bufferPtr;
	
	return 1;
}

// TODO: Check if new position is inside the file.
char KernelFile::seek(BytesCnt bytesCnt) {
	filePtr->lvl1IndexEntry = bytesCnt / (1 << 22);

	if (ClusterAllocation::readCluster(filePtr->lvl1IndexCluster, clusterBuffer) == -1)
		return 0;
	ClusterNo* lvl2IndexClusterPtr = (ClusterNo*)clusterBuffer + filePtr->lvl1IndexEntry;
	filePtr->lvl2IndexCluster = *lvl2IndexClusterPtr;

	bytesCnt -= filePtr->lvl1IndexEntry * (1 << 22);

	filePtr->lvl2IndexEntry = bytesCnt / (1 << 11);;

	if (ClusterAllocation::readCluster(filePtr->lvl2IndexCluster, clusterBuffer) == -1)
		return 0;
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
	ClusterAllocation::readCluster(filePtr->lvl1IndexCluster, clusterBuffer);

	char additionalBuffer[CLUSTER_SIZE], lvl2IndexBuffer[CLUSTER_SIZE];

	ClusterNo *lvl1Ptr, *lvl2Ptr, *dataPtr;
	for (ClusterNo i = 0; i < ENTRIES_PER_INDEX; i++) {
		lvl1Ptr = (ClusterNo*)clusterBuffer + i;

		if (*lvl1Ptr == 0)
			continue;

		ClusterAllocation::readCluster(*lvl1Ptr, additionalBuffer);

		for (ClusterNo j = 0; j < ENTRIES_PER_INDEX; j++) {
			lvl2Ptr = (ClusterNo*)additionalBuffer + j;

			if (*lvl2Ptr == 0)
				continue;

			ClusterAllocation::readCluster(*lvl2Ptr, lvl2IndexBuffer);

			for (ClusterNo k = 0; k < ENTRIES_PER_INDEX; k++) {
				dataPtr = (ClusterNo*)lvl2IndexBuffer + k;

				if (*dataPtr == 0)
					continue;

				ClusterAllocation::deallocateCluster(*dataPtr);
			}

			ClusterAllocation::deallocateCluster(*lvl2Ptr);
		}

		ClusterAllocation::deallocateCluster(*lvl1Ptr);
		*lvl1Ptr = 0;
	}

	FileSystemUtils::setLength(filePtr->rootDirCluster, filePtr->rootDirEntry, 0);

	return 1;
	
	return 0;
}
