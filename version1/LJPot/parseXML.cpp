//
// Created by polya on 5/13/24.
//
#include "parseXML.hpp"

///UAll, xA_All, xB_All folder's files
void reader::searchFiles() {

    this->UPath = this->TDir + "/UAll/";
    this->xAPath = this->TDir + "/xA_All/";
    this->xBPath = this->TDir + "/xB_All/";
//    std::cout<<UPath<<std::endl;
//    std::cout<<xAPath<<std::endl;
//    std::cout<<xAPath<<std::endl;
    for (const auto &entry: fs::directory_iterator(UPath)) {
        this->UFilesAll.push_back(entry.path().string());
    }

    for (const auto &entry: fs::directory_iterator(xAPath)) {
        this->xAFilesAll.push_back(entry.path().string());
    }

    for (const auto &entry: fs::directory_iterator(xBPath)) {
        this->xBFilesAll.push_back(entry.path().string());
    }


}


///
/// @param path the path containing xml files
/// @return sorted xml files by starting loop
std::vector <std::string> reader::sortOneDir(const std::vector <std::string> &allFiles){
    std::vector<int> startingLoopsAll;
    for (const std::string &name: allFiles) {
        std::regex startPattern("loopStart(\\d+)loopEnd");
        std::smatch matchPattern;
        if (std::regex_search(name, matchPattern, startPattern)) {
            startingLoopsAll.push_back(std::stoi(matchPattern.str(1)));
        }

    }

    std::vector <size_t> inds = this->argsort<int>(startingLoopsAll);;

    std::vector <std::string> sortedFiles;
    for (const auto &i: inds) {
        sortedFiles.push_back(allFiles[i]);
    }

    return sortedFiles;


}


///sort files by starting loop
void reader::sortFiles(){
    this->sorted_UFilesAll = this->sortOneDir(this->UFilesAll);
    this->sorted_xAFilesAll=this->sortOneDir(this->xAFilesAll);
    this->sorted_xBFilesAll=this->sortOneDir(this->xBFilesAll);




}


void reader::parseSummary(){
    std::string smrPath=TDir+"/summary.txt";
    std::regex lagPattern("lag=([+-]?\\d+)");
    std::regex lastFilesNumPattern("lastFileNum=(\\d+)");

    std::smatch matchLag;
    std::smatch matchFileNum;


    std::ifstream smrIn(smrPath);
    for(std::string line; std::getline(smrIn,line);){
        //extract lag value
        if (std::regex_search(line, matchLag, lagPattern)) {
            this->lag = std::stoi(matchLag.str(1));
//            std::cout<<"lag="<<lag<<std::endl;
        }
        //extract lastFilesNum
        if (std::regex_search(line, matchFileNum, lastFilesNumPattern)) {

            this->lastFileNum = std::stoi(matchFileNum.str(1));

//            std::cout<<"lastFilesNum="<<lastFileNum<<std::endl;
        }

    }//end readline for



}