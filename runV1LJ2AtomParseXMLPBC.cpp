//
// Created by polya on 5/13/24.
//

#include "./version1/LJPotPBC/parseXML.hpp"
std::vector<std::string> scanFiles(const int& rowNum){
    std::string searchPath="./version1Data/1d/funcLJPotPBC/row"+std::to_string(rowNum)+"/";
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
    if (argc != 3) {
        std::cout << "wrong arguments" << std::endl;
        std::exit(2);
    }


    int rowNum=std::stoi(argv[1]);
    int whichT=std::stoi(argv[2]);
    const auto tStart{std::chrono::steady_clock::now()};
    unsigned long long cellNum=10;
    auto rd=reader(rowNum,whichT,cellNum);
    rd.searchFiles();
    rd.sortFiles();
    rd.parseSummary();
    rd.parseUFiles();
    rd.parsexAxB();
    rd.data2json();
    rd.colmeans();
    rd.computeGAA();
    rd.computeGAB();
    rd.computeGBB();







}
