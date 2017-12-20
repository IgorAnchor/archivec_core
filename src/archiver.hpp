#ifndef ARCHIVEC_CORE_ARCHIVER_HPP
#define ARCHIVEC_CORE_ARCHIVER_HPP

#include <string>
#include <list>
#include <boost/filesystem.hpp>

#include "util/compressors/lzw/lzw.hpp"

namespace fs = boost::filesystem;

struct Stamp {
	uint8_t x = 0x52;
	uint8_t y = 0x84;
	uint8_t z = 0x91;
	unsigned filesCount = 0;
};

struct Entry {
	unsigned id;
	uint64_t size;
	uint64_t compressedSize;
	unsigned nameLength;
};

struct ArchivedFile {
	unsigned id;
	uint64_t size;
	uint64_t compressedSize;
	wchar_t *name;
};

class Archiver {
private:
	std::vector<fs::path> *files;
	std::vector<std::wstring> *titles;

	unsigned maxBufferSize = 2'000'000;

public:
	explicit Archiver();
	~Archiver();

	void crush(std::wstring_view fileName, bool askReplace = true);
	void add(std::vector<std::wstring_view> &files);
	void addToExisting(std::vector<std::wstring_view> &files, std::wstring_view archive);
	void extract(std::wstring_view archive, std::wstring_view dest, bool askReplace = true);
	bool extractFiles(std::wstring_view archive, std::wstring_view dest, std::vector<unsigned> &ids, bool askReplace = true, bool fullPath = true);
	void remove(std::vector<unsigned> &ids, std::wstring_view archive);
	std::vector<ArchivedFile> extractInfo(std::wstring_view archive);
	unsigned getLastId(std::wstring_view path);
	void setBufferSize(unsigned newSize);
	void reset();

private:
	void initDir(std::wstring_view dirName, std::wstring_view rootDirName);
	inline void mkdir(fs::path &path);
	inline bool checkStamp(const Stamp &stamp);
	inline bool checkReplace(fs::path &path, bool askReplace = true);
	inline void rewriteFileCompress(FILE *in, FILE *out, const uint64_t &fileSize, uint64_t &compressedSize);
	inline void rewriteFileExpand(FILE *in, FILE *out, const uint64_t &compressedSize, const uint64_t &fileSize);
};

#endif //ARCHIVEC_CORE_ARCHIVER_HPP
