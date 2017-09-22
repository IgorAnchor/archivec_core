#include <sstream>
#include "archiver.hpp"
#include "util/messagebox.hpp"

Archiver::Archiver() {
    files_ = new std::vector<fs::path>();
    titles_ = new std::vector<std::string>();
}

Archiver::~Archiver() {
    delete files_;
    delete titles_;
}

void Archiver::init_dir(std::string_view dir_name, std::string_view root_dir_name) {
    fs::path dir(dir_name.data());

    if (!fs::exists(dir) || !fs::is_directory(dir)) {
        Message::message_box("Is not a directory ", "Error", dir_name.data());
        return;
    }

    auto iter = fs::directory_iterator(dir);
    auto end = fs::directory_iterator();

    while (iter != end) {
        if (!fs::is_regular_file(iter->path())) {
            init_dir(iter->path().string(), root_dir_name);
            ++iter;
            continue;
        }

        files_->push_back(iter->path());

        auto full_path = iter->path().string();
        auto path_from_root = full_path.substr(root_dir_name.length(), full_path.length());

        titles_->push_back(path_from_root);
        ++iter;
    }
}

void Archiver::crush(std::string_view out_file_name, bool ask_replace) {

    if (titles_->size() <= 0) {
        Message::message_box("Nothing to archivate!", "Message");
        return;
    }

    fs::path out_path(out_file_name.data());

    //create dir
    mkdir(out_path);

    if (!check_replace(out_path, ask_replace)) return;

    FILE *out = fopen(out_file_name.data(), "wb");

    if (out == nullptr) {
        Message::message_box("Couldn't create file ", "Error", out_file_name.data());

        fclose(out);
        return;
    }

    //write stamp
    Stamp stamp;
    stamp.files_count = files_->size();
    fwrite(&stamp, sizeof(stamp), 1, out);

    std::cout << "Archiving: " << std::endl;
    for (auto i = 0; i < titles_->size(); ++i) {
        Entry entry;
        entry.id = i;
        entry.size = fs::file_size(files_->at(i));
        entry.name_length = titles_->at(i).length();

        //write entry to crushed file
        fwrite(&entry, sizeof(Entry), 1, out);

        //write filename
        fwrite(titles_->at(i).c_str(), titles_->at(i).length(), 1, out);

        //open file
        FILE *in = fopen(files_->at(i).string().c_str(), "rb");
        if (in == nullptr) {
            Message::message_box("Couldn't access file ", "Error", titles_->at(i));

            fclose(in);
            continue;
        }

        std::cout << "\t->" << titles_->at(i) << std::endl;

        rewrite_file(in, out, entry.size);

        fclose(in);
    }
    fclose(out);
}


void Archiver::add_to_existing_archive(std::vector<std::string_view> &file_paths, std::string_view existing_archive) {

    //id
    uint32_t id = get_last_id(existing_archive) + 1;

    FILE *out = fopen(existing_archive.data(), "rw+b");

    titles_->clear();
    files_->clear();
    add_to_archive(file_paths);

    if (out == nullptr) {
        if (Message::message_box_yes_no("file does not exist, create it?\n", "Message", existing_archive.data())) {
            crush(existing_archive);
            return;
        } else {
            fclose(out);
            return;
        }
    }

    //read stamp
    Stamp stamp;
    memset(&stamp, 0, sizeof(Stamp));
    fread(&stamp, sizeof(Stamp), 1, out);

    fseek(out, 0, SEEK_END);


    for (int i = 0; i < titles_->size(); ++i) {
        Entry entry;
        entry.id = id + i;
        entry.size = fs::file_size(files_->at(i));
        entry.name_length = titles_->at(i).length();

        fwrite(&entry, sizeof(Entry), 1, out);
        fwrite(titles_->at(i).data(), titles_->at(i).length(), 1, out);

        FILE *in = fopen(files_->at(i).string().c_str(), "rb");

        if (in == nullptr) {
            Message::message_box("Couldn't access file ", "Error", titles_->at(i));
            file_paths.pop_back();
            continue;
        }

        std::cout << "\t->" << titles_->at(i) << std::endl;

        rewrite_file(in, out, entry.size);

        fclose(in);
    }

    //write new files count
    stamp.files_count += titles_->size();
    fseek(out, 0, SEEK_SET);
    fwrite(&stamp, sizeof(Stamp), 1, out);

    fclose(out);
}

void Archiver::add_to_archive(std::vector<std::string_view> &files) {
    for (auto &&file : files) {
        fs::path path_to_file(file.data());

        if (fs::exists(path_to_file)) {
            if (fs::is_regular_file(path_to_file)) {
                files_->push_back(path_to_file);
                titles_->push_back(path_to_file.filename().string());
            } else {
                init_dir(file, path_to_file.parent_path().string() + "/");
            }
        } else {
            Message::message_box("File or directory is not exists ", "Warning", file.data());
        }

    }
}

bool Archiver::check_stamp(const Stamp &stamp) {
    return stamp.x == 0x52 && stamp.y == 0x84 && stamp.z == 0x91;
}

bool
Archiver::extract_files(std::string_view path_to_archive, std::string_view dest_path, std::vector<uint32_t> &file_ids) {
    FILE *in = fopen(path_to_archive.data(), "rb");

    if (in == nullptr) {
        Message::message_box("Couldn't access file ", "Error", path_to_archive.data());
        exit(EXIT_FAILURE);
    }

    Stamp stamp;
    memset(&stamp, 0, sizeof(Stamp));
    fread(&stamp, sizeof(Stamp), 1, in);

    if (!check_stamp(stamp)) {
        Message::message_box("Unknown file format!", "Error");
        exit(EXIT_FAILURE);
    }

    bool found = false;
    uint32_t found_files = 0;

    std::cout << "Extracting: " << std::endl;
    for (auto i = 0; i < stamp.files_count; i++) {
        Entry entry;
        memset(&entry, 0, sizeof(Entry));
        fread(&entry, sizeof(Entry), 1, in);

        for (uint32_t file_id : file_ids) {
            if (entry.id == file_id) {
                found = true;
                found_files++;
                break;
            }
        }

        if (!found) {
            fseek(in, entry.name_length + entry.size, SEEK_CUR);
            continue;
        }
        found = false;

        //file path_to_archive
        uint8_t *title = new uint8_t[entry.name_length + 1];
        fread(title, entry.name_length, 1, in);
        title[entry.name_length] = '\0';

        std::cout << "\t->" << title << std::endl;

        //create dir
        fs::path out_path(dest_path.data());
        out_path.append(reinterpret_cast<const char *>(title));
        mkdir(out_path);

        if (check_replace(out_path)) {

            //split file
            FILE *out = fopen(out_path.string().c_str(), "wb");
            if (out == nullptr) {
                Message::message_box("Couldn't write extracted file!\n", "Error", out_path.filename().string());

                fseek(in, entry.size, SEEK_CUR);
                fclose(out);
                delete[] title;
                continue;
            }

            rewrite_file(in, out, entry.size);

            fclose(out);
        }
        delete[] title;

        if (found_files == file_ids.size()) {
            break;
        }
    }
    fclose(in);
    return found_files != 0;
}

void Archiver::extract(std::string_view title, std::string_view dest_path) {

    FILE *in = fopen(title.data(), "rb");

    if (in == nullptr) {
        Message::message_box("Couldn't access file ", "Error", title.data());
        exit(EXIT_FAILURE);
    }

    Stamp stamp;
    memset(&stamp, 0, sizeof(Stamp));
    fread(&stamp, sizeof(Stamp), 1, in);

    if (!check_stamp(stamp)) {
        Message::message_box("Unknown file format!", "Error");
        exit(EXIT_FAILURE);
    }

    //create dir
    fs::path out_path(dest_path.data());
    mkdir(out_path);

    std::cout << "Extracting: " << std::endl;
    for (auto i = 0; i < stamp.files_count; i++) {
        Entry entry;
        memset(&entry, 0, sizeof(Entry));

        fread(&entry, sizeof(Entry), 1, in);

        //file title
        uint8_t *title = new uint8_t[entry.name_length + 1];
        fread(title, entry.name_length, 1, in);
        title[entry.name_length] = '\0';

        std::cout << "\t->" << title << std::endl;

        fs::path path_to_file(dest_path.data());
        path_to_file.append(reinterpret_cast<const char *>(title));

        mkdir(path_to_file);

        if (check_replace(path_to_file)) {
            //split file
            FILE *out = fopen(path_to_file.string().c_str(), "wb");
            if (out == nullptr) {
                Message::message_box("Couldn't write extracted file!", "Error");

                fclose(out);
                fseek(in, entry.size, SEEK_CUR);
                delete[] title;
                continue;
            }

            rewrite_file(in, out, entry.size);

            fclose(out);
        } else {
            fseek(in, entry.size, SEEK_CUR);
        }
        delete[] title;
    }

    fclose(in);
}

std::vector<ArchivedFile> Archiver::extract_files_info(std::string_view title) {
    std::vector<ArchivedFile> entries;

    FILE *in = fopen(title.data(), "rb");

    if (in == nullptr) {
        Message::message_box("Couldn't access file ", "Error", title.data());
        exit(EXIT_FAILURE);
    }

    Stamp stamp;
    memset(&stamp, 0, sizeof(Stamp));

    fread(&stamp, sizeof(Stamp), 1, in);

    if (!check_stamp(stamp)) {
        Message::message_box("Unknown file format!", "Error");
        exit(EXIT_FAILURE);
    }


    for (auto i = 0; i < stamp.files_count; i++) {
        Entry entry;
        memset(&entry, 0, sizeof(Entry));
        fread(&entry, sizeof(Entry), 1, in);

        uint8_t *name = new uint8_t[entry.name_length + 1];
        fread(name, entry.name_length, 1, in);
        name[entry.name_length] = '\0';

        ArchivedFile file;
        file.id = entry.id;
        file.size = entry.size;
        file.name = name;

        entries.push_back(file);

        fseek(in, entry.size, SEEK_CUR);
    }

    fclose(in);
    return entries;
}

void Archiver::remove_from_archive(std::vector<uint32_t> &file_ids, std::string_view archive_path) {
    FILE *in = fopen(archive_path.data(), "rb");
    if (in == nullptr) {
        Message::message_box("Couldn't access file ", "Error", archive_path.data());
        exit(EXIT_FAILURE);
    }

    Stamp stamp;
    memset(&stamp, 0, sizeof(Stamp));
    fread(&stamp, sizeof(Stamp), 1, in);

    if (!check_stamp(stamp)) {
        Message::message_box("Unknown file format!", "Error");

        fclose(in);
        return;
    }

    fs::path temp_archive(archive_path.data());
    temp_archive.replace_extension(".7dck___");

    if (fs::exists(temp_archive) && fs::is_regular_file(temp_archive)) {
        fs::remove_all(temp_archive);
    }

    FILE *out = fopen(temp_archive.string().c_str(), "wb");

    if (out == nullptr) {
        Message::message_box("Couldn't create temp file\n", "Error", temp_archive.filename().string());

        fclose(out);
        return;
    }

    //stamp space
    fseek(out, sizeof(Stamp), SEEK_SET);

    bool found = false;
    uint32_t removed_count = 0;

    std::cout << "Removing..." << std::endl;
    for (int i = 0; i < stamp.files_count; ++i) {
        Entry entry;
        memset(&entry, 0, sizeof(Entry));
        fread(&entry, sizeof(Entry), 1, in);

        for (uint32_t file_id : file_ids) {
            if (entry.id == file_id) {
                found = true;
                removed_count++;
                break;
            }
        }

        if (found) {
            found = false;
            fseek(in, entry.name_length + entry.size, SEEK_CUR);
            continue;
        }

        uint8_t *name_and_content = new uint8_t[entry.name_length + entry.size];
        fread(name_and_content, entry.name_length + entry.size, 1, in);

        fwrite(&entry, sizeof(Entry), 1, out);
        fwrite(name_and_content, entry.name_length + entry.size, 1, out);

        delete[] name_and_content;
    }

    fseek(out, 0, SEEK_SET);
    stamp.files_count -= removed_count;
    fwrite(&stamp, sizeof(Stamp), 1, out);

    fclose(in);
    fclose(out);

    fs::remove_all(archive_path.data());
    fs::rename(temp_archive, archive_path.data());
}


uint32_t Archiver::get_last_id(std::string_view path_to_archive) {

    auto files = extract_files_info(path_to_archive);

    return files[files.size() - 1].id;
}

void Archiver::mkdir(fs::path &path) {
    //create directory
    if (path.branch_path() != "") {

        if (fs::exists(path.branch_path())) return;

        if (!fs::create_directories(path.branch_path())) {
            Message::message_box("Couldn't create directory ", "Error", path.branch_path().string());
            exit(EXIT_FAILURE);
        }
    }
}

bool Archiver::check_replace(fs::path &path, bool ask_replace) {
    if (fs::exists(path) && fs::is_regular_file(path)) {
        if (ask_replace) {
            if (Message::message_box_yes_no("File is already exists! Replace it?\n", "Message",
                                            path.filename().string())) {
                fs::remove_all(path);
                return true;
            }
            return false;
        } else {
            fs::remove_all(path);
            return true;
        }
    }
    return true;
}

void Archiver::rewrite_file(FILE *in, FILE *out, uint64_t file_size) {
    uint8_t *buffer;

    uint64_t rest_size = file_size;

    if (file_size > max_buffer_size) {
        uint64_t clusters = file_size / max_buffer_size;
        rest_size = file_size % max_buffer_size;

        buffer = new uint8_t[max_buffer_size];
        for (int i = 0; i < clusters; ++i) {
            fread(buffer, max_buffer_size, 1, in);
            fwrite(buffer, max_buffer_size, 1, out);

        }
        delete[] buffer;
    }

    buffer = new uint8_t[rest_size];
    fread(buffer, rest_size, 1, in);
    fwrite(buffer, rest_size, 1, out);
    delete[] buffer;
}

void Archiver::set_buffer_size(uint32_t new_size) {
    max_buffer_size = new_size;
}

void Archiver::reset() {
    titles_->clear();
    files_->clear();
}