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

void Archiver::crush(std::string_view fileName, bool askReplace) {

	if (titles_->empty()) {
		Message::messageBox("Nothing to archivate!", "Message");
		return;
	}

	fs::path outFile(fileName.data());

	//create dir
	mkdir(outFile);

	if (!checkReplace(outFile, askReplace)) return;

	//open out file
	FILE *out = fopen(fileName.data(), "wb");

	if (out == nullptr) {
		Message::messageBox("Couldn't create file\n", "Error", fileName.data());

		fclose(out);
		return;
	}

	//write stamp
	Stamp stamp;
	stamp.files_count = static_cast<uint32_t>(files_->size());
	fwrite(&stamp, sizeof(stamp), 1, out);

	for (uint64_t i = 0; i < titles_->size(); ++i) {
		Entry entry{};
		entry.id = static_cast<uint32_t>(i);
		entry.size = fs::file_size(files_->at(i));
		entry.nameLength = static_cast<uint32_t>(titles_->at(i).length());

		//write entry to crushed file
		fwrite(&entry, sizeof(Entry), 1, out);

		//write filename
		fwrite(titles_->at(i).c_str(), entry.nameLength, 1, out);

		//open file
		FILE *in = fopen(files_->at(i).string().c_str(), "rb");
		if (in == nullptr) {
			Message::messageBox("Couldn't access file\n", "Error", titles_->at(i));

			fclose(in);
			continue;
		}
		//rewrite file bytes to archive \w compress
		rewriteFileCompress(in, out, entry.size, entry.compressedSize);
		fclose(in);

		//write entry to archive
		fseek(out, static_cast<long>(-(entry.compressedSize + entry.nameLength + sizeof(Entry))), SEEK_CUR);
		fwrite(&entry, sizeof(Entry), 1, out);
		fseek(out, static_cast<long>(entry.compressedSize + entry.nameLength), SEEK_CUR);
	}
	fclose(out);
}

void Archiver::add(std::vector<std::string_view> &files) {
	for (auto &&file : files) {
		fs::path path_to_file(file.data());

		if (fs::exists(path_to_file)) {
			if (fs::is_regular_file(path_to_file)) {
				files_->push_back(path_to_file);
				titles_->push_back(path_to_file.filename().string());
			} else {
				initDir(file, path_to_file.parent_path().string() + "/");
			}
		} else {
			Message::messageBox("File or directory is not exists\n", "Warning", file.data());
		}

	}
}

void Archiver::addToExisting(std::vector<std::string_view> &files, std::string_view archive) {
	titles_->clear();
	files_->clear();

	add(files);

	//get last entry id
	uint32_t id = getLastId(archive) + 1;

	//open archive
	FILE *out = fopen(archive.data(), "rw+b");
	if (out == nullptr) {
		if (Message::messageBoxYesNo("file does not exist, create it?\n", "Message", archive.data())) {
			crush(archive);
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

	for (uint64_t i = 0; i < titles_->size(); ++i) {
		Entry entry{};
		entry.id = static_cast<uint32_t>(id + i);
		entry.size = fs::file_size(files_->at(i));
		entry.nameLength = static_cast<uint32_t>(titles_->at(i).length());

		//write entry
		fwrite(&entry, sizeof(Entry), 1, out);
		fwrite(titles_->at(i).data(), titles_->at(i).length(), 1, out);

		//open file
		FILE *in = fopen(files_->at(i).string().c_str(), "rb");

		if (in == nullptr) {
			Message::messageBox("Couldn't access file\n", "Error", titles_->at(i));
			files.pop_back();
			continue;
		}
		rewriteFileCompress(in, out, entry.size, entry.compressedSize);

		fclose(in);

		fseek(out, static_cast<long>(-(entry.compressedSize + entry.nameLength + sizeof(Entry))), SEEK_CUR);
		fwrite(&entry, sizeof(Entry), 1, out);
		fseek(out, static_cast<long>(entry.compressedSize + entry.nameLength + sizeof(Entry)), SEEK_CUR);
	}

	//write new files count
	stamp.files_count += titles_->size();
	fseek(out, 0, SEEK_SET);
	fwrite(&stamp, sizeof(Stamp), 1, out);

	fclose(out);
}

void Archiver::extract(std::string_view archive, std::string_view dest, bool askReplace) {
	FILE *in = fopen(archive.data(), "rb");
	if (in == nullptr) {
		Message::messageBox("Couldn't access file\n", "Error", archive.data());
		exit(EXIT_FAILURE);
	}

	Stamp stamp;
	memset(&stamp, 0, sizeof(Stamp));
	fread(&stamp, sizeof(Stamp), 1, in);

	if (!checkStamp(stamp)) {
		Message::messageBox("Unknown file format!", "Error");
		exit(EXIT_FAILURE);
	}

	//create dir
	fs::path outPath(dest.data());
	mkdir(outPath);

	for (auto i = 0; i < stamp.files_count; i++) {
		Entry entry{};
		memset(&entry, 0, sizeof(Entry));

		fread(&entry, sizeof(Entry), 1, in);

		//file archive
		auto *title = new uint8_t[entry.nameLength + 1];
		fread(title, entry.nameLength, 1, in);
		title[entry.nameLength] = '\0';

		//create file
		fs::path pathToFile(dest.data());
		pathToFile.append(reinterpret_cast<const char *>(title));
		mkdir(pathToFile);

		if (checkReplace(pathToFile, askReplace)) {
			//split file
			FILE *out = fopen(pathToFile.string().c_str(), "wb");
			if (out == nullptr) {
				Message::messageBox("Couldn't write extracted file!", "Error");

				fclose(out);
				fseek(in, static_cast<long>(entry.compressedSize), SEEK_CUR);
				delete[] title;
				continue;
			}
			rewriteFileExpand(in, out, entry.compressedSize, entry.size);

			fclose(out);
		} else {
			fseek(in, static_cast<long>(entry.compressedSize), SEEK_CUR);
		}
		delete[] title;
	}
	fclose(in);
}

bool Archiver::extractFiles(std::string_view archive, std::string_view dest, std::vector<uint32_t> &ids, bool askReplace) {
	FILE *in = fopen(archive.data(), "rb");
	if (in == nullptr) {
		Message::messageBox("Couldn't access file\n", "Error", archive.data());
		exit(EXIT_FAILURE);
	}

	Stamp stamp;
	memset(&stamp, 0, sizeof(Stamp));
	fread(&stamp, sizeof(Stamp), 1, in);

	if (!checkStamp(stamp)) {
		Message::messageBox("Unknown file format!", "Error");
		exit(EXIT_FAILURE);
	}

	bool found = false;
	uint32_t foundFiles = 0;

	for (auto i = 0; i < stamp.files_count; i++) {
		Entry entry{};
		memset(&entry, 0, sizeof(Entry));
		fread(&entry, sizeof(Entry), 1, in);

		for (uint32_t fileId : ids) {
			if (entry.id == fileId) {
				found = true;
				foundFiles++;
				break;
			}
		}

		if (!found) {
			fseek(in, static_cast<long>(entry.nameLength + entry.compressedSize), SEEK_CUR);
			continue;
		}
		found = false;

		//file archive
		auto *title = new uint8_t[entry.nameLength + 1];
		fread(title, entry.nameLength, 1, in);
		title[entry.nameLength] = '\0';

		//create dir
		fs::path out_path(dest.data());
		out_path.append(reinterpret_cast<const char *>(title));
		mkdir(out_path);

		if (checkReplace(out_path, askReplace)) {
			FILE *out = fopen(out_path.string().c_str(), "wb");
			if (out == nullptr) {
				Message::messageBox("Couldn't write extracted file!\n", "Error", out_path.filename().string());

				fseek(in, static_cast<long>(entry.compressedSize), SEEK_CUR);
				fclose(out);
				delete[] title;
				continue;
			}
			rewriteFileExpand(in, out, entry.compressedSize, entry.size);

			fclose(out);
		}
		delete[] title;

		if (foundFiles == ids.size()) {
			break;
		}
	}
	fclose(in);
	return foundFiles != 0;
}

void Archiver::remove(std::vector<uint32_t> &ids, std::string_view archive) {
	FILE *in = fopen(archive.data(), "rb");
	if (in == nullptr) {
		Message::messageBox("Couldn't access file\n", "Error", archive.data());
		exit(EXIT_FAILURE);
	}

	Stamp stamp;
	memset(&stamp, 0, sizeof(Stamp));
	fread(&stamp, sizeof(Stamp), 1, in);

	if (!checkStamp(stamp)) {
		Message::messageBox("Unknown file format!", "Error");

		fclose(in);
		return;
	}

	//create temp archive
	fs::path tempArchive(archive.data());
	tempArchive.replace_extension(".7dck___");
	//remove if exists
	if (fs::exists(tempArchive) && fs::is_regular_file(tempArchive)) {
		fs::remove_all(tempArchive);
	}

	//open to write
	FILE *out = fopen(tempArchive.string().c_str(), "wb");
	if (out == nullptr) {
		Message::messageBox("Couldn't create temp file\n", "Error", tempArchive.filename().string());

		fclose(out);
		return;
	}

	//stamp space
	fseek(out, sizeof(Stamp), SEEK_SET);

	bool found = false;
	uint32_t removedCount = 0;

	for (uint32_t i = 0; i < stamp.files_count; ++i) {
		Entry entry{};
		memset(&entry, 0, sizeof(Entry));
		fread(&entry, sizeof(Entry), 1, in);

		for (uint32_t file_id : ids) {
			if (entry.id == file_id) {
				found = true;
				removedCount++;
				break;
			}
		}

		if (found) {
			found = false;
			fseek(in, static_cast<long>(entry.nameLength + entry.compressedSize), SEEK_CUR);
			continue;
		}

		//get file name and content
		auto *nameAndContent = new uint8_t[entry.nameLength + entry.compressedSize];
		fread(nameAndContent, entry.nameLength + entry.compressedSize, 1, in);

		//write name and content to new archive
		fwrite(&entry, sizeof(Entry), 1, out);
		fwrite(nameAndContent, entry.nameLength + entry.compressedSize, 1, out);

		delete[] nameAndContent;
	}

	fseek(out, 0, SEEK_SET);
	stamp.files_count -= removedCount;
	fwrite(&stamp, sizeof(Stamp), 1, out);

	fclose(in);
	fclose(out);

	//remove old archive & rename new
	fs::remove_all(archive.data());
	fs::rename(tempArchive, archive.data());
}

std::vector<ArchivedFile> Archiver::extractInfo(std::string_view archive) {
	std::vector<ArchivedFile> entries;

	FILE *in = fopen(archive.data(), "rb");
	if (in == nullptr) {
		Message::messageBox("Couldn't access file\n", "Error", archive.data());
		exit(EXIT_FAILURE);
	}

	Stamp stamp;
	memset(&stamp, 0, sizeof(Stamp));
	fread(&stamp, sizeof(Stamp), 1, in);

	if (!checkStamp(stamp)) {
		Message::messageBox("Unknown file format!", "Error");
		exit(EXIT_FAILURE);
	}

	for (uint32_t i = 0; i < stamp.files_count; i++) {
		Entry entry{};
		memset(&entry, 0, sizeof(Entry));
		fread(&entry, sizeof(Entry), 1, in);

		//get file name
		auto *name = new uint8_t[entry.nameLength + 1];
		fread(name, entry.nameLength, 1, in);
		name[entry.nameLength] = '\0';

		//create archived entry
		ArchivedFile file{};
		file.id = entry.id;
		file.size = entry.size;
		file.compressedSize = entry.compressedSize;
		file.name = name;

		entries.push_back(file);

		fseek(in, static_cast<long>(entry.compressedSize), SEEK_CUR);
	}

	fclose(in);
	return entries;
}

uint32_t Archiver::getLastId(std::string_view path) {
	auto files = extractInfo(path);
	return files[files.size() - 1].id;
}

void Archiver::setBufferSize(uint32_t new_size) {
	maxBufferSize = new_size;
}

void Archiver::reset() {
	titles_->clear();
	files_->clear();
}

//private

void Archiver::initDir(std::string_view dirName, std::string_view rootDirName) {
	fs::path dir(dirName.data());

	if (!fs::exists(dir) || !fs::is_directory(dir)) {
		Message::messageBox("Is not a directory\n", "Error", dirName.data());
		return;
	}

	auto iter = fs::directory_iterator(dir);
	auto end = fs::directory_iterator();

	while (iter != end) {
		if (!fs::is_regular_file(iter->path())) {
			initDir(iter->path().string(), rootDirName);
			++iter;
			continue;
		}

		files_->push_back(iter->path());

		auto full_path = iter->path().string();
		auto path_from_root = full_path.substr(rootDirName.length(), full_path.length());

		titles_->push_back(path_from_root);
		++iter;
	}
}

void Archiver::mkdir(fs::path &path) {
	//create directory
	if (path.branch_path() != "") {

		if (fs::exists(path.branch_path())) return;

		if (!fs::create_directories(path.branch_path())) {
			Message::messageBox("Couldn't create directory\n", "Error", path.branch_path().string());
			exit(EXIT_FAILURE);
		}
	}
}

bool Archiver::checkStamp(const Stamp &stamp) {
	return stamp.x == 0x52 && stamp.y == 0x84 && stamp.z == 0x91;
}

bool Archiver::checkReplace(fs::path &path, bool askReplace) {
	if (fs::exists(path) && fs::is_regular_file(path)) {
		if (askReplace) {
			if (Message::messageBoxYesNo("File is already exists! Replace it?\n", "Message", path.filename().string())) {
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

void Archiver::rewriteFileExpand(FILE *in, FILE *out, const uint64_t &compressedSize, const uint64_t &fileSize) {
	uint32_t compressedSizeBytes;
	uint32_t compressedSizeBits;
	uint64_t bytesRead = 0;

	std::vector<std::uint8_t> *uncompressedBuffer = nullptr;

	while (bytesRead != compressedSize) {
		//read size
		fread(&compressedSizeBytes, sizeof(uint32_t), 1, in);
		fread(&compressedSizeBits, sizeof(uint32_t), 1, in);

		//create cluster
		auto *buffer = new uint8_t[compressedSizeBytes];
		fread(buffer, compressedSizeBytes, 1, in);

		bytesRead += compressedSizeBytes + sizeof(uint32_t) * 2;

		//uncompress and write
		uncompressedBuffer = new std::vector<uint8_t>(fileSize, 0);
		const uint32_t uncompressedSize = lzw::expand(buffer, compressedSizeBytes, compressedSizeBits,
		                                              uncompressedBuffer->data());
		fwrite(uncompressedBuffer->data(), uncompressedSize, 1, out);

		delete[] buffer;
	}
	delete uncompressedBuffer;
}

void Archiver::rewriteFileCompress(FILE *in, FILE *out, const uint64_t &fileSize, uint64_t &compressedSize) {
	uint32_t compressedSizeBytes;
	uint32_t compressedSizeBits;
	uint8_t *compressedBuffer = nullptr;

	uint8_t *buffer;
	uint64_t restSize = fileSize;
	compressedSize = 0;

	if (fileSize > maxBufferSize) {
		//split file
		auto clusters = static_cast<uint32_t>(fileSize / maxBufferSize);
		restSize = fileSize % maxBufferSize;

		//create cluster
		buffer = new uint8_t[maxBufferSize];
		for (int i = 0; i < clusters; ++i) {
			//read and compress
			fread(buffer, maxBufferSize, 1, in);
			lzw::compress(buffer, maxBufferSize, &compressedBuffer, &compressedSizeBytes, &compressedSizeBits);

			//write compressed buffer
			fwrite(&compressedSizeBytes, sizeof(uint32_t), 1, out);
			fwrite(&compressedSizeBits, sizeof(uint32_t), 1, out);
			fwrite(compressedBuffer, compressedSizeBytes, 1, out);

			compressedSize += compressedSizeBytes + sizeof(uint32_t) * 2;
		}
		delete[] buffer;
	}
	//rest file part
	buffer = new uint8_t[restSize];
	//read and compress
	fread(buffer, restSize, 1, in);
	lzw::compress(buffer, static_cast<uint32_t>(restSize), &compressedBuffer, &compressedSizeBytes, &compressedSizeBits);

	//write compressed
	fwrite(&compressedSizeBytes, sizeof(uint32_t), 1, out);
	fwrite(&compressedSizeBits, sizeof(uint32_t), 1, out);
	fwrite(compressedBuffer, compressedSizeBytes, 1, out);

	compressedSize += compressedSizeBytes + sizeof(uint32_t) * 2;

	delete[] buffer;
}
