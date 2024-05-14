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

std::string reader::searchSummaryAfterEq(){

std::regex afterEqPattern("summaryAfterEq");
std::smatch matchAfter;

    for (const auto &entry: fs::directory_iterator(this->TDir)) {
        std::string fileName = entry.path().string();
        if (std::regex_search(fileName, matchAfter, afterEqPattern)) {
            return fileName;

        }


    }//end for

    return "";


}

void reader::parseSummaryAfterEq(const std::string &afterEqPath){

    std::regex  loopPattern("total loop number:\\s*(\\d+)");
    std::smatch  matchLoop;
    if (afterEqPath.size()>0){
        std::ifstream afterIn(afterEqPath);
        for(std::string line;std::getline(afterIn,line);){
            if (std::regex_search(line, matchLoop, loopPattern)) {
                this->loopNumAfterEq=std::stoi(matchLoop.str(1));
//                std::cout<<"loopNumAfterEq="<<loopNumAfterEq<<std::endl;
                break;

            }//end if

        }//end for


    }//end if

}



void reader::UAndxFilesSelected(){
    loopNumToInclude=moveNumInOneFlush*lastFileNum+loopNumAfterEq;
//    std::cout<<"loopNumToInclude="<<loopNumToInclude<<std::endl;
 double loopNumToIncludeDB=static_cast<double >(loopNumToInclude);
 double moveNumInOneFlushDB=static_cast<double >(moveNumInOneFlush);

 fileNumSelected=static_cast<int>(std::ceil(loopNumToIncludeDB/moveNumInOneFlushDB));
//    std::cout<<"fileNumSelected="<<fileNumSelected<<std::endl;


 for(int i=sorted_UFilesAll.size()-fileNumSelected;i<sorted_UFilesAll.size();i++){
     this->UFilesSelected.push_back(sorted_UFilesAll[i]);
 }
// std::cout<<"fileNumSelected="<<fileNumSelected<<std::endl;
// std::cout<<"len(UFilesSelected)="<<UFilesSelected.size()<<std::endl;

for(int i=sorted_xAFilesAll.size()-fileNumSelected;i<sorted_xAFilesAll.size();i++){
    this->xAFilesSelected.push_back(sorted_xAFilesAll[i]);
}

    for(int i=sorted_xBFilesAll.size()-fileNumSelected;i<sorted_xBFilesAll.size();i++){
        this->xBFilesSelected.push_back(sorted_xBFilesAll[i]);
    }





}


void reader::parseUFiles() {
    const auto tUStart{std::chrono::steady_clock::now()};
    for (const std::string &oneUFile: this->UFilesSelected) {
        std::vector<double> vecInOneFile;
        std::ifstream ifs(oneUFile);
        if (!ifs.is_open()) {
            std::cerr << "cannot open " << oneUFile << std::endl;
            return;
        }
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(vecInOneFile);

        UIn.insert(UIn.end(), vecInOneFile.begin(), vecInOneFile.end());


    }

    const auto tUEnd{std::chrono::steady_clock::now()};
    const std::chrono::duration<double> elapsed_secondsAll{tUEnd - tUStart};
    std::cout << "parse U time: " << elapsed_secondsAll.count() << " s" << std::endl;

    for(int i=0;i<UIn.size();i+=lag){
        USelected.push_back(UIn[i]);
    }
//    std::cout<<"lag="<<lag<<std::endl;
//    std::cout<<"len(UIn)="<<UIn.size()<<std::endl;
//    std::cout<<"len(USelected)="<<USelected.size()<<std::endl;
    armaU= arma::dcolvec (USelected);

//    std::cout<<armaU<<std::endl;


}




void reader::parsexAxB(){

    //A
    const auto tAStart{std::chrono::steady_clock::now()};

    for(const std::string &onexAFile: xAFilesSelected){
        std::vector<std::vector<double>> vecVecInOneFile;
    std::ifstream ifs(onexAFile);
        if (!ifs.is_open()) {
            std::cerr << "cannot open "<<onexAFile << std::endl;
            return;
        }
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(vecVecInOneFile);
        xAIn.insert(xAIn.end(),vecVecInOneFile.begin(),vecVecInOneFile.end());


    }

    const auto tAEnd{std::chrono::steady_clock::now()};
    const std::chrono::duration<double> A_elapsed_secondsAll{tAEnd - tAStart};
    std::cout << "parse A time: " << A_elapsed_secondsAll.count() << " s" << std::endl;




}