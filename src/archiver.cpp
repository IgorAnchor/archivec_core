#include "archiver.hpp"
#include "util/message.hpp"

Archiver::Archiver() {
    files_ = new std::vector<fs::path>();
    titles_ = new std::vector<std::string>();
}

Archiver::Archiver(const char *dir_name) {
    files_ = new std::vector<fs::path>();
    titles_ = new std::vector<std::string>();
    init(dir_name);
}

Archiver::~Archiver() {
    delete files_;
    delete titles_;
}

void Archiver::init(const char *dir_name) {
    root_dir = dir_name;
    fs::path dir(dir_name);

    if (!fs::exists(dir) || !fs::is_directory(dir)) {
        Message::message_box("Is not a directory ", "Error", 1, &dir_name);
        exit(EXIT_FAILURE);
    }

    auto iter = fs::directory_iterator(dir);
    auto end = fs::directory_iterator();

    while (iter != end) {
        if (!fs::is_regular_file(iter->path())) {
            ++iter;
            continue;
        }

        files_->push_back(iter->path());

        auto title = std::string(iter->path().stem().string() + iter->path().extension().string());

        titles_->push_back(title);
        ++iter;
    }
}

void Archiver::crush(const char *out_file_name) {

    fs::path out_path(out_file_name);

    //create dir
    mkdir(out_path);

    FILE *out = fopen(out_file_name, "wb");

    if (out == nullptr) {
        Message::message_box("Couldn't create file ", "Error", 1, &out_file_name);
        exit(EXIT_FAILURE);
    }

    Stamp stamp;
    stamp.files_count = files_->size();

    //write stamp
    fwrite(&stamp, sizeof(stamp), 1, out);

    //archiving
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

        //split file

        //open file and read
        uint8_t *contents = new uint8_t[entry.size + 1];
        FILE *in = fopen(files_->at(i).string().c_str(), "rb");

        if (in == nullptr) {
            const char *title = titles_->at(i).c_str();
            Message::message_box("Couldn't access file ", "Error", 1, &title);
            exit(EXIT_FAILURE);
        }

        std::cout << "\t->" << titles_->at(i).c_str() << std::endl;
        for (auto j = 0; j < entry.size; j++) {
            fread(contents + j, sizeof(uint8_t), 1, in);
            fwrite(contents + j, sizeof(uint8_t), 1, out);
        }

        delete[] contents;
        fclose(in);
    }
    fclose(out);

}

bool Archiver::check_stamp(const Stamp &stamp) {
    return stamp.x == 0x52 && stamp.y == 0x84 && stamp.z == 0x91;
}

bool Archiver::extract_file(const char *title, const char *dest_path, const uint32_t file_id) {
    bool found = false;

    FILE *in = fopen(title, "rb");

    if (in == nullptr) {
        Message::message_box("Couldn't access file ", "Error", 1, &title);
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

        uint8_t *title = new uint8_t[entry.name_length + 1];

        if (entry.id != file_id) {
            fseek(in, entry.name_length + entry.size, SEEK_CUR);
            continue;
        }

        uint8_t *contents = new uint8_t[entry.size + 1];

        for (auto j = 0; j < entry.name_length; j++) {
            fread(title + j, sizeof(uint8_t), 1, in);
        }
        title[entry.name_length] = '\0';

        std::cout << "\t->" << title << std::endl;

        //create dir
        fs::path out_path = dest_path;
        mkdir(out_path);

        //split file
        FILE *out = fopen(dest_path, "wb");
        if (out == nullptr) {
            Message::message_box("Couldn't write extracted file!", "Error");
            exit(EXIT_FAILURE);
        }

        for (auto j = 0; j < entry.size; j++) {
            fread(contents + j, sizeof(uint8_t), 1, in);
            fwrite(contents + j, sizeof(uint8_t), 1, out);
        }
        contents[entry.size] = '\0';

        delete[] contents;
        delete[] title;
        fclose(out);
        found = true;
        break;
    }

    fclose(in);

    return found ? found : false;
}

void Archiver::extract(const char *title, const char *dest_path) {

    FILE *in = fopen(title, "rb");

    if (in == nullptr) {
        Message::message_box("Couldn't access file ", "Error", 1, &title);
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
    fs::path out_path = dest_path;
    mkdir(out_path);

    std::cout << "Extracting: " << std::endl;
    for (auto i = 0; i < stamp.files_count; i++) {
        Entry entry;
        memset(&entry, 0, sizeof(Entry));

        fread(&entry, sizeof(Entry), 1, in);

        uint8_t *title = new uint8_t[entry.name_length + 1];
        uint8_t *contents = new uint8_t[entry.size + 1];

        for (auto j = 0; j < entry.name_length; j++) {
            fread(title + j, sizeof(uint8_t), 1, in);
        }
        title[entry.name_length] = '\0';

        std::cout << "\t->" << title << std::endl;


        std::string path_file = dest_path;
        path_file.append(reinterpret_cast<const char *>(title));

        //split file
        FILE *out = fopen(path_file.c_str(), "wb");
        if (out == nullptr) {
            Message::message_box("Couldn't write extracted file!", "Error");
            exit(EXIT_FAILURE);
        }

        for (auto j = 0; j < entry.size; j++) {
            fread(contents + j, sizeof(uint8_t), 1, in);
            fwrite(contents + j, sizeof(uint8_t), 1, out);
        }
        contents[entry.size] = '\0';

        delete[] contents;
        delete[] title;
        fclose(out);
    }

    fclose(in);
}

std::vector<ArchivedFile> Archiver::extract_files_info(const char *title) {
    std::vector<ArchivedFile> entries;

    FILE *in = fopen(title, "rb");

    if (in == nullptr) {
        Message::message_box("Couldn't access file ", "Error", 1, &title);
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
        file.name = reinterpret_cast<char *>(name);

        entries.push_back(file);

        fseek(in, entry.size, SEEK_CUR);
    }

    fclose(in);
    return entries;
}


uint32_t Archiver::extract_files_count(const char *title) {
    FILE *in = fopen(title, "r");
    if (in == nullptr) {
        Message::message_box("Couldn't access file ", "Error", 1, &title);
        exit(EXIT_FAILURE);
    }

    Stamp stamp;

    memset(&stamp, 0, sizeof(Stamp));
    fread(&stamp, sizeof(Stamp), 1, in);

    if (!check_stamp(stamp)) {
        Message::message_box("Unknown file format!", "Error");
        exit(EXIT_FAILURE);
    }

    return stamp.files_count;
}


void Archiver::mkdir(fs::path &path) {
    //create directory
    if (path.branch_path() != "") {
        if (!fs::create_directories(path.branch_path())) {
            const char *dir_path = path.branch_path().string().c_str();
            Message::message_box("Couldn't create directory ", "Error", 1, &dir_path);
            exit(EXIT_FAILURE);
        }
    }
}