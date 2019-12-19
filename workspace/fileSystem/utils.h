#pragma once

typedef unsigned long ClusterNo;
const unsigned long CLUSTER_SIZE = 2048;
const unsigned long BITS_PER_CLUSTER = CLUSTER_SIZE * 8;

typedef unsigned char rootDirEntry[20];

typedef unsigned long FileCnt;
typedef unsigned long BytesCnt;
const unsigned int FNAMELEN = 8;
const unsigned int FEXTLEN = 3;