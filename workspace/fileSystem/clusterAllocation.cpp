#include "clusterAllocation.h"

Partition* ClusterAllocation::partition = nullptr;
char* ClusterAllocation::bitVector = nullptr;
unsigned ClusterAllocation::bitVectorByteSize = 0;
unsigned ClusterAllocation::clusterCount = 0;
ClusterAllocation::CacheNode* ClusterAllocation::head = nullptr;
ClusterAllocation::CacheNode* ClusterAllocation::tail = nullptr;
std::unordered_map<ClusterNo, ClusterAllocation::CacheNode*>* ClusterAllocation::cachedMap = new std::unordered_map<ClusterNo, ClusterAllocation::CacheNode*>();
unsigned ClusterAllocation::cacheSize = 0;
unsigned ClusterAllocation::maxCacheSize = 0;

// TODO: Optimize this to avoid unncessery (de)allocation but to reuse nodes
int ClusterAllocation::readCluster(ClusterNo cluster, char* buffer) {
	if (cachedMap->find(cluster) == cachedMap->end()) {
		CacheNode* ch = new CacheNode(cluster);
		if (partition->readCluster(cluster, ch->cachedCluster) == 0)
			return 0;

		if (cacheSize == maxCacheSize) {
			CacheNode* delNode = tail;
			tail = tail->prev;
			if (tail == nullptr)
				head = nullptr;

			cachedMap->erase(cachedMap->find(delNode->cluster));
			if (delNode->changed)
				partition->writeCluster(delNode->cluster, delNode->cachedCluster);

			delete delNode;

			cacheSize--;
		}

		if (head == nullptr)
			head = tail = ch;
		else {
			ch->next = head;
			head->prev = ch;
			head = ch;
		}

		(*cachedMap)[cluster] = ch;
		cacheSize++;
	}
	
	CacheNode* ch = (*cachedMap)[cluster];

	if (ch != head) {
		if (ch->next != nullptr) {
			(ch->next)->prev = ch->prev;
		}
		if (ch->prev != nullptr) {
			(ch->prev)->next = ch->next;
		}

		if (head == nullptr)
			head = tail = ch;
		else {
			ch->next = head;
			head->prev = ch;
			head = ch;
		}

		(*cachedMap)[cluster] = ch;
		cacheSize++;
	}


	memcpy(buffer, ch->cachedCluster, CLUSTER_SIZE);

	return 1;
}

// TODO: Same thing as above. Also, remove this code duplication.
int ClusterAllocation::writeCluster(ClusterNo cluster, const char* buffer) {
	if (cachedMap->find(cluster) == cachedMap->end()) {
		CacheNode* ch = new CacheNode(cluster);
		if (partition->readCluster(cluster, ch->cachedCluster) == 0)
			return 0;

		if (cacheSize == maxCacheSize) {
			CacheNode* delNode = tail;
			tail = tail->prev;
			if (tail == nullptr)
				head = nullptr;

			cachedMap->erase(cachedMap->find(delNode->cluster));
			if (delNode->changed)
				partition->writeCluster(delNode->cluster, delNode->cachedCluster);

			delete delNode;

			cacheSize--;
		}

		ch->next = head;
		head = ch;
		if (tail == nullptr)
			tail = ch;

		(*cachedMap)[cluster] = ch;
		cacheSize++;
	}

	CacheNode* ch = (*cachedMap)[cluster];

	if (ch != head) {
		if (ch->next != nullptr) {
			(ch->next)->prev = ch->prev;
		}
		if (ch->prev != nullptr) {
			(ch->prev)->next = ch->next;
		}

		if (ch == tail) {
			tail = ch->prev;
		}

		ch->next = head;
		ch->prev = nullptr;
		head = ch;
	}

	ch->changed = true;
	memcpy(ch->cachedCluster, buffer, CLUSTER_SIZE);

	return 1;
}

void ClusterAllocation::markAllocated(ClusterNo clusterId) {
	unsigned correspondingByte = clusterId / 8;
	unsigned correspondingBit = 7 - clusterId % 8;

	bitVector[correspondingByte] &= ~(1 << correspondingBit);
}

void ClusterAllocation::markDeallocated(ClusterNo clusterId) {
	unsigned correspondingByte = clusterId / 8;
	unsigned correspondingBit = 7 - clusterId % 8;

	bitVector[correspondingByte] |= (1 << correspondingBit);
}

char ClusterAllocation::checkAllocated(ClusterNo clusterId) {
	unsigned correspondingByte = clusterId / 8;
	unsigned correspondingBit = 7 - clusterId % 8;

	if (correspondingByte >= bitVectorByteSize)
		return -1;

	return ((bitVector[correspondingByte] & (1 << correspondingBit)) == 0);
}

void ClusterAllocation::setPartition(Partition* p) {	
	CacheNode* delNode, *itr = head;
	while (itr != nullptr) {
		delNode = itr;
		itr = itr->next;
	
		if (delNode->changed)
			partition->writeCluster(delNode->cluster, delNode->cachedCluster);

		cachedMap->erase(cachedMap->find(delNode->cluster));
		delete delNode;
	}

	head = tail = nullptr;
	cacheSize = 0;

	partition = p;
	clusterCount = p->getNumOfClusters();
	maxCacheSize = clusterCount / 10;
}

void ClusterAllocation::setBitVector(unsigned bVSize, char* bV) {
	if (bitVector != nullptr)
		delete[] bitVector;

	bitVector = new char[bitVectorByteSize = bVSize];
	memcpy(bitVector, bV, bVSize);
}

long ClusterAllocation::freeClustersCount() {
	long cnt = 0;

	for (int i = 0; i < clusterCount; i++)
		if (!checkAllocated(i))
			cnt++;

	return cnt;
}

ClusterNo ClusterAllocation::allocateCluster() {
	char clusterBuffer[CLUSTER_SIZE];

	for (ClusterNo i = 0; i < clusterCount; i++) {
		if (!checkAllocated(i)) {
			markAllocated(i);
			// std::cout << "Allocated cluster: " << i << std::endl;
			memset(clusterBuffer, 0, CLUSTER_SIZE);
			writeCluster(i, clusterBuffer);

			return i;
		}
	}

	std::cout << "CANNOT ALLOCATE CLUSTER." << std::endl;
	return 0;
}

char ClusterAllocation::deallocateCluster(ClusterNo freeClusterIdx) {
	if (!checkAllocated(freeClusterIdx))
		return 0;

	markDeallocated(freeClusterIdx);
	return 1;
}