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
    uint32_t id;
    uint64_t size;
    uint32_t name_length;
} __attribute__((__packed__));

struct ArchivedFile{
    uint32_t  id;
    uint64_t  size;
    uint8_t* name;
} __attribute__((__packed__));

class Archiver {

private:
    std::vector<fs::path> *files_;
    std::vector<std::string> *titles_;
public:
    explicit Archiver();

    explicit Archiver(std::string_view dir_name);

    ~Archiver();

    void init_dir(std::string_view dir_name, std::string_view root_dir_name);

    void crush(std::string_view out_file_name);

    void add_to_archive(std::vector<std::string_view> &files);

    void add_to_existing_archive(std::vector<std::string_view> &files, std::string_view existing_archive);

    void extract(std::string_view title, std::string_view dest_path);

    bool extract_file(std::string_view title, std::string_view dest_path, uint32_t file_id);

    std::vector<ArchivedFile> extract_files_info(std::string_view title);

    uint32_t extract_files_count(std::string_view title);


private:
    inline void mkdir(fs::path &path);

    inline bool check_stamp(const Stamp &stamp);
};


#endif //ARCHIVEC_CORE_ARCHIVER_HPP
