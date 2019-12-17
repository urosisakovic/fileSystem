#include "kernelFile.h"

KernelFile::~KernelFile()
{
}

char KernelFile::write(BytesCnt bytesCnt, char* buffer)
{
	return 0;
}

BytesCnt KernelFile::read(BytesCnt bytesCnt, char* buffer)
{
	return BytesCnt();
}

char KernelFile::seek(BytesCnt bytesCnt)
{
	return 0;
}

BytesCnt KernelFile::filePos()
{
	return BytesCnt();
}

char KernelFile::eof()
{
	return 0;
}

BytesCnt KernelFile::getFileSize()
{
	return BytesCnt();
}

char KernelFile::truncate()
{
	return 0;
}
