//
// Created by polya on 5/13/24.
//
#include "parseXML.hpp"

///UAll, xA_All, xB_All folder's files
void reader::searchFiles() {

//    this->UPath = this->TDir + "/UAll/";
    this->UPath = this->TDir + "/UAllBin/";
//    this->xAPath = this->TDir + "/xA_All/";
    this->xAPath = this->TDir + "/xA_AllBin/";
//    this->xBPath = this->TDir + "/xB_All/";
    this->xBPath = this->TDir + "/xB_AllBin/";
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
std::vector<std::string> reader::sortOneDir(const std::vector<std::string> &allFiles) {
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
//    printVec(sortedFiles);
    return sortedFiles;



}


///sort files by starting loop
void reader::sortFiles() {
    this->sorted_UFilesAll = this->sortOneDir(this->UFilesAll);
    this->sorted_xAFilesAll = this->sortOneDir(this->xAFilesAll);
    this->sorted_xBFilesAll = this->sortOneDir(this->xBFilesAll);


}


void reader::parseSummary() {
    std::string smrPath = TDir + "/summary.txt";
    std::regex lagPattern("lag=([+-]?\\d+)");
    std::regex ctStartPattern("nCounterStart=([+-]?\\d+)");
    std::regex stfnPattern("startingFileNum=(\\d+)");

    std::smatch matchLag;
    std::smatch matchCtStart;
    std::smatch matchStfn;

    std::ifstream smrIn(smrPath);
    for (std::string line; std::getline(smrIn, line);) {

        //extract lag value
        if (std::regex_search(line, matchLag, lagPattern)) {
            this->lagEst = std::stoull(matchLag.str(1));
            lagEst=static_cast<unsigned long long>(static_cast<double >(lagEst)*1.5);
            std::cout << "lagEst=" << lagEst << std::endl;
        }

        if(std::regex_search(line,matchCtStart,ctStartPattern)){
            this->nCounterStart=std::stoull(matchCtStart.str(1));
            std::cout<<"nCounterStart="<<nCounterStart<<std::endl;
        }

        if(std::regex_search(line,matchStfn,stfnPattern)){
            this->startingFileNum=std::stoull(matchStfn.str(1));
            std::cout<<"startingFileNum="<<startingFileNum<<std::endl;
        }

    }//end readline for




}

//std::string reader::searchSummaryAfterEq() {
//
//    std::regex afterEqPattern("summaryAfterEq");
//    std::smatch matchAfter;
//
//    for (const auto &entry: fs::directory_iterator(this->TDir)) {
//        std::string fileName = entry.path().string();
//        if (std::regex_search(fileName, matchAfter, afterEqPattern)) {
//            return fileName;
//
//        }
//
//
//    }//end for
//
//    return "";
//
//
//}

//void reader::parseSummaryAfterEq(const std::string &afterEqPath) {
//
//    std::regex loopPattern("total loop number:\\s*(\\d+)");
//    std::smatch matchLoop;
//    if (afterEqPath.size() > 0) {
//        std::ifstream afterIn(afterEqPath);
//        for (std::string line; std::getline(afterIn, line);) {
//            if (std::regex_search(line, matchLoop, loopPattern)) {
//                this->loopNumAfterEq = std::stoi(matchLoop.str(1));
////                std::cout<<"loopNumAfterEq="<<loopNumAfterEq<<std::endl;
//                break;
//
//            }//end if
//
//        }//end for
//
//
//    }//end if
//
//}




void reader::parseUFiles() {
    const auto tReadUStart{std::chrono::steady_clock::now()};
    this->maxDataNum=static_cast<unsigned long long >(std::ceil(static_cast<double>(UFilesAll.size()*version1dLJPot2Atom::moveNumInOneFlush)
            /static_cast<double>(lagEst)));
    UIn.reserve(maxDataNum);
    unsigned long long startingFileInd=startingFileNum-1;
    unsigned long long pointerStart=nCounterStart%version1dLJPot2Atom::moveNumInOneFlush;
    for(unsigned long long i=startingFileInd;i<sorted_UFilesAll.size();i++){
        std::string oneUFile=sorted_UFilesAll[i];
//        std::cout<<"current file is "<<oneUFile<<std::endl;
        std::vector<double> vecInOneFile = readMsgBinVec(oneUFile);
        size_t lengthInOneFile=vecInOneFile.size();
        unsigned long long j=pointerStart;
        while(j<lengthInOneFile){
            UIn.push_back(vecInOneFile[j]);
            j+=lagEst;
        }
        unsigned long long rest=lengthInOneFile-(j-lagEst);
        pointerStart=lagEst-rest;

    }
    const auto tReadUEnd{std::chrono::steady_clock::now()};
    const std::chrono::duration<double> elapsed_secondsAll{tReadUEnd - tReadUStart};
    std::cout<<"USize="<<UIn.size()<<std::endl;
    std::cout << "read U time: " << elapsed_secondsAll.count() / 3600.0 << " h" << std::endl;

}


void reader::parsexAxB() {

//    //A
    const auto tRead_xAStart{std::chrono::steady_clock::now()};
//    //reserve lengths
//    std::vector<std::vector<double>> zerothVecVec = readMsgBinVecVec(sorted_xAFilesAll[0]);
//
//    cellNum = zerothVecVec[0].size();
//    std::cout << "cellNum=" << cellNum << std::endl;
    xAInFlat.reserve(cellNum*maxDataNum);


    xBInFlat.reserve(cellNum*maxDataNum);

    unsigned long long startingFileInd=startingFileNum-1;
    unsigned long long pointerStart=nCounterStart%version1dLJPot2Atom::moveNumInOneFlush;
    //xA
    unsigned long long counterA=0;
    for(unsigned long long i=startingFileInd;i<sorted_xAFilesAll.size();i++) {

        std::string onexAFile = sorted_xAFilesAll[i];
        std::vector<std::vector<double>> vecVecInOneFile = readMsgBinVecVec(onexAFile);
        size_t lengthInOneFile=vecVecInOneFile.size();
        std::cout<<"current file is "<<onexAFile<<std::endl;
        unsigned long long j=pointerStart;
        while(j<lengthInOneFile){
            std::vector<double> vecTmp=vecVecInOneFile[j];
            xAInFlat.insert(xAInFlat.end(),vecTmp.begin(),vecTmp.end());
            j+=lagEst;
            counterA++;
        }
        unsigned long long rest=lengthInOneFile-(j-lagEst);
        pointerStart=lagEst-rest;

    }
    arma_xA = ((arma::dmat(xAInFlat)).reshape(cellNum, counterA)).t();

    const auto tRead_xAEnd{std::chrono::steady_clock::now()};

    const std::chrono::duration<double> elapsed_xAsecondsAll{tRead_xAEnd - tRead_xAStart};
    std::cout<<"xA_size="<<xAInFlat.size()<<std::endl;
    std::cout << "read xA time: " << elapsed_xAsecondsAll.count() / 3600.0 << " h" << std::endl;
    //xB
    const auto tRead_xBStart{std::chrono::steady_clock::now()};

    pointerStart=nCounterStart%version1dLJPot2Atom::moveNumInOneFlush;

    for(unsigned long long i=startingFileInd;i<sorted_xBFilesAll.size();i++) {

        std::string onexBFile = sorted_xBFilesAll[i];
        std::cout<<"current file is "<<onexBFile<<std::endl;

        std::vector<std::vector<double>> vecVecInOneFile = readMsgBinVecVec(onexBFile);
        size_t lengthInOneFile=vecVecInOneFile.size();
        unsigned long long j=pointerStart;
        while(j<lengthInOneFile){
            std::vector<double> vecTmp=vecVecInOneFile[j];
            xBInFlat.insert(xBInFlat.end(),vecTmp.begin(),vecTmp.end());
            j+=lagEst;
        }
        unsigned long long rest=lengthInOneFile-(j-lagEst);
        pointerStart=lagEst-rest;
    }
    arma_xB = ((arma::dmat(xBInFlat)).reshape(cellNum, counterA)).t();

    const auto tRead_xBEnd{std::chrono::steady_clock::now()};
    const std::chrono::duration<double> elapsed_xBsecondsAll{tRead_xBEnd - tRead_xBStart};
    std::cout<<"xB_size="<<xBInFlat.size()<<std::endl;
    std::cout << "read xB time: " << elapsed_xBsecondsAll.count() / 3600.0 << " h" << std::endl;

//    arma::drowvec meanB=arma::mean(arma_xB,0);
//    std::cout<<"B size=("<<arma_xB.n_rows<<", "<<arma_xB.n_cols<<")"<<std::endl;
//    std::cout<<"-------------------------"<<std::endl;
}


///
/// @param filename bin file name
/// @return vector
std::vector<double> reader::readMsgBinVec(const std::string &filename) {
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

///
/// @param filename bin file name
/// @return vector<vector<double>>
std::vector<std::vector<double>> reader::readMsgBinVecVec(const std::string &filename) {

// Step 1: Read the binary data from the file
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

    std::vector<std::vector<double>> nested_vec;
    obj.convert(nested_vec);

    return nested_vec;


}


///data to json, json as input to plot
void reader::data2json() {

    std::string jsonPath = this->TDir + "/jsonData/";

//    //write U
    std::string UJsonPath = jsonPath + "/jsonU/";
    if (!fs::is_directory(UJsonPath) || !fs::exists(UJsonPath)) {
        fs::create_directories(UJsonPath);
    }

    std::string UJsonFile = UJsonPath + "/UData.json";

    boost::json::object objU;
    boost::json::array arrU;
    for (const auto &val: UIn) {
        arrU.push_back(val);
    }
    objU["U"] = arrU;
    std::ofstream ofsU(UJsonFile);
    std::string UStr = boost::json::serialize(objU);
    ofsU << UStr << std::endl;
    ofsU.close();
//
//
//    //write xA, xB
    for (int i = 0; i < cellNum; i++) {
        std::string cellPathTmp = jsonPath + "jsonUnitCell" + std::to_string(i) + "/";
        if (!fs::is_directory(cellPathTmp) || !fs::exists(cellPathTmp)) {
            fs::create_directories(cellPathTmp);
        }
        boost::json::object obj_xAxB;

        std::string cellJsonFile = cellPathTmp + "xAxBData.json";

        arma::dcolvec colA = arma_xA.col(i);
        boost::json::array oneCol_xA;
        for (const auto &val: colA) {
            oneCol_xA.push_back(val);
        }
        arma::dcolvec colB = arma_xB.col(i);
        boost::json::array oneCol_xB;
        for (const auto &val: colB) {
            oneCol_xB.push_back(val);
        }
        obj_xAxB["xA"] = oneCol_xA;

        obj_xAxB["xB"] = oneCol_xB;

        std::ofstream ofsxAxB(cellJsonFile);
        std::string xAxBStr = boost::json::serialize(obj_xAxB);
        ofsxAxB << xAxBStr << std::endl;
        ofsxAxB.close();


    }


}



///compute the column means of arma_xA, arma_xB
void reader::colmeans(){
    this->E_xARow=arma::mean(arma_xA,0);
    this->E_xBRow=arma::mean(arma_xB,0);

    this->E_xACol=E_xARow.t();
    this->E_xBCol=E_xBRow.t();

    this->E_xA2=E_xACol*E_xARow;
    this->E_xB2=E_xBCol*E_xBRow;
//    std::cout<<"cellNum="<<cellNum<<std::endl;

//    E_xARow.print("mean xA:");
//    E_xBRow.print("mean xB");
//
//E_xA2.print("EA2:");
//    E_xB2.print("EB2:");


}

///compute correlation functions GAA
void reader::computeGAA() {

    arma::dmat YA = arma::zeros(cellNum, cellNum);

    int Q = arma_xA.n_rows;

    for (int q = 0; q < Q; q++) {
        arma::drowvec rowTmp = arma_xA.row(q);
        arma::dcolvec colTmp = rowTmp.t();
        YA += colTmp * rowTmp;

    }

    double QDB = static_cast<double>(Q);

    YA /= QDB;

//    YA.print("YA:");

    arma::dmat GAA = YA - E_xA2;

    std::string outGAA=TDir+"/GAA.csv";
    std::ofstream ofs(outGAA);

    printMat(GAA,ofs);
    ofs.close();


}


///compute correlation functions GAB
void reader::computeGAB(){

    arma::dmat YAB = arma::zeros(cellNum, cellNum);
    int Q = arma_xA.n_rows;

    for(int q=0;q<Q;q++){
        arma::drowvec rowATmp = arma_xA.row(q);
        arma::dcolvec colATmp=rowATmp.t();

        arma::drowvec rowBTmp=arma_xB.row(q);

        YAB+=colATmp*rowBTmp;

    }

    double QDB = static_cast<double>(Q);

    YAB/=QDB;

    arma::dmat GAB=YAB-E_xACol*E_xBRow;
    std::string outGAB=TDir+"/GAB.csv";
    std::ofstream ofs(outGAB);

    printMat(GAB,ofs);
    ofs.close();


}


///compute correlation functions GBB
void reader::computeGBB(){
    arma::dmat YB = arma::zeros(cellNum, cellNum);

    int Q = arma_xB.n_rows;

    for(int q=0;q<Q;q++){
        arma::drowvec rowTmp = arma_xB.row(q);
        arma::dcolvec colTmp = rowTmp.t();
        YB += colTmp * rowTmp;


    }
    double QDB = static_cast<double>(Q);
    YB/=QDB;

    arma::dmat GBB=YB-E_xB2;
    std::string outGBB=TDir+"/GBB.csv";
    std::ofstream ofs(outGBB);

    printMat(GBB,ofs);
    ofs.close();

}