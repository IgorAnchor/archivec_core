#include <sstream>

#include "archiver.hpp"
#include "util/messagebox.hpp"
#include "util/util.hpp"

Archiver::Archiver() {
	files = new std::vector<fs::path>();
	titles = new std::vector<std::wstring>();
}

Archiver::~Archiver() {
	delete files;
	delete titles;
}

void Archiver::crush(std::wstring_view fileName, bool askReplace) {
	if (titles->empty()) {
		Message::messageBox(L"Nothing to archivate!", L"Message");
		return;
	}

	fs::path outFile(fileName.data());

	//create dir
	mkdir(outFile);

	if (!checkReplace(outFile, askReplace)) return;

	//open out file
	FILE *out = _wfopen(fileName.data(), L"wb");

	if (!out) {
		Message::messageBox(L"Couldn't create file\n", L"Error", fileName.data());

		fclose(out);
		return;
	}

	//write stamp
	Stamp stamp;
	stamp.filesCount = static_cast<unsigned>(files->size());
	fwrite(&stamp, sizeof(stamp), 1, out);

	for (uint64_t i = 0; i < titles->size(); ++i) {
		Entry entry{};
		entry.id = static_cast<unsigned>(i);
		entry.size = fs::file_size(files->at(i));
		entry.nameLength = static_cast<unsigned>(titles->at(i).length() * 2);

		//write entry to crushed file
		fwrite(&entry, sizeof(Entry), 1, out);

		//write filename
		fwrite(titles->at(i).c_str(), entry.nameLength, 1, out);


		//open file
		FILE *in = _wfopen(files->at(i).wstring().c_str(), L"rb");
		if (!in) {
			Message::messageBox(L"Couldn't access file\n", L"Error", titles->at(i));

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

void Archiver::add(std::vector<std::wstring_view> &files) {
	for (auto &&file : files) {
		fs::path pathToFile(file.data());

		if (fs::exists(pathToFile)) {
			if (fs::is_regular_file(pathToFile)) {
				this->files->push_back(pathToFile);
				titles->push_back(pathToFile.filename().wstring());
			} else {
				initDir(file, pathToFile.parent_path().wstring() + L"/");
			}
		} else {
			Message::messageBox(L"File or directory is not exists\n", L"Warning", file.data());
		}

	}
}

void Archiver::addToExisting(std::vector<std::wstring_view> &files, std::wstring_view archive) {
	this->titles->clear();
	this->files->clear();

	add(files);

	//get last entry id
	unsigned id = getLastId(archive) + 1;

	//open archive
	FILE *out = _wfopen(archive.data(), L"rb+");

	if (!out) {
		fclose(out);
		if (Message::messageBoxYesNo(L"File does not exist, create it?\n", L"Message", archive.data())) {
			crush(archive);
			return;
		}
		return;
	}

	//read stamp
	Stamp stamp;
	memset(&stamp, 0, sizeof(Stamp));
	fread(&stamp, sizeof(Stamp), 1, out);

	fseek(out, 0, SEEK_END);

	for (uint64_t i = 0; i < titles->size(); ++i) {
		Entry entry{};
		entry.id = static_cast<unsigned>(id + i);
		entry.size = fs::file_size(this->files->at(i));
		entry.nameLength = static_cast<unsigned>(titles->at(i).length() * 2);

		//write entry
		fwrite(&entry, sizeof(Entry), 1, out);
		fwrite(titles->at(i).data(), entry.nameLength, 1, out);

		//open file
		FILE *in = _wfopen(this->files->at(i).wstring().c_str(), L"rb");

		if (in == nullptr) {
			Message::messageBox(L"Couldn't access file\n", L"Error", titles->at(i));
			files.pop_back();
			continue;
		}
		rewriteFileCompress(in, out, entry.size, entry.compressedSize);

		fclose(in);

		fseek(out, static_cast<long>(-(entry.compressedSize + entry.nameLength + sizeof(Entry))), SEEK_CUR);
		fwrite(&entry, sizeof(Entry), 1, out);
		fseek(out, static_cast<long>(entry.compressedSize + entry.nameLength), SEEK_CUR);
	}

	//write new files count
	stamp.filesCount += titles->size();
	fseek(out, 0, SEEK_SET);
	fwrite(&stamp, sizeof(Stamp), 1, out);

	fclose(out);
}

void Archiver::extract(std::wstring_view archive, std::wstring_view dest, bool askReplace) {
	FILE *in = _wfopen(archive.data(), L"rb");
	if (in == nullptr) {
		Message::messageBox(L"Couldn't access file\n", L"Error", archive.data());
		exit(EXIT_FAILURE);
	}

	Stamp stamp;
	memset(&stamp, 0, sizeof(Stamp));
	fread(&stamp, sizeof(Stamp), 1, in);

	if (!checkStamp(stamp)) {
		Message::messageBox(L"Unknown file format!", L"Error");
		exit(EXIT_FAILURE);
	}

	//create dir
	fs::path outPath(dest.data());
	mkdir(outPath);

	for (auto i = 0; i < stamp.filesCount; i++) {
		Entry entry{};
		memset(&entry, 0, sizeof(Entry));
		fread(&entry, sizeof(Entry), 1, in);

		//file archive
		auto *title = new wchar_t[(entry.nameLength / 2) + 1];
		fread(title, entry.nameLength, 1, in);
		title[entry.nameLength / 2] = '\0';

		//create file
		fs::path pathToFile(dest.data());
		pathToFile.append(title);
		mkdir(pathToFile);

		if (checkReplace(pathToFile, askReplace)) {
			//split file
			FILE *out = _wfopen(pathToFile.wstring().c_str(), L"wb");
			if (out == nullptr) {
				Message::messageBox(L"Couldn't write extracted file!", L"Error");

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

bool Archiver::extractFiles(std::wstring_view archive, std::wstring_view dest, std::vector<unsigned> &ids, bool askReplace,
                            bool fullPath) {
	FILE *in = _wfopen(archive.data(), L"rb");
	if (in == nullptr) {
		Message::messageBox(L"Couldn't access file\n", L"Error", archive.data());
		exit(EXIT_FAILURE);
	}

	Stamp stamp;
	memset(&stamp, 0, sizeof(Stamp));
	fread(&stamp, sizeof(Stamp), 1, in);

	if (!checkStamp(stamp)) {
		Message::messageBox(L"Unknown file format!", L"Error");
		exit(EXIT_FAILURE);
	}

	bool found = false;
	unsigned foundFiles = 0;

	for (auto i = 0; i < stamp.filesCount; i++) {
		Entry entry{};
		memset(&entry, 0, sizeof(Entry));
		fread(&entry, sizeof(Entry), 1, in);

		for (unsigned fileId : ids) {
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
		auto *title = new wchar_t[(entry.nameLength / 2) + 1];
		fread(title, entry.nameLength, 1, in);
		title[entry.nameLength / 2] = '\0';

		const wchar_t *filePath;

		if (fullPath) {
			filePath = title;
		} else {
			auto name = util::split(title, fs::path::preferred_separator);
			filePath = name.back().c_str();
		}
		//create dir
		fs::path outPath(dest.data());
		//is full path
		outPath.append(filePath);
		mkdir(outPath);

		if (checkReplace(outPath, askReplace)) {
			FILE *out = _wfopen(outPath.wstring().c_str(), L"wb");
			if (!out) {
				Message::messageBox(L"Couldn't write extracted file!\n", L"Error", outPath.filename().wstring());

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

void Archiver::remove(std::vector<unsigned> &ids, std::wstring_view archive) {
	FILE *in = _wfopen(archive.data(), L"rb");
	if (in == nullptr) {
		Message::messageBox(L"Couldn't access file\n", L"Error", archive.data());
		exit(EXIT_FAILURE);
	}

	Stamp stamp;
	memset(&stamp, 0, sizeof(Stamp));
	fread(&stamp, sizeof(Stamp), 1, in);

	if (!checkStamp(stamp)) {
		Message::messageBox(L"Unknown file format!", L"Error");

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
		Message::messageBox(L"Couldn't create temp file\n", L"Error", tempArchive.filename().wstring());

		fclose(out);
		return;
	}

	//stamp space
	fseek(out, sizeof(Stamp), SEEK_SET);

	bool found = false;
	unsigned removedCount = 0;

	for (unsigned i = 0; i < stamp.filesCount; ++i) {
		Entry entry{};
		memset(&entry, 0, sizeof(Entry));
		fread(&entry, sizeof(Entry), 1, in);

		for (unsigned file_id : ids) {
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
		auto *nameAndContent = new wchar_t[(entry.nameLength / 2) + entry.compressedSize];
		fread(nameAndContent, entry.nameLength + entry.compressedSize, 1, in);

		//write name and content to new archive
		fwrite(&entry, sizeof(Entry), 1, out);
		fwrite(nameAndContent, entry.nameLength + entry.compressedSize, 1, out);

		delete[] nameAndContent;
	}

	fseek(out, 0, SEEK_SET);
	stamp.filesCount -= removedCount;
	fwrite(&stamp, sizeof(Stamp), 1, out);

	fclose(in);
	fclose(out);

	//remove old archive & rename new
	fs::remove_all(archive.data());
	fs::rename(tempArchive, archive.data());
}

std::vector<ArchivedFile> Archiver::extractInfo(std::wstring_view archive) {
	std::vector<ArchivedFile> entries;

	FILE *in = _wfopen(archive.data(), L"rb");
	if (in == nullptr) {
		Message::messageBox(L"Couldn't access file\n", L"Error", archive.data());
		throw "access denied";
	}

	Stamp stamp;
	memset(&stamp, 0, sizeof(Stamp));
	fread(&stamp, sizeof(Stamp), 1, in);

	if (!checkStamp(stamp)) {
		Message::messageBox(L"Unknown file format!", L"Error");
		throw "bad file format";
	}

	for (unsigned i = 0; i < stamp.filesCount; i++) {
		Entry entry{};
		memset(&entry, 0, sizeof(Entry));
		fread(&entry, sizeof(Entry), 1, in);

		//get file name
		auto *title = new wchar_t[(entry.nameLength / 2) + 1];
		fread(title, entry.nameLength, 1, in);
		title[entry.nameLength / 2] = '\0';

		//create archived entry
		ArchivedFile file{};
		file.id = entry.id;
		file.size = entry.size;
		file.compressedSize = entry.compressedSize;
		file.name = title;

		entries.push_back(file);

		fseek(in, static_cast<long>(entry.compressedSize), SEEK_CUR);
	}

	fclose(in);
	return entries;
}

unsigned Archiver::getLastId(std::wstring_view path) {
	auto files = extractInfo(path);
	return files[files.size() - 1].id;
}

void Archiver::setBufferSize(unsigned newSize) {
	maxBufferSize = newSize;
}

void Archiver::reset() {
	titles->clear();
	files->clear();
}

//private

void Archiver::initDir(std::wstring_view dirName, std::wstring_view rootDirName) {
	fs::path dir(dirName.data());

	if (!fs::exists(dir) || !fs::is_directory(dir)) {
		Message::messageBox(L"Is not a directory\n", L"Error", dirName.data());
		return;
	}

	auto iter = fs::directory_iterator(dir);
	auto end = fs::directory_iterator();

	while (iter != end) {
		if (!fs::is_regular_file(iter->path())) {
			initDir(iter->path().wstring(), rootDirName);
			++iter;
			continue;
		}

		files->push_back(iter->path());

		auto fullPath = iter->path().wstring();
		auto pathFromRoot = fullPath.substr(rootDirName.length(), fullPath.length());

		titles->push_back(pathFromRoot);
		++iter;
	}
}

void Archiver::mkdir(fs::path &path) {
	//create directory
	if (path.branch_path() != "") {

		if (fs::exists(path.branch_path())) return;

		if (!fs::create_directories(path.branch_path())) {
			Message::messageBox(L"Couldn't create directory\n", L"Error", path.branch_path().wstring());
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
			if (Message::messageBoxYesNo(L"File is already exists! Replace it?\n", L"Message", path.filename().wstring())) {
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
	unsigned compressedSizeBytes;
	unsigned compressedSizeBits;
	uint64_t bytesRead = 0;

	std::vector<std::uint8_t> *uncompressedBuffer = nullptr;

	while (bytesRead != compressedSize) {
		//read size
		fread(&compressedSizeBytes, sizeof(unsigned), 1, in);
		fread(&compressedSizeBits, sizeof(unsigned), 1, in);

		//create cluster
		auto *buffer = new uint8_t[compressedSizeBytes];
		fread(buffer, compressedSizeBytes, 1, in);

		bytesRead += compressedSizeBytes + sizeof(unsigned) * 2;

		//uncompress and write
		uncompressedBuffer = new std::vector<uint8_t>(fileSize, 0);
		const unsigned uncompressedSize = lzw::expand(buffer, compressedSizeBytes, compressedSizeBits,
		                                              uncompressedBuffer->data());
		fwrite(uncompressedBuffer->data(), uncompressedSize, 1, out);

		delete[] buffer;
	}
	delete uncompressedBuffer;
}

void Archiver::rewriteFileCompress(FILE *in, FILE *out, const uint64_t &fileSize, uint64_t &compressedSize) {
	unsigned compressedSizeBytes;
	unsigned compressedSizeBits;
	uint8_t *compressedBuffer = nullptr;

	uint8_t *buffer;
	uint64_t restSize = fileSize;
	compressedSize = 0;

	if (fileSize > maxBufferSize) {
		//split file
		auto clusters = static_cast<unsigned>(fileSize / maxBufferSize);
		restSize = fileSize % maxBufferSize;

		//create cluster
		buffer = new uint8_t[maxBufferSize];
		for (int i = 0; i < clusters; ++i) {
			//read and compress
			fread(buffer, maxBufferSize, 1, in);
			lzw::compress(buffer, maxBufferSize, &compressedBuffer, &compressedSizeBytes, &compressedSizeBits);

			//write compressed buffer
			fwrite(&compressedSizeBytes, sizeof(unsigned), 1, out);
			fwrite(&compressedSizeBits, sizeof(unsigned), 1, out);
			fwrite(compressedBuffer, compressedSizeBytes, 1, out);

			compressedSize += compressedSizeBytes + sizeof(unsigned) * 2;
		}
		delete[] buffer;
	}
	//rest file part
	buffer = new uint8_t[restSize];
	//read and compress
	fread(buffer, restSize, 1, in);
	lzw::compress(buffer, static_cast<unsigned>(restSize), &compressedBuffer, &compressedSizeBytes, &compressedSizeBits);

	//write compressed
	fwrite(&compressedSizeBytes, sizeof(unsigned), 1, out);
	fwrite(&compressedSizeBits, sizeof(unsigned), 1, out);
	fwrite(compressedBuffer, compressedSizeBytes, 1, out);

	compressedSize += compressedSizeBytes + sizeof(unsigned) * 2;

	delete[] buffer;
}
