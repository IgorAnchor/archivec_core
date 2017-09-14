#ifndef ARCHIVEC_CORE_ARCHIVER_HPP
#define ARCHIVEC_CORE_ARCHIVER_HPP

#include <string>
#include <list>

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

class Archiver {

private:
    struct Stamp {
        uint8_t x = 0x52;
        uint8_t y = 0x84;
        uint8_t z = 0x91;
        uint32_t files_count;
    } __attribute__((__packed__));

    struct Entry {
        uint32_t number;        //order n
        uint64_t size;          //file size
        uint32_t name_length;   //path+extension
    } __attribute__((__packed__));

    std::vector<fs::path> *files_;
    std::vector<std::string> *titles_;

    std::string root_dir;
public:
    explicit Archiver();

    explicit Archiver(const char *dir_name);

    ~Archiver();

    void init(const char *dir_name);

    void crush(const char *out_file_name);

    inline bool check_stamp(const Stamp &stamp);

    uint32_t extract_files_count(const char *title);

    std::vector<std::string> extract_files_names(const char *title);

    bool extract_file(const char *title, const char *dest_path, const uint32_t file_id);

    void extract(const char *title, const char *dest_path);

private:
    inline void mkdir(fs::path &path);
};


#endif //ARCHIVEC_CORE_ARCHIVER_HPP
