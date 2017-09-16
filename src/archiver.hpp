#ifndef ARCHIVEC_CORE_ARCHIVER_HPP
#define ARCHIVEC_CORE_ARCHIVER_HPP

#include <string>
#include <list>

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

struct Stamp {
    uint8_t x = 0x52;
    uint8_t y = 0x84;
    uint8_t z = 0x91;
    uint32_t files_count;
} __attribute__((__packed__));

struct Entry {
    uint32_t id;            //id
    uint64_t size;          //file size
    uint32_t name_length;   //path+extension
} __attribute__((__packed__));

struct ArchivedFile{
    uint32_t  id;
    uint64_t  size;
    char* name;
} __attribute__((__packed__));

class Archiver {

private:
    std::vector<fs::path> *files_;
    std::vector<std::string> *titles_;

    std::string root_dir;
public:
    explicit Archiver();

    explicit Archiver(const char *dir_name);

    ~Archiver();

    void init(const char *dir_name);

    void crush(const char *out_file_name);

    void extract(const char *title, const char *dest_path);

    bool extract_file(const char *title, const char *dest_path, uint32_t file_id);

    std::vector<ArchivedFile> extract_files_info(const char *title);

    uint32_t extract_files_count(const char *title);


private:
    inline void mkdir(fs::path &path);

    inline bool check_stamp(const Stamp &stamp);
};


#endif //ARCHIVEC_CORE_ARCHIVER_HPP
