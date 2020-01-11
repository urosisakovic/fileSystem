#include "file.h"

File::~File() {
	KernelFS::enterCriticSection();
	KernelFS::releaseFile(myImpl->fname);
	KernelFS::exitCriticSection();
	delete myImpl;
}

char File::write(BytesCnt bytesCnt, char* buffer) {
	return myImpl->write(bytesCnt, buffer);
}

BytesCnt File::read(BytesCnt bytesCnt, char* buffer) {
	return myImpl->read(bytesCnt, buffer);
}

char File::seek(BytesCnt bytesCnt) {
	return myImpl->seek(bytesCnt);
}

BytesCnt File::filePos() {
	return myImpl->filePos();
}

char File::eof() {
	return myImpl->eof();
}

BytesCnt File::getFileSize() {
	return myImpl->getFileSize();
}

char File::truncate() {
	return myImpl->truncate();
}
