#include <iostream>
#include <ctime>
#include <jni.h>
#include "src/archiver.hpp"

#include <io.h>
#include <fcntl.h>


int main(int argc, char *argv[]) {
	setlocale(LC_ALL, "Russian");
	_setmode(_fileno(stdout), _O_U16TEXT);
	_setmode(_fileno(stdin), _O_U16TEXT);
	_setmode(_fileno(stderr), _O_U16TEXT);


	Archiver *archiver = new Archiver();
	std::vector<std::wstring_view> s;
	s.emplace_back(L"cyrillic/test");
	s.emplace_back(L"cyrillic/Бодя.mp4");
	s.emplace_back(L"cyrillic/lzw.h");


	archiver->add(s);
	archiver->crush(L"cyrillic/1/пизда.ar");





	std::vector<std::wstring_view> s1;
	s1.emplace_back(L"cyrillic/swaston.txt");
	archiver->addToExisting(s1,L"cyrillic/1/пизда.ar");

	std::vector<uint32_t> id;
    id.emplace_back(0);
	archiver->extractFiles(L"cyrillic/1/пизда.ar", L"cyrillic/1/out", id, false);


//	archiver->extract(L"cyrillic/1/пизда.ar", L"cyrillic/1/out", false);

//    std::vector<uint32_t> id;
//    id.emplace_back(0);
//    id.emplace_back(1);
//
//    archiver->remove(id,
//                                  "C:/Users/IgorTheMLGPro/CLionProjects/3-1/archivec-core/cmake-build-debug/1/test_jni.ar");
//

//    std::vector<uint32_t> ids;
//    ids.emplace_back(0);
//    ids.emplace_back(3);
//    ids.emplace_back(6);
//
//    archiver->extractFiles("C:/Users/IgorTheMLGPro/CLionProjects/3-1/archivec-core/cmake-build-debug/1/test1.ar",
//                            "C:/Users/IgorTheMLGPro/CLionProjects/3-1/archivec-core/cmake-build-debug/out_lzw",
//                            ids);




//
//    archiver->extract("C:/Users/IgorTheMLGPro/CLionProjects/3-1/archivec-core/cmake-build-debug/1/test.ar",
//                      "C:/Users/IgorTheMLGPro/CLionProjects/3-1/archivec-core/cmake-build-debug/out/");


//    fs::remove_all("C:/Users/IgorTheMLGPro/CLionProjects/3-1/archivec-core/cmake-build-debug/1/");
//    fs::remove_all("C:/Users/IgorTheMLGPro/CLionProjects/3-1/archivec-core/cmake-build-debug/out/");

	return EXIT_SUCCESS;
}