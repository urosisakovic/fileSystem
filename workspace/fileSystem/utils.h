#pragma once

typedef unsigned long ClusterNo;
const unsigned long CLUSTER_SIZE = 2048;
const unsigned long BITS_PER_CLUSTER = CLUSTER_SIZE * 8;

const unsigned ROOT_DIR_ENTRY_SIZE = 32;
typedef unsigned char rootDirEntry[ROOT_DIR_ENTRY_SIZE];

const unsigned long ENTRIES_PER_INDEX = CLUSTER_SIZE / sizeof(ClusterNo);
const unsigned long ENTRIES_PER_ROOT_DIR = CLUSTER_SIZE / sizeof(rootDirEntry);

typedef unsigned long FileCnt;
typedef unsigned long BytesCnt;
const unsigned int FNAMELEN = 8;
const unsigned int FEXTLEN = 3;