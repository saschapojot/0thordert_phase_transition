//
// Created by polya on 5/13/24.
//

#include "./version1/LJPot/parseXML.hpp"
std::vector<std::string> scanFiles(const int& rowNum){
    std::string searchPath="./version1Data/1d/row"+std::to_string(rowNum)+"/funcLJPot/initstd::string/";
    std::vector<std::string> TDirs;
    if(fs::exists(searchPath) && fs::is_directory(searchPath)){
        for(const auto &entry:fs::directory_iterator(searchPath)){
            if(entry.path().filename().string()[0]=='T'){
                TDirs.push_back(entry.path().filename().string());
            }

        }
    }
//    for(const auto&s:TDirs)
//    {
//        std::cout<<s<<std::endl;
//    }

    return TDirs;

}



int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "wrong number of arguments" << std::endl;
        exit(1);
    }
    int rowNum = std::stoi(argv[1]);

    std::vector<std::string> TDirs = scanFiles(rowNum);
    for (const auto &s: TDirs) {
//        std::cout<<"file is "<<s<<std::endl;
        const auto tCStart{std::chrono::steady_clock::now()};
        auto rd = reader(rowNum, s);
        rd.searchFiles();
        rd.sortFiles();

        rd.parseSummary();
        std::string smrAfterEq = rd.searchSummaryAfterEq();
        rd.parseSummaryAfterEq(smrAfterEq);
        rd.UAndxFilesSelected();
        rd.parseUFiles();
        rd.parsexAxB();

    }

}
