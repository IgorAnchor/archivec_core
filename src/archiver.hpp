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

struct ArchivedFile {
    uint32_t id;
    uint64_t size;
    uint8_t *name;
} __attribute__((__packed__));

class Archiver {

private:
    std::vector<fs::path> *files_;
    std::vector<std::string> *titles_;

    uint32_t max_buffer_size = 20'000'000;
public:
    explicit Archiver();

    ~Archiver();

    void crush(std::string_view out_file_name, bool ask_replace = true);

    void add_to_archive(std::vector<std::string_view> &files);

    void add_to_existing_archive(std::vector<std::string_view> &file_paths, std::string_view existing_archive);

    void extract(std::string_view title, std::string_view dest_path);

    bool extract_files(std::string_view path_to_archive, std::string_view dest_path, std::vector<uint32_t> &file_ids);

    std::vector<ArchivedFile> extract_files_info(std::string_view title);

    void remove_from_archive(std::vector<uint32_t> &file_ids, std::string_view archive_path);

    void set_buffer_size(uint32_t new_size);

    void reset();

    uint32_t get_last_id(std::string_view path_to_archive);

private:
    void init_dir(std::string_view dir_name, std::string_view root_dir_name);

    inline void mkdir(fs::path &path);

    inline bool check_stamp(const Stamp &stamp);

    inline bool check_replace(fs::path &path, bool ask_replace = true);

    inline void rewrite_file(FILE *in, FILE *out, uint64_t file_size);
};


#endif //ARCHIVEC_CORE_ARCHIVER_HPP
