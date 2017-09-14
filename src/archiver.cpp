#include "archiver.hpp"
#include "util/message.hpp"

Archiver::Archiver() {
    files_ = new std::list<fs::path>();
    titles_ = new std::vector<std::string>();
}

Archiver::Archiver(const char *dir_name) {
    files_ = new std::list<fs::path>();
    titles_ = new std::vector<std::string>();
    init(dir_name);
}

Archiver::~Archiver() {
    delete files_;
    delete titles_;
}

void Archiver::init(const char *dir_name) {
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
        entry.number = i + 1;
        entry.size = fs::file_size(titles_->at(i));
        entry.name_length = titles_->at(i).length();

        //write entry to crushed file
        fwrite(&entry, sizeof(entry), 1, out);

        //write filename
        fwrite(titles_->at(i).c_str(), titles_->at(i).length(), 1, out);

        //open file and read bytes
        uint8_t *contents = new uint8_t[entry.size + 1];
        FILE *in = fopen(titles_->at(i).c_str(), "rb");

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

bool Archiver::check_stamp(const Archiver::Stamp &stamp) {
    return stamp.x == 0x52 && stamp.y == 0x84 && stamp.z == 0x91;
}

uint32_t Archiver::extract_files_count(const char *title) {
    return 0;
}

std::vector<std::string> Archiver::extract_files_names(const char *title) {
    return std::vector<std::string>();
}

bool Archiver::extract_file(const char *title, const uint32_t file_id) {
    return false;
}

void Archiver::extract(const char *title) {

}