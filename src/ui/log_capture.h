#pragma once

#include <functional>
#include <mutex>
#include <streambuf>
#include <string>

/**
 * @brief Tee std::cout and std::cerr into a callback while active (one instance per thread).
 */
class StreamCapture {
public:
	explicit StreamCapture(std::function<void(const std::string& line)> onLine);
	~StreamCapture();

	StreamCapture(const StreamCapture&) = delete;
	StreamCapture& operator=(const StreamCapture&) = delete;

private:
	class TeeBuf : public std::streambuf {
	public:
		TeeBuf(std::streambuf* original, std::function<void(const std::string& line)> onLine);

	protected:
		int overflow(int ch) override;
		int sync() override;

	private:
		std::streambuf* original_;
		std::function<void(const std::string& line)> onLine_;
		std::string pending_;
	};

	std::function<void(const std::string& line)> onLine_;
	TeeBuf coutBuf_;
	TeeBuf cerrBuf_;
	std::streambuf* oldCout_;
	std::streambuf* oldCerr_;
};
