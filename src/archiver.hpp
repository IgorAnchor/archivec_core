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
	uint32_t files_count;
};

struct Entry {
	uint32_t id;
	uint64_t size;
	uint64_t compressedSize;
	uint32_t nameLength;
};

struct ArchivedFile {
	uint32_t id;
	uint64_t size;
	uint64_t compressedSize;
	uint8_t *name;
};

class Archiver {
private:
	std::vector<fs::path> *files_;
	std::vector<std::string> *titles_;

	uint32_t maxBufferSize = 2'000'000;

public:
	explicit Archiver();
	~Archiver();

	void crush(std::string_view fileName, bool askReplace = true);
	void add(std::vector<std::string_view> &files);
	void addToExisting(std::vector<std::string_view> &files, std::string_view archive);
	void extract(std::string_view archive, std::string_view dest, bool askReplace = true);
	bool extractFiles(std::string_view archive, std::string_view dest, std::vector<uint32_t> &ids, bool askReplace = true);
	void remove(std::vector<uint32_t> &ids, std::string_view archive);
	std::vector<ArchivedFile> extractInfo(std::string_view archive);
	uint32_t getLastId(std::string_view path);
	void setBufferSize(uint32_t new_size);
	void reset();

private:
	void initDir(std::string_view dirName, std::string_view rootDirName);
	inline void mkdir(fs::path &path);
	inline bool checkStamp(const Stamp &stamp);
	inline bool checkReplace(fs::path &path, bool askReplace = true);
	inline void rewriteFileCompress(FILE *in, FILE *out, const uint64_t &fileSize, uint64_t &compressedSize);
	inline void rewriteFileExpand(FILE *in, FILE *out, const uint64_t &compressedSize, const uint64_t &fileSize);
};

#endif //ARCHIVEC_CORE_ARCHIVER_HPP
