#include "version1LJPotPBC2Atom.hpp"


///
/// @param xA positions of atom A
/// @param xB positions of atom B
/// @param rLast
/// @return beta*potential
double version1dLJPot2Atom::f(const arma::dcolvec &xA, const arma::dcolvec &xB, const double & L) {
    return this->beta * ((*potFuncPtr)(xA, xB,L));

}


///
/// @param xACurr positions of atom A
/// @param xBCurr positions of atom B
/// @param LCurr
/// @param zANext proposed positions of atom A
/// @param zBNext proposed positions of atom B
/// @param LNext
void version1dLJPot2Atom::proposal(const arma::dcolvec &xACurr, const arma::dcolvec &xBCurr, const double &LCurr,
                                   arma::dcolvec &zANext, arma::dcolvec &zBNext, double &LNext) {
    std::random_device rd;
    std::ranlux24_base gen(rd());
    //fix left end (0A)
//    zANext(0)=xACurr(0);

    for (int j = 0; j < N; j++) {
        std::normal_distribution<double> dTmp(xACurr(j), stddev);
        zANext(j) = dTmp(gen);
    }


    for (int j = 0; j < N; j++) {
        std::normal_distribution<double> dTmp(xBCurr(j), stddev);
        zBNext(j) = dTmp(gen);
    }

    std::normal_distribution<double> dLast(LCurr,stddev);
    LNext=dLast(gen);

}


///
/// @param cmd python execution string
/// @return signal from the python
std::string version1dLJPot2Atom::execPython(const char *cmd) {
    std::array<char, 4096*4> buffer; // Buffer to store command output
    std::string result; // String to accumulate output

    // Open a pipe to read the output of the executed command
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    // Read the output a chunk at a time and append it to the result string
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result; // Return the accumulated output



}


///
/// @param xA current positions of atom A
/// @param xB current positions of atom B
/// @param LCurr
/// @param zA proposed positions of atom A
/// @param zB proposed positions of atom B
/// @param LNext
/// @return
double version1dLJPot2Atom::acceptanceRatio(const arma::dcolvec &xA, const arma::dcolvec &xB,const double& LCurr,
                                            const arma::dcolvec &zA, const arma::dcolvec &zB, const double &LNext) {

    double numerator = -f(zA, zB, LNext);

    double denominator = -f(xA, xB,LCurr);
//    double UCurr=(*potFuncPtr)(xA, xB,LCurr);
//    double UNext=(*potFuncPtr)(zA, zB, LNext);
//    std::cout<<"UCurr="<<UCurr<<", UNext="<<UNext<<std::endl;
    double ratio = std::exp(numerator - denominator);

    return std::min(1.0, ratio);

}


///
/// @param filename xml file name of vec
///@param vec vector to be saved
void version1dLJPot2Atom::saveVecToXML(const std::string &filename, const std::vector<double> &vec) {

    std::ofstream ofs(filename);
    boost::archive::xml_oarchive oa(ofs);
    oa & BOOST_SERIALIZATION_NVP(vec);

}


///
/// @param filename  xml file name of vecvec
/// @param vecvec vector<vector> to be saved
void version1dLJPot2Atom::saveVecVecToXML(const std::string &filename, const std::vector<std::vector<double>> &vecvec) {
    std::ofstream ofs(filename);
    boost::archive::xml_oarchive oa(ofs);
    oa & BOOST_SERIALIZATION_NVP(vecvec);


}


///
/// @param lag decorrelation length
/// @param loopTotal total mc steps
/// @param equilibrium whether equilibrium has reached
/// @param same whether all values of potential are the same
/// @param xALast last positions of atom A
/// @param xBLast last positions of atom B
/// @param LLast
void version1dLJPot2Atom::readEqMc(unsigned long long &lag, unsigned long long &loopTotal, bool &equilibrium, bool &same, std::vector<double> &xALast,
                                   std::vector<double> &xBLast, double &LLast) {
    std::random_device rd;
    std::ranlux24_base e2(rd());
    std::uniform_real_distribution<> distUnif01(0, 1);//[0,1)

    //generate initial values of positions
//    double leftEnd = -300.0;
//    double rightEnd = 300.0;
//    std::uniform_real_distribution<> distUnifLeftRight(leftEnd, rightEnd);

    arma::dcolvec xACurr(N);
    arma::dcolvec xBCurr(N);
    double LCurr=0;

//    for(int j=0;j<N;j++){
//        xACurr(j)=distUnifLeftRight(e2);
//        xBCurr(j)=distUnifLeftRight(e2);
//    }
    this->initPositionsEquiDistance(xACurr, xBCurr,LCurr);
    //initial value of energy
    double UCurr = (*potFuncPtr)(xACurr, xBCurr,LCurr);
    //output directory
    std::ostringstream sObjT;
    sObjT << std::fixed;
    sObjT << std::setprecision(10);
    sObjT << T;
    std::string TStr = sObjT.str();
    std::string funcName = demangle(typeid(*potFuncPtr).name());
//    std::string  initFuncName= demangle(typeid(initFuncName).name());
    std::string outDir = "./version1Data/1d/func" + funcName +"/row"+std::to_string(rowNum)+ "/T" + TStr + "/";

    std::string outUAllSubDir = outDir + "UAllPickle/";
//    std::string out_xA_AllSubDir = outDir + "xA_All/";
//    std::string out_xB_AllSubDir = outDir + "xB_All/";
    std::string outLAllSubDir=outDir+"LAllPickle/";


    std::string outUAllBinSubDir = outDir + "UAllBin/";
    std::string out_xA_AllBinSubDir = outDir + "xA_AllBin/";
    std::string out_xB_AllBinSubDir = outDir + "xB_AllBin/";
    std::string outLAllBinSubDir=outDir+"LAllBin/";

    if (!fs::is_directory(outUAllSubDir) || !fs::exists(outUAllSubDir)) {
        fs::create_directories(outUAllSubDir);
    }

//    if (!fs::is_directory(out_xA_AllSubDir) || !fs::exists(out_xA_AllSubDir)) {
//        fs::create_directories(out_xA_AllSubDir);
//    }

//    if (!fs::is_directory(out_xB_AllSubDir) || !fs::exists(out_xB_AllSubDir)) {
//        fs::create_directories(out_xB_AllSubDir);
//    }

    if (!fs::is_directory(outLAllSubDir) || !fs::exists(outLAllSubDir)) {
        fs::create_directories(outLAllSubDir);
    }




    if (!fs::is_directory(outUAllBinSubDir) || !fs::exists(outUAllBinSubDir)) {
        fs::create_directories(outUAllBinSubDir);
    }

    if (!fs::is_directory(out_xA_AllBinSubDir) || !fs::exists(out_xA_AllBinSubDir)) {
        fs::create_directories(out_xA_AllBinSubDir);
    }
    if (!fs::is_directory(out_xB_AllBinSubDir ) || !fs::exists(out_xB_AllBinSubDir )) {
        fs::create_directories(out_xB_AllBinSubDir );
    }

    if (!fs::is_directory(outLAllBinSubDir ) || !fs::exists(outLAllBinSubDir )) {
        fs::create_directories(outLAllBinSubDir );
    }

    std::regex stopRegex("stop");
    std::regex wrongRegex("wrong");
    std::regex ErrRegex("Err");
    std::regex lagRegex("lag=\\s*(\\d+)");
    std::regex fileNumRegex("fileNum=\\s*(\\d+)");
    std::regex sameRegex("same");
    std::regex eqRegex("equilibrium");
    std::regex ctStartRegex("nCounterStart=\\s*(\\d+)");
    std::regex stfnRegex("startingFileNum=\\s*(\\d+)");
    std::regex dataNumEqRegex("numDataPoints=\\s*(\\d+)");


    std::smatch matchUStop;
    std::smatch matchUWrong;
    std::smatch matchUErr;
    std::smatch matchULag;
    std::smatch matchFileNum;
    std::smatch matchUSame;
    std::smatch matchUEq;
    std::smatch matchCounterStart;
    std::smatch matchStfn;
    std::smatch matchDataNumEq;



    unsigned long long counter = 0;
    unsigned long long fls = 0;
    bool active = true;
    const auto tMCStart{std::chrono::steady_clock::now()};
    std::vector<double> last_xA;
    std::vector<double> last_xB;
    double last_L;

    while (fls < this->flushMaxNum and active == true) {
        std::vector<std::vector<double>> xA_AllPerFlush;
        //init xA
        xA_AllPerFlush.reserve(moveNumInOneFlush);
        std::vector<double> initVecVal(N, 0);
        for(int i=0;i<moveNumInOneFlush;i++){
            xA_AllPerFlush.push_back(initVecVal);
        }

        std::vector<std::vector<double>> xB_AllPerFlush;
        //init B
        xB_AllPerFlush.reserve(moveNumInOneFlush);
        for(int i=0;i<moveNumInOneFlush;i++){
            xB_AllPerFlush.push_back(initVecVal);
        }
        std::vector<double> UAllPerFlush;
        //init U
        UAllPerFlush.reserve(moveNumInOneFlush);
        for(int i=0;i<moveNumInOneFlush;i++){
            UAllPerFlush.push_back(0);
        }
        std::vector<double> LAllPerFlush;
        //init L
        LAllPerFlush.reserve(moveNumInOneFlush);
        for(int i=0;i<moveNumInOneFlush;i++){
            LAllPerFlush.push_back(0);
        }

        int loopStart = fls * moveNumInOneFlush;

        for (int i = 0; i < moveNumInOneFlush; i++) {
            //propose a move
            arma::dcolvec xANext = arma::dcolvec(N);
            arma::dcolvec xBNext = arma::dcolvec(N);
            double LNext;
            proposal(xACurr, xBCurr,LCurr, xANext, xBNext,LNext);
            double r = acceptanceRatio(xACurr, xBCurr,LCurr, xANext, xBNext,LNext);
            double u = distUnif01(e2);

            counter++;
            if (u <= r) {
//                std::cout<<"UCurr="<<UCurr<<std::endl;
                xACurr = xANext;
                xBCurr = xBNext;
                LCurr=LNext;
                UCurr = (*potFuncPtr)(xACurr, xBCurr,LCurr);
//                std::cout<<"UNext="<<UCurr<<std::endl;
            }

            xA_AllPerFlush[i]=(arma::conv_to<std::vector<double>>::from(xACurr));
            xB_AllPerFlush[i]=(arma::conv_to<std::vector<double>>::from(xBCurr));
            UAllPerFlush[i]=(UCurr);
            LAllPerFlush[i]=(LCurr);


        }//end for loop in 1 flush

        int loopEnd = loopStart + moveNumInOneFlush - 1;
        std::string filenameMiddle = "loopStart" + std::to_string(loopStart) +
                                     "loopEnd" + std::to_string(loopEnd) + "T" + TStr;

        std::string outUPickleFileName = outUAllSubDir + filenameMiddle + ".UAll.pkl";
        std::string outUBinFileName=outUAllBinSubDir+filenameMiddle+"UAll.bin";
//        this->saveVecToXML(outUFileName, UAllPerFlush);
        save_vector_to_pickle(UAllPerFlush,outUPickleFileName);
        this->saveVecToBin(outUBinFileName,UAllPerFlush);

//        std::string out_xAFileName = out_xA_AllSubDir + filenameMiddle + ".xA_All.xml";
//        this->saveVecVecToXML(out_xAFileName, xA_AllPerFlush);
        std::string out_xABinFileName=out_xA_AllBinSubDir+filenameMiddle+".xA_All.bin";
        this->saveVecVecToBin(out_xABinFileName,xA_AllPerFlush);


//        std::string out_xBFileName = out_xB_AllSubDir + filenameMiddle + ".xB_All.xml";
//        this->saveVecVecToXML(out_xBFileName, xB_AllPerFlush);
        std::string out_xBBinFileName=out_xB_AllBinSubDir+filenameMiddle+".xB_All.bin";
        this->saveVecVecToBin(out_xBBinFileName,xB_AllPerFlush);

        std::string outLPickleFileName=outLAllSubDir+filenameMiddle+".LAll.pkl";
        save_vector_to_pickle(LAllPerFlush,outLPickleFileName);
//        this->saveVecToXML(outLFileName,LAllPerFlush);
        std::string outLBinFileName=outLAllBinSubDir+filenameMiddle+".LAll.bin";
        this->saveVecToBin(outLBinFileName,LAllPerFlush);


        const auto tflushEnd{std::chrono::steady_clock::now()};

        const std::chrono::duration<double> elapsed_seconds{tflushEnd - tMCStart};
        std::cout << "flush " << fls << std::endl;
        std::cout << "time elapsed: " << elapsed_seconds.count() / 3600.0 << " h" << std::endl;

        //communicate with python to inquire equilibrium

        //inquire equilibrium of U
        std::string commandU = "python3 checkVec.py " + outUAllSubDir;
        std::string resultU;

        if ((fls+1) % 1 == 0 and fls>=0) {
            try {
                const auto tPyStart{std::chrono::steady_clock::now()};
                resultU = this->execPython(commandU.c_str());
                std::cout << "U message from python: " << resultU << std::endl;
                const auto tPyEnd{std::chrono::steady_clock::now()};

                const std::chrono::duration<double> elapsedpy_secondsAll{tPyEnd - tPyStart};
                std::cout << "py time: " << elapsedpy_secondsAll.count()  << " s" << std::endl;

            }
            catch (const std::exception &e) {
                std::cerr << "Error: " << e.what() << std::endl;
                std::exit(10);
            }
            catch (...) {
                // Handle any other exceptions
                std::cerr << "Error" << std::endl;
                std::exit(11);
            }
            // parse result
            if (std::regex_search(resultU, matchUErr, ErrRegex)) {
                std::cout << "error encountered" << std::endl;
                std::exit(12);
            }

            if (std::regex_search(resultU, matchUWrong, wrongRegex)) {
                std::exit(13);
            }
            if (std::regex_search(resultU, matchUStop, stopRegex)) {
                if (std::regex_search(resultU, matchUSame, sameRegex)) {
                    active = false;
                    same = true;
                    std::regex_search(resultU, matchFileNum, fileNumRegex);
                    std::string fileNumStr = matchFileNum.str(1);
//                    this->lastFileNum = std::stoi(fileNumStr);

                    last_xA = xA_AllPerFlush[xA_AllPerFlush.size() - 1];
                    last_xB = xB_AllPerFlush[xB_AllPerFlush.size() - 1];
                    last_L=LAllPerFlush[LAllPerFlush.size()-1];


                }


            }//end of regex search

            if (std::regex_search(resultU, matchUEq, eqRegex)) {
                if (std::regex_search(resultU, matchULag, lagRegex)) {

                    std::string lagStrU = matchULag.str(1);
                    int lagU = std::stoi(lagStrU);
                    std::cout << "lag=" << lagU << std::endl;
                    lag = lagU;
//                    std::regex_search(resultU, matchFileNum, fileNumRegex);
//                    std::string fileNumStr = matchFileNum.str(1);
//                    this->lastFileNum = std::stoi(fileNumStr);

                    if(std::regex_search(resultU,matchCounterStart,ctStartRegex)) {
                        this->nCounterStart = std::stoull(matchCounterStart.str(1));
                    }

                    active = false;
                    last_xA = xA_AllPerFlush[xA_AllPerFlush.size() - 1];
                    last_xB = xB_AllPerFlush[xB_AllPerFlush.size() - 1];
                    last_L=LAllPerFlush[LAllPerFlush.size()-1];
                    if(std::regex_search(resultU,matchStfn,stfnRegex)){
                        this->startingFileNum=std::stoull(matchStfn.str(1));
                    }

                    if(std::regex_search(resultU,matchDataNumEq,dataNumEqRegex)){
                        this->dataNumInEq=std::stoull(matchDataNumEq.str(1));

                        std::cout<<"dataNumInEq="<<dataNumInEq<<std::endl;

                    }

                }


            }//end of regex search

        }//end if
        fls++;


    }//end while

    loopTotal = counter;
    std::ofstream outSummary(outDir + "summary.txt");
    const auto tMCEnd{std::chrono::steady_clock::now()};
    const std::chrono::duration<double> elapsed_secondsAll{tMCEnd - tMCStart};
    outSummary << "total mc time: " << elapsed_secondsAll.count() / 3600.0 << " h" << std::endl;
    outSummary << "total loop number: " << loopTotal << std::endl;

//    outSummary << "lastFileNum=" << lastFileNum << std::endl;
    outSummary << "equilibrium reached: " << !active << std::endl;
    outSummary << "same: " << same << std::endl;

    outSummary << "lag=" << lag << std::endl;
    outSummary<<"step length="<<stddev<<std::endl;
    outSummary<<"nCounterStart="<<nCounterStart<<std::endl;
    outSummary<<"startingFileNum="<<startingFileNum<<std::endl;
    outSummary<<"collected number of data points: "<<dataNumInEq<<std::endl;

    outSummary.close();

    equilibrium = !active;
    xALast = last_xA;
    xBLast = last_xB;
    LLast=last_L;


}//end of reachMC function





///
/// @param lag decorrelation length
/// @param loopEq total loop numbers in reaching equilibrium
/// @param xA_init xA from readEqMc
/// @param xB_init xB from readEqMc
/// @param LInit
void version1dLJPot2Atom::executionMCAfterEq(const unsigned long long &lag, const unsigned long long &loopEq, const std::vector<double> &xA_init,
                                             const std::vector<double> &xB_init, const double &LInit) {

    unsigned long long counter = 0;
    if (dataNumTotal <= dataNumInEq) {
        return;
    }
    unsigned long long remainingDataNum = this->dataNumTotal - this->dataNumInEq;

    unsigned long long remainingLoopNum = remainingDataNum * lag;

    std::cout<<"remainingLoopNum="<<remainingLoopNum<<std::endl;

    double remainingLoopNumDB = static_cast<double>(remainingLoopNum);
    double remainingFlushNumDB = std::ceil(remainingLoopNumDB / static_cast<double>(moveNumInOneFlush));
    unsigned long long remainingFlushNum = static_cast<unsigned long long>(remainingFlushNumDB);
    std::cout<<"remaining flush: "<<remainingFlushNum<<std::endl;
    std::random_device rd;
    std::ranlux24_base e2(rd());
    std::uniform_real_distribution<> distUnif01(0, 1);//[0,1)

    arma::dcolvec xACurr(xA_init);
    arma::dcolvec xBCurr(xB_init);
    double LCurr=LInit;

    double UCurr = (*potFuncPtr)(xACurr, xBCurr,LCurr);
    //output directory
    std::ostringstream sObjT;
    sObjT << std::fixed;
    sObjT << std::setprecision(10);
    sObjT << T;
    std::string TStr = sObjT.str();
    std::string funcName = demangle(typeid(*potFuncPtr).name());

//    std::string  initFuncName= demangle(typeid(initFuncName).name());
    std::string outDir = "./version1Data/1d/func" + funcName +"/row"+std::to_string(rowNum)+ "/T" + TStr + "/";

    std::string outUAllSubDir = outDir + "UAllPickle/";
//    std::string out_xA_AllSubDir = outDir + "xA_All/";
//    std::string out_xB_AllSubDir = outDir + "xB_All/";
//    std::string outLAllSubDir=outDir+"LAll/";


    std::string outUAllBinSubDir = outDir + "UAllBin/";
    std::string out_xA_AllBinSubDir = outDir + "xA_AllBin/";
    std::string out_xB_AllBinSubDir = outDir + "xB_AllBin/";
    std::string outLAllBinSubDir=outDir+"LAllBin/";
    const auto tMCStart{std::chrono::steady_clock::now()};

    std::cout << "remaining flush number: " << remainingFlushNum << std::endl;

    for (unsigned long long fls = 0; fls < remainingFlushNum; fls++) {
        std::vector<std::vector<double>> xA_AllPerFlush;
        //init xA
        xA_AllPerFlush.reserve(moveNumInOneFlush);
        std::vector<double> initVecVal(N, 0);
        for(int i=0;i<moveNumInOneFlush;i++){
            xA_AllPerFlush.push_back(initVecVal);
        }
        std::vector<std::vector<double>> xB_AllPerFlush;
        //init B
        xB_AllPerFlush.reserve(moveNumInOneFlush);
        for(int i=0;i<moveNumInOneFlush;i++){
            xB_AllPerFlush.push_back(initVecVal);
        }
        std::vector<double> UAllPerFlush;
        //init U
        UAllPerFlush.reserve(moveNumInOneFlush);
        for(int i=0;i<moveNumInOneFlush;i++){
            UAllPerFlush.push_back(0);
        }
        std::vector<double> LAllPerFlush;
        //init L
        LAllPerFlush.reserve(moveNumInOneFlush);
        for(int i=0;i<moveNumInOneFlush;i++){
            LAllPerFlush.push_back(0);
        }
        unsigned long long loopStart = loopEq + fls * moveNumInOneFlush;
        for (unsigned long long i = 0; i < moveNumInOneFlush; i++) {
            //propose a move
            arma::dcolvec xANext = arma::dcolvec(N);
            arma::dcolvec xBNext = arma::dcolvec(N);
            double LNext=0;
            proposal(xACurr, xBCurr,LCurr, xANext, xBNext,LNext);

            double r = acceptanceRatio(xACurr, xBCurr,LCurr, xANext, xBNext,LNext);
            double u = distUnif01(e2);
            counter++;
            if (u <= r) {
                xACurr = xANext;
                xBCurr = xBNext;
                LCurr=LNext;
                UCurr = (*potFuncPtr)(xACurr, xBCurr,LCurr);
            }

            xA_AllPerFlush[i]=(arma::conv_to<std::vector<double>>::from(xACurr));
            xB_AllPerFlush[i]=(arma::conv_to<std::vector<double>>::from(xBCurr));
            UAllPerFlush[i]=(UCurr);
            LAllPerFlush[i]=(LCurr);

        }//end for loop
        unsigned long long loopEnd = loopStart + moveNumInOneFlush - 1;
        std::string filenameMiddle = "loopStart" + std::to_string(loopStart) +
                                     "loopEnd" + std::to_string(loopEnd) + "T" + TStr;

//        std::string outUFileName = outUAllSubDir + filenameMiddle + ".UAll.xml";
//        this->saveVecToXML(outUFileName, UAllPerFlush);
        std::string outUBinFileName=outUAllBinSubDir+filenameMiddle+"UAll.bin";
        this->saveVecToBin(outUBinFileName,UAllPerFlush);

//        std::string out_xAFileName = out_xA_AllSubDir + filenameMiddle + ".xA_All.xml";
//        this->saveVecVecToXML(out_xAFileName, xA_AllPerFlush);
        std::string out_xABinFileName=out_xA_AllBinSubDir+filenameMiddle+".xA_All.bin";
        this->saveVecVecToBin(out_xABinFileName,xA_AllPerFlush);



//        std::string out_xBFileName = out_xB_AllSubDir + filenameMiddle + ".xB_All.xml";
//        this->saveVecVecToXML(out_xBFileName, xB_AllPerFlush);
        std::string out_xBBinFileName=out_xB_AllBinSubDir+filenameMiddle+".xB_All.bin";
        this->saveVecVecToBin(out_xBBinFileName,xB_AllPerFlush);

//        std::string outLFileName=outLAllSubDir+filenameMiddle+".LAll.xml";
//        this->saveVecToXML(outLFileName,LAllPerFlush);
        std::string outLBinFileName=outLAllBinSubDir+filenameMiddle+".LAll.bin";
        this->saveVecToBin(outLBinFileName,LAllPerFlush);


        const auto tflushEnd{std::chrono::steady_clock::now()};
        const std::chrono::duration<double> elapsed_seconds{tflushEnd - tMCStart};
        std::cout << "flush " << fls << std::endl;
        std::cout << "time elapsed: " << elapsed_seconds.count() / 3600.0 << " h" << std::endl;


    }//end of flush loop

    std::ofstream outSummary(outDir + "summaryAfterEq.txt");
    int loopTotal = counter;
    const auto tMCEnd{std::chrono::steady_clock::now()};
    const std::chrono::duration<double> elapsed_secondsAll{tMCEnd - tMCStart};
    outSummary << "total mc time: " << elapsed_secondsAll.count() / 3600.0 << " h" << std::endl;
    outSummary << "total loop number: " << loopTotal << std::endl;


}//end of function executionMCAfterEq
//void version1dLJPot2Atom::executionMCAfterEq(int &lag, const int &loopEq, const std::vector<double> &xA_init,
//                                             const std::vector<double> &xB_init, const double &LInit) {
//
//
//    bool active = true;
//    int fls = 0;
//    std::random_device rd;
//    std::ranlux24_base e2(rd());
//    std::uniform_real_distribution<> distUnif01(0, 1);//[0,1)
//    arma::dcolvec xACurr(xA_init);
//    arma::dcolvec xBCurr(xB_init);
//    double LCurr = LInit;
//    double UCurr = (*potFuncPtr)(xACurr, xBCurr, LCurr);
//    //output directory
//    std::ostringstream sObjT;
//    sObjT << std::fixed;
//    sObjT << std::setprecision(10);
//    sObjT << T;
//    std::string TStr = sObjT.str();
//    std::string funcName = demangle(typeid(*potFuncPtr).name());
//
////    std::string  initFuncName= demangle(typeid(initFuncName).name());
//    std::string outDir = "./version1Data/1d/func" + funcName + "/row" + std::to_string(rowNum) + "/T" + TStr + "/";
//
//    std::string outUAllSubDir = outDir +  "UAllPickle/";
////    std::string out_xA_AllSubDir = outDir + "xA_All/";
////    std::string out_xB_AllSubDir = outDir + "xB_All/";
//    std::string outLAllSubDir = outDir + "LAllPickle/";
//
//
//    std::string outUAllBinSubDir = outDir + "UAllBin/";
//    std::string out_xA_AllBinSubDir = outDir + "xA_AllBin/";
//    std::string out_xB_AllBinSubDir = outDir + "xB_AllBin/";
//    std::string outLAllBinSubDir = outDir + "LAllBin/";
//
//    std::regex wrongRegex("wrong");
//    std::regex ErrRegex("Err");
//    std::regex lagRegex("lag=\\s*(\\d+)");
//    std::regex statisticRegex("statistic");
//    std::regex pRegex("p=([+-]?(\\d+(\\.\\d*)?|\\.\\d+)([eE][+-]?\\d+)?)");
//    std::regex nDataPntRegex("numDataPoints=(\\d+)");
//
//    std::smatch matchUWrong;
//    std::smatch matchUErr;
//    std::smatch matchULag;
//    std::smatch matchStats;
//    std::smatch match_p;
//    std::smatch matchDataPntNum;
//
//
//    const auto tMCStart{std::chrono::steady_clock::now()};
//    while (active == true) {
//        std::vector<std::vector<double>> xA_AllPerFlush;
//        std::vector<std::vector<double>> xB_AllPerFlush;
//        std::vector<double> UAllPerFlush;
//        std::vector<double> LAllPerFlush;
//        int loopStart = loopEq + fls * moveNumInOneFlush;
//        for (int i = 0; i < moveNumInOneFlush; i++) {
//            //propose a move
//            arma::dcolvec xANext = arma::dcolvec(N);
//            arma::dcolvec xBNext = arma::dcolvec(N);
//            double LNext = 0;
//            proposal(xACurr, xBCurr, LCurr, xANext, xBNext, LNext);
//            double r = acceptanceRatio(xACurr, xBCurr, LCurr, xANext, xBNext, LNext);
//            double u = distUnif01(e2);
//            if (u <= r) {
//                xACurr = xANext;
//                xBCurr = xBNext;
//                LCurr = LNext;
//                UCurr = (*potFuncPtr)(xACurr, xBCurr, LCurr);
//            }
//            xA_AllPerFlush.push_back(arma::conv_to<std::vector<double>>::from(xACurr));
//            xB_AllPerFlush.push_back(arma::conv_to<std::vector<double>>::from(xBCurr));
//            UAllPerFlush.push_back(UCurr);
//            LAllPerFlush.push_back(LCurr);
//
//        }//end for loop in 1 flush
//        int loopEnd = loopStart + moveNumInOneFlush - 1;
//        std::string filenameMiddle = "loopStart" + std::to_string(loopStart) +
//                                     "loopEnd" + std::to_string(loopEnd) + "T" + TStr;
//        std::string outUPickleFileName = outUAllSubDir + filenameMiddle + ".UAll.pkl";
//        std::string outUBinFileName = outUAllBinSubDir + filenameMiddle + "UAll.bin";
////        this->saveVecToXML(outUFileName, UAllPerFlush);
//        save_vector_to_pickle(UAllPerFlush,outUPickleFileName);
//        this->saveVecToBin(outUBinFileName, UAllPerFlush);
//
////        std::string out_xAFileName = out_xA_AllSubDir + filenameMiddle + ".xA_All.xml";
////        this->saveVecVecToXML(out_xAFileName, xA_AllPerFlush);
//        std::string out_xABinFileName = out_xA_AllBinSubDir + filenameMiddle + ".xA_All.bin";
//        this->saveVecVecToBin(out_xABinFileName, xA_AllPerFlush);
//
////        std::string out_xBFileName = out_xB_AllSubDir + filenameMiddle + ".xB_All.xml";
////        this->saveVecVecToXML(out_xBFileName, xB_AllPerFlush);
//        std::string out_xBBinFileName = out_xB_AllBinSubDir + filenameMiddle + ".xB_All.bin";
//        this->saveVecVecToBin(out_xBBinFileName, xB_AllPerFlush);
//
//
//        std::string outLPickleFileName=outLAllSubDir+filenameMiddle+".LAll.pkl";
////        this->saveVecToXML(outLFileName, LAllPerFlush);
//        save_vector_to_pickle(LAllPerFlush,outLPickleFileName);
//        std::string outLBinFileName = outLAllBinSubDir + filenameMiddle + ".LAll.bin";
//        this->saveVecToBin(outLBinFileName, LAllPerFlush);
//
//        const auto tflushEnd{std::chrono::steady_clock::now()};
//
//        const std::chrono::duration<double> elapsed_seconds{tflushEnd - tMCStart};
//        std::cout << "flush " << fls << std::endl;
//        std::cout << "time elapsed: " << elapsed_seconds.count() / 3600.0 << " h" << std::endl;
//        //communicate with python to decide continuing
//
////        std::string commandU = "python3 setCounter.py " + outUAllSubDir + " " + std::to_string(nCounterStart);
////        std::string resultU;
////        if (fls % 10000 == 0 and fls > 9999) {
////            try {
////                const auto tPyStart{std::chrono::steady_clock::now()};
////                resultU = this->execPython(commandU.c_str());
////                std::cout << "U message from python: " << resultU << std::endl;
////                const auto tPyEnd{std::chrono::steady_clock::now()};
////
////                const std::chrono::duration<double> elapsedpy_secondsAll{tPyEnd - tPyStart};
////                std::cout << "py time: " << elapsedpy_secondsAll.count()  << " s" << std::endl;
////
////            }
////            catch (const std::exception &e) {
////                std::cerr << "Error: " << e.what() << std::endl;
////                std::exit(10);
////            }
////            catch (...) {
////                // Handle any other exceptions
////                std::cerr << "Error" << std::endl;
////                std::exit(11);
////            }
////            // parse result
////            if (std::regex_search(resultU, matchUErr, ErrRegex)) {
////                std::cout << "error encountered" << std::endl;
////                std::exit(12);
////            }
////
////            if (std::regex_search(resultU, matchUWrong, wrongRegex)) {
////                std::exit(13);
////            }
////
////            if (std::regex_search(resultU, matchStats, statisticRegex)) {
////                std::regex_search(resultU, match_p, pRegex);
////                double pVal = std::stod(match_p.str(1));
////
////                std::regex_search(resultU, matchDataPntNum, nDataPntRegex);
////                int dataPntNum = std::stoi(matchDataPntNum.str(1));
////
////                std::regex_search(resultU, matchULag, lagRegex);
////                lag = std::stoi(matchULag.str(1));
////
////                if (pVal <= 0.1) {
////                    nCounterStart += 1000;
//////                    continue;
////                } else {
////                    if (dataPntNum >= dataNumTotal) {
////                        active = false;
////                    }
////
////                }
////
////                std::cout<<"flush "<<fls<<", nCounterStart="<<nCounterStart<<std::endl;
////
////
////            }//end of regex search
////
////
////        }//end if
//        fls++;
//    }//end while
//
////    std::ofstream outSummary(outDir + "summaryAfterEq.txt");
////
////    const auto tMCEnd{std::chrono::steady_clock::now()};
////    const std::chrono::duration<double> elapsed_secondsAll{tMCEnd - tMCStart};
////    outSummary << "total mc time: " << elapsed_secondsAll.count() / 3600.0 << " h" << std::endl;
////
////    outSummary << "nCounterStart=" << std::to_string(nCounterStart) << std::endl;
////    outSummary<<"lag="<<lag<<std::endl;
////    outSummary.close();
//}//end of function executionMCAfterEq

///
/// @param rowNum row number
void version1dLJPot2Atom::parseCSV(const int &rowNum, double &alpha1, double &beta1, double &p1, double &q1,
                                   double &alpha2, double &beta2, double &p2, double &q2,double &r0) {

    std::string filePath = "./version1Input/1d/LJPotPBC/V1LJ2Atom1d.csv";
    std::string pyFile = "./version1/LJPotPBC/readCSV.py";
    std::string commandToReadCSV = "python3 " + pyFile + " " + filePath + " " + std::to_string(rowNum);

    std::string result = execPython(commandToReadCSV.c_str());

    std::regex pattern_alpha1("alpha1([+-]?\\d+(\\.\\d+)?)beta1");
    std::smatch match_alpha1;
    if (std::regex_search(result, match_alpha1, pattern_alpha1)) {
        alpha1 = std::stod(match_alpha1[1].str());
    }

    std::regex pattern_beta1("beta1([+-]?\\d+(\\.\\d+)?)p1");
    std::smatch match_beta1;
    if (std::regex_search(result, match_beta1, pattern_beta1)) {
        beta1 = std::stod(match_beta1[1].str());
    }

    std::regex pattern_p1("p1([+-]?\\d+(\\.\\d+)?)q1");
    std::smatch match_p1;
    if (std::regex_search(result, match_p1, pattern_p1)) {
        p1 = std::stod(match_p1[1].str());
    }

    std::regex pattern_q1("q1([+-]?\\d+(\\.\\d+)?)alpha2");
    std::smatch match_q1;
    if (std::regex_search(result, match_q1, pattern_q1)) {
        q1 = std::stod(match_q1[1].str());
    }

    std::regex pattern_alpha2("alpha2([+-]?\\d+(\\.\\d+)?)beta2");
    std::smatch match_alpha2;
    if (std::regex_search(result, match_alpha2, pattern_alpha2)) {
        alpha2 = std::stod(match_alpha2[1].str());
    }

    std::regex pattern_beta2("beta2([+-]?\\d+(\\.\\d+)?)p2");
    std::smatch match_beta2;
    if (std::regex_search(result, match_beta2, pattern_beta2)) {
        beta2 = std::stod(match_beta2[1].str());
    }

    std::regex pattern_p2("p2([+-]?\\d+(\\.\\d+)?)q2");
    std::smatch match_p2;
    if (std::regex_search(result, match_p2, pattern_p2)) {
        p2 = std::stod(match_p2[1].str());
    }

    std::regex pattern_q2("q2([+-]?\\d+(\\.\\d+)?)");
    std::smatch match_q2;
    if (std::regex_search(result, match_q2, pattern_q2)) {
        q2 = std::stod(match_q2[1].str());
    }

    std::regex pattern_x("x=([+-]?\\d+(\\.\\d+)?([eE][-+]?\\d+)?)");
    std::smatch match_x;
    if(std::regex_search(result,match_x,pattern_x)){
        r0=std::stod(match_x[1].str());

    }
    if (r0<0){
        r0=1;
    }


}


///
/// @param xAInit initial positions of A
/// @param xBInit initial positions of B
/// @param LInit
void version1dLJPot2Atom::initPositionsEquiDistance(arma::dcolvec &xAInit, arma::dcolvec &xBInit, double &LInit) {
    double a = 5;
    for (int n = 0; n < N; n++) {
        double nDB = static_cast<double >(n);
        xAInit(n) = a * nDB * 2.0;
        xBInit(n) = a * (2.0 * nDB + 1);
    }

    LInit=xBInit(N-1)+2*a;


}


///
/// @param filename bin file name of vec
/// @param vec vector to be saved
void version1dLJPot2Atom::saveVecToBin(const std::string &filename,const std::vector<double> &vec){
    std::stringstream buffer;
    msgpack::pack(buffer,vec);
    std::string const& serialized_data = buffer.str();

    std::ofstream outfile(filename, std::ios::binary);
    if (outfile.is_open()) {
        outfile.write(serialized_data.data(), serialized_data.size());
        outfile.close();

    } else {
        std::cerr << "Unable to open file for writing "<<filename << std::endl;
    }



}



///
/// @param filename bin file name of vecvec
/// @param vecvec vector<vector> to be saved
void version1dLJPot2Atom::saveVecVecToBin(const std::string &filename,const std::vector<std::vector<double>> &vecvec){


    std::stringstream buffer;
    msgpack::pack(buffer, vecvec);
    std::string const& serialized_data = buffer.str();

    std::ofstream outfile(filename, std::ios::binary);
    if (outfile.is_open()) {
        outfile.write(serialized_data.data(), serialized_data.size());
        outfile.close();
//        std::cout << "Data successfully written to " << filename << std::endl;
    } else {
        std::cerr << "Unable to open file for writing" << std::endl;
    }



}


void save_vector_to_pickle(const std::vector<double>& vec, const std::string& filename) {
    using namespace boost::python;
    Py_Initialize();  // Initialize the Python interpreter

    try {
        // Import the pickle module
        object pickle = import("pickle");
        object pickle_dumps = pickle.attr("dumps");

        // Create a Python list from the C++ vector
        list py_list;
        for (const double& value : vec) {
            py_list.append(value);
        }

        // Serialize the list using pickle.dumps
        object serialized_vec = pickle_dumps(py_list);

        // Extract the serialized data as a string
        std::string serialized_str = extract<std::string>(serialized_vec);

        // Write the serialized data to a file
        std::ofstream file(filename, std::ios::binary);
        file.write(serialized_str.data(), serialized_str.size());
        file.close();
    } catch (error_already_set) {
        PyErr_Print();
    }

    Py_Finalize();  // Finalize the Python interpreter
}