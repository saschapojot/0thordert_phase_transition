#include "combineVecs.hpp"


///search U files
void loader::searchFiles(){
    for (const auto &entry: fs::directory_iterator(UPath)) {
        this->UFilesAll.push_back(entry.path().string());
    }



}



///
/// @param path the path containing bin files
/// @return sorted bin files by starting loop
std::vector<std::string> loader::sortOneDir(const std::vector<std::string> &allFiles){

    std::vector<int> startingLoopsAll;
    for (const std::string &name: allFiles) {
        std::regex startPattern("loopStart(\\d+)loopEnd");
        std::smatch matchPattern;
        if (std::regex_search(name, matchPattern, startPattern)) {
            startingLoopsAll.push_back(std::stoi(matchPattern.str(1)));
        }

    }

    std::vector<size_t> inds = this->argsort<int>(startingLoopsAll);;

    std::vector<std::string> sortedFiles;
    for (const auto &i: inds) {
        sortedFiles.push_back(allFiles[i]);
    }

    return sortedFiles;


}

///sort files by starting loop
void loader::sortFiles(){

    this->sorted_UFilesAll = this->sortOneDir(this->UFilesAll);



}

void loader::parseUFiles(){

    const auto tUStart{std::chrono::steady_clock::now()};
    UIn.reserve(this->sorted_UFilesAll.size() * moveNumInOneFlush);
    for (const std::string &oneUFile: this->sorted_UFilesAll) {
        std::vector<double> vecInOneFile = readMsgBinVec(oneUFile);
        UIn.insert(UIn.end(), vecInOneFile.begin(), vecInOneFile.end());
    }


}

///
/// @param filename bin file name
/// @return vector
std::vector<double> loader::readMsgBinVec(const std::string &filename){

    std::ifstream infile(filename, std::ios::binary);
    if (!infile.is_open()) {
        throw std::runtime_error("Unable to open file for reading");
    }

    std::stringstream buffer;
    buffer << infile.rdbuf();
    std::string serialized_data = buffer.str();
    infile.close();

    // Step 2: Deserialize the binary data using MessagePack
    msgpack::object_handle oh = msgpack::unpack(serialized_data.data(), serialized_data.size());
    msgpack::object obj = oh.get();

    std::vector<double> vec;
    obj.convert(vec);

    return vec;



}

void loader::toPickle(const std::string & outUPickleName){
    save_vector_to_pickle(UIn,outUPickleName);


}