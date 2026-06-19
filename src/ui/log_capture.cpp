#include "log_capture.h"

#include <iostream>

StreamCapture::TeeBuf::TeeBuf(
	std::streambuf* original,
	std::function<void(const std::string& line)> onLine)
	: original_(original)
	, onLine_(std::move(onLine))
{
}

int StreamCapture::TeeBuf::overflow(const int ch) {
	if (ch == EOF) {
		return 0;
	}

	const char c = static_cast<char>(ch);
	if (c == '\n') {
		if (onLine_ && !pending_.empty()) {
			onLine_(pending_);
		}
		pending_.clear();
	}
	else {
		pending_ += c;
	}

	if (original_ != nullptr) {
		return original_->sputc(c);
	}
	return ch;
}

int StreamCapture::TeeBuf::sync() {
	if (original_ != nullptr) {
		return original_->pubsync();
	}
	return 0;
}

StreamCapture::StreamCapture(std::function<void(const std::string& line)> onLine)
	: onLine_(std::move(onLine))
	, coutBuf_(std::cout.rdbuf(), onLine_)
	, cerrBuf_(std::cerr.rdbuf(), onLine_)
	, oldCout_(std::cout.rdbuf())
	, oldCerr_(std::cerr.rdbuf())
{
	std::cout.rdbuf(&coutBuf_);
	std::cerr.rdbuf(&cerrBuf_);
}

StreamCapture::~StreamCapture() {
	std::cout.rdbuf(oldCout_);
	std::cerr.rdbuf(oldCerr_);
}
