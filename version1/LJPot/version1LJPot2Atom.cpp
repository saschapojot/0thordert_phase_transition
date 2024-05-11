#include "version1LJPot2Atom.hpp"



///
/// @param xA positions of atom A
/// @param xB positions of atom B
/// @return beta*potential
double version1dLJPot2Atom::f(const arma::dcolvec& xA, const arma::dcolvec& xB){
    return this->beta*((*potFuncPtr)(xA,xB));

}



///
/// @param xACurr positions of atom A
/// @param xBCurr positions of atom B
/// @param zANext proposed positions of atom A
/// @param zBNext proposed positions of atom B
void version1dLJPot2Atom::proposal(const arma::dcolvec& xACurr, const arma::dcolvec& xBCurr,
              arma::dcolvec& zANext, arma::dcolvec& zBNext){
    std::random_device rd;
    std::ranlux24_base gen(rd());

    for(int j=0;j<N;j++){
        std::normal_distribution<double>  dTmp(xACurr(j),stddev);
        zANext(j)=dTmp(gen);
    }


    for(int j=0;j<N;j++){
        std::normal_distribution<double>  dTmp(xBCurr(j),stddev);
        zBNext(j)=dTmp(gen);
    }

}



///
/// @param cmd python execution string
/// @return signal from the python
std::string version1dLJPot2Atom::execPython(const char* cmd){
    std::array<char, 4096> buffer; // Buffer to store command output
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
/// @param zA proposed positions of atom A
/// @param zB proposed positions of atom B
/// @return
double version1dLJPot2Atom::acceptanceRatio(const arma::dcolvec& xA,const arma::dcolvec& xB,
                       const arma::dcolvec& zA,const arma::dcolvec& zB){

    double numerator=-f(zA,zB);

    double denominator=-f(xA,xB);


    double ratio=std::exp(numerator-denominator);

    return std::min(1.0,ratio);

}


///
/// @param filename xml file name of vec
///@param vec vector to be saved
void version1dLJPot2Atom::saveVecToXML(const std::string &filename,const std::vector<double> &vec){

    std::ofstream ofs(filename);
    boost::archive::xml_oarchive oa(ofs);
    oa & BOOST_SERIALIZATION_NVP(vec);

}


///
/// @param filename  xml file name of vecvec
/// @param vecvec vector<vector> to be saved
void version1dLJPot2Atom::saveVecVecToXML(const std::string &filename,const std::vector<std::vector<double>> &vecvec){
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
void version1dLJPot2Atom::readEqMc(int& lag,int &loopTotal,bool &equilibrium, bool &same, std::vector<double>& xALast,std::vector<double>& xBLast){
    std::random_device rd;
    std::ranlux24_base e2(rd());
    std::uniform_real_distribution<> distUnif01(0, 1);//[0,1)

    //generate initial values of positions
    double leftEnd=-300.0;
    double rightEnd=300.0;
    std::uniform_real_distribution<> distUnifLeftRight(leftEnd,rightEnd);

    arma::dcolvec  xACurr(N);
    arma::dcolvec xBCurr(N);

    for(int j=0;j<N;j++){
        xACurr(j)=distUnifLeftRight(e2);
        xBCurr(j)=distUnifLeftRight(e2);
    }
    //initial value of energy
    double UCurr=(*potFuncPtr)(xACurr,xBCurr);
    //output directory
    std::ostringstream sObjT;
    sObjT << std::fixed;
    sObjT << std::setprecision(10);
    sObjT << T;
    std::string TStr = sObjT.str();
    std::string  funcName= demangle(typeid(*potFuncPtr).name());

    std::string outDir="./version1Data/1d/func"+funcName+"/T"+TStr+"/";

    std::string outUAllSubDir=outDir+"UAll/";
    std::string out_xA_AllSubDir=outDir+"xA_All/";
    std::string out_xB_AllSubDir=outDir+"xB_All/";

    if (!fs::is_directory(outUAllSubDir) || !fs::exists(outUAllSubDir)) {
        fs::create_directories(outUAllSubDir);
    }

    if (!fs::is_directory(out_xA_AllSubDir) || !fs::exists(out_xA_AllSubDir)) {
        fs::create_directories(out_xA_AllSubDir);
    }

    if (!fs::is_directory(out_xB_AllSubDir) || !fs::exists(out_xB_AllSubDir)) {
        fs::create_directories(out_xB_AllSubDir);
    }

    std::regex stopRegex("stop");
    std::regex wrongRegex("wrong");
    std::regex ErrRegex("Err");
    std::regex lagRegex("lag=\\s*(\\d+)");
    std::regex fileNumRegex("fileNum=\\s*(\\d+)");
    std::regex sameRegex("same");
    std::regex eqRegex("equilibrium");

    std::smatch matchUStop;
    std::smatch matchUWrong;
    std::smatch matchUErr;
    std::smatch matchULag;
    std::smatch matchFileNum;
    std::smatch matchUSame;
    std::smatch matchUEq;


    int counter = 0;
    int fls = 0;
    bool active = true;
    const auto tMCStart{std::chrono::steady_clock::now()};
    std::vector<double> last_xA;
    std::vector<double> last_xB;

    while(fls<this->flushMaxNum and active==true){
        std::vector<std::vector<double>>xA_AllPerFlush;
        std::vector<std::vector<double>>xB_AllPerFlush;
        std::vector<double> UAllPerFlush;

        int loopStart=fls*moveNumInOneFlush;

        for(int i=0;i<moveNumInOneFlush;i++){
            //propose a move
            arma::dcolvec xANext=arma::dcolvec(N);
            arma::dcolvec xBNext=arma::dcolvec(N);
            proposal(xACurr,xBCurr,xANext,xBNext);

            double r= acceptanceRatio(xACurr,xBCurr,xANext,xBNext);
            double u = distUnif01(e2);
            counter++;
            if(u<=r){
                xACurr=xANext;
                xBCurr=xBNext;
                UCurr=(*potFuncPtr)(xACurr,xBCurr);
            }

            xA_AllPerFlush.push_back(arma::conv_to<std::vector<double>>::from(xACurr));
            xB_AllPerFlush.push_back(arma::conv_to<std::vector<double>>::from(xBCurr));
            UAllPerFlush.push_back(UCurr);


        }//end for loop in 1 flush

        int loopEnd = loopStart +moveNumInOneFlush-1;
        std::string filenameMiddle = "loopStart" + std::to_string(loopStart) +
                                     "loopEnd" + std::to_string(loopEnd) + "T" + TStr;

        std::string outUFileName=outUAllSubDir+filenameMiddle+".UAll.xml";
        this->saveVecToXML(outUFileName,UAllPerFlush);

        std::string out_xAFileName=out_xA_AllSubDir+filenameMiddle+".xA_All.xml";
        this->saveVecVecToXML(out_xAFileName,xA_AllPerFlush);

        std::string out_xBFileName=out_xB_AllSubDir+filenameMiddle+".xB_All.xml";
        this->saveVecVecToXML(out_xBFileName,xB_AllPerFlush);

        const auto tflushEnd{std::chrono::steady_clock::now()};

        const std::chrono::duration<double> elapsed_seconds{tflushEnd - tMCStart};
        std::cout << "flush " << fls << std::endl;
        std::cout << "time elapsed: " << elapsed_seconds.count() / 3600.0 << " h" << std::endl;

        //communicate with python to inquire equilibrium

        //inquire equilibrium of U
        std::string commandU = "python3 checkVec.py " + outUAllSubDir;
        std::string resultU;

        if (fls % 6 == 5) {
            try {
                resultU = this->execPython(commandU.c_str());
                std::cout << "U message from python: " << resultU << std::endl;

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
            if (std::regex_search(resultU, matchUStop, stopRegex)){
                if (std::regex_search(resultU, matchUSame, sameRegex)){
                    active = false;
                    same=true;
                    std::regex_search(resultU, matchFileNum, fileNumRegex);
                    std::string fileNumStr = matchFileNum.str(1);
                    this->lastFileNum = std::stoi(fileNumStr);

                    last_xA=xA_AllPerFlush[xA_AllPerFlush.size()-1];
                    last_xB=xB_AllPerFlush[xB_AllPerFlush.size()-1];


                }


            }//end of regex search

            if (std::regex_search(resultU, matchUEq, eqRegex)){
                if (std::regex_search(resultU, matchULag, lagRegex)){

                    std::string lagStrU = matchULag.str(1);
                    int lagU = std::stoi(lagStrU);
                    std::cout << "lag=" << lagU << std::endl;
                    lag=lagU;
                    std::regex_search(resultU, matchFileNum, fileNumRegex);
                    std::string fileNumStr = matchFileNum.str(1);
                    this->lastFileNum = std::stoi(fileNumStr);
                    active = false;
                    last_xA=xA_AllPerFlush[xA_AllPerFlush.size()-1];
                    last_xB=xB_AllPerFlush[xB_AllPerFlush.size()-1];
                }


            }//end of regex search

        }//end if
        fls++;



    }//end while

    loopTotal=counter;
    std::ofstream  outSummary(outDir+"summary.txt");
    const auto tMCEnd{std::chrono::steady_clock::now()};
    const std::chrono::duration<double> elapsed_secondsAll{tMCEnd - tMCStart};
    outSummary << "total mc time: " << elapsed_secondsAll.count() / 3600.0 << " h" << std::endl;
    outSummary << "total loop number: " << loopTotal << std::endl;

    outSummary << "lastFileNum=" << lastFileNum << std::endl;
    outSummary << "equilibrium reached: " << !active << std::endl;
    outSummary << "same: " << same << std::endl;

    outSummary << "lag=" << lag << std::endl;
    outSummary.close();

    equilibrium=!active;
    xALast=last_xA;
    xBLast=last_xB;


}//end of reachMC function





///
/// @param lag decorrelation length
/// @param loopEq total loop numbers in reaching equilibrium
/// @param xA_init xA from readEqMc
/// @param xB_init xB from readEqMc
void version1dLJPot2Atom::executionMCAfterEq(const int& lag,const int & loopEq,const std::vector<double>& xA_init,const std::vector<double>& xB_init){

    int counter=0;
    int remainingDataNum = this->dataNumTotal-static_cast<int>(std::floor(lastFileNum*moveNumInOneFlush/lag));

    int remainingLoopNum = remainingDataNum * lag;
    if (remainingLoopNum <= 0) {
        return;
    }

    double remainingLoopNumDB = static_cast<double>(remainingLoopNum);
    double remainingFlushNumDB = std::ceil(remainingLoopNumDB/moveNumInOneFlush);
    int remainingFlushNum = static_cast<int>(remainingFlushNumDB);
    std::random_device rd;
    std::ranlux24_base e2(rd());
    std::uniform_real_distribution<> distUnif01(0, 1);//[0,1)

    arma::dcolvec xACurr(xA_init);
    arma::dcolvec xBCurr(xB_init);

    double UCurr=(*potFuncPtr)(xACurr,xBCurr);
    //output directory
    std::ostringstream sObjT;
    sObjT << std::fixed;
    sObjT << std::setprecision(10);
    sObjT << T;
    std::string TStr = sObjT.str();
    std::string  funcName= demangle(typeid(*potFuncPtr).name());

    std::string outDir="./version1Data/1d/func"+funcName+"/T"+TStr+"/";

    std::string outUAllSubDir=outDir+"UAll/";
    std::string out_xA_AllSubDir=outDir+"xA_All/";
    std::string out_xB_AllSubDir=outDir+"xB_All/";
    const auto tMCStart{std::chrono::steady_clock::now()};

    std::cout<<"remaining flush number: "<<remainingFlushNum<<std::endl;

    for (int fls = 0; fls < remainingFlushNum; fls++) {
        std::vector<std::vector<double>>xA_AllPerFlush;
        std::vector<std::vector<double>>xB_AllPerFlush;
        std::vector<double> UAllPerFlush;
        int loopStart =loopEq+fls*moveNumInOneFlush;
        for(int i=0;i<moveNumInOneFlush;i++){
            //propose a move
            arma::dcolvec xANext=arma::dcolvec(N);
            arma::dcolvec xBNext=arma::dcolvec(N);
            proposal(xACurr,xBCurr,xANext,xBNext);

            double r= acceptanceRatio(xACurr,xBCurr,xANext,xBNext);
            double u = distUnif01(e2);
            counter++;
            if(u<=r){
                xACurr=xANext;
                xBCurr=xBNext;
                UCurr=(*potFuncPtr)(xACurr,xBCurr);
            }

            xA_AllPerFlush.push_back(arma::conv_to<std::vector<double>>::from(xACurr));
            xB_AllPerFlush.push_back(arma::conv_to<std::vector<double>>::from(xBCurr));
            UAllPerFlush.push_back(UCurr);

        }//end for loop
        int loopEnd = loopStart +moveNumInOneFlush-1;
        std::string filenameMiddle = "loopStart" + std::to_string(loopStart) +
                                     "loopEnd" + std::to_string(loopEnd) + "T" + TStr;

        std::string outUFileName=outUAllSubDir+filenameMiddle+".UAll.xml";
        this->saveVecToXML(outUFileName,UAllPerFlush);

        std::string out_xAFileName=out_xA_AllSubDir+filenameMiddle+".xA_All.xml";
        this->saveVecVecToXML(out_xAFileName,xA_AllPerFlush);

        std::string out_xBFileName=out_xB_AllSubDir+filenameMiddle+".xB_All.xml";
        this->saveVecVecToXML(out_xBFileName,xB_AllPerFlush);
        const auto tflushEnd{std::chrono::steady_clock::now()};
        const std::chrono::duration<double> elapsed_seconds{tflushEnd - tMCStart};
        std::cout << "flush " << fls << std::endl;
        std::cout << "time elapsed: " << elapsed_seconds.count() / 3600.0 << " h" << std::endl;




    }//end of flush loop

    std::ofstream outSummary(outDir + "summaryAfterEq.txt");
    int loopTotal=counter;
    const auto tMCEnd{std::chrono::steady_clock::now()};
    const std::chrono::duration<double> elapsed_secondsAll{tMCEnd - tMCStart};
    outSummary << "total mc time: " << elapsed_secondsAll.count() / 3600.0 << " h" << std::endl;
    outSummary << "total loop number: " << loopTotal << std::endl;







}//end of function executionMCAfterEq

///
/// @param rowNum row number
void version1dLJPot2Atom::parseCSV(const int& rowNum,double &alpha1,double& beta1, double& p1,double&q1,
                                   double &alpha2,double&beta2,double & p2, double& q2){
    std::string filePath="./version1Input/LJPot/1d/V1LJ2Atom1d.csv";
    std::string pyFile="./version1/LJPot/readCSV.py";
    std::string commandToReadCSV="python3 "+pyFile+" "+filePath+" "+std::to_string(rowNum);

    std::string result=execPython(commandToReadCSV.c_str());

    std::regex pattern_alpha1("alpha1([+-]?\\d+(\\.\\d+)?)beta1");
    std::smatch match_alpha1;
    if(std::regex_search(result,match_alpha1,pattern_alpha1)){
        alpha1=std::stod(match_alpha1[1].str());
    }

    std::regex  pattern_beta1("beta1([+-]?\\d+(\\.\\d+)?)p1");
    std::smatch  match_beta1;
    if(std::regex_search(result,match_beta1,pattern_beta1)){
        beta1=std::stod(match_beta1[1].str());
    }

    std::regex pattern_p1("p1([+-]?\\d+(\\.\\d+)?)q1");
    std::smatch match_p1;
    if(std::regex_search(result,match_p1,pattern_p1)){
        p1=std::stod(match_p1[1].str());
    }

    std::regex pattern_q1("q1([+-]?\\d+(\\.\\d+)?)alpha2");
    std::smatch  match_q1;
    if(std::regex_search(result,match_q1,pattern_q1)){
        q1=std::stod(match_q1[1].str());
    }

    std::regex pattern_alpha2("alpha2([+-]?\\d+(\\.\\d+)?)beta2");
    std::smatch match_alpha2;
    if(std::regex_search(result,match_alpha2,pattern_alpha2)){
        alpha2=std::stod(match_alpha2[1].str());
    }

    std::regex pattern_beta2("beta2([+-]?\\d+(\\.\\d+)?)p2");
    std::smatch match_beta2;
    if(std::regex_search(result,match_beta2,pattern_beta2)){
        beta2=std::stod(match_beta2[1].str());
    }

    std::regex pattern_p2("p2([+-]?\\d+(\\.\\d+)?)q2");
    std::smatch match_p2;
    if(std::regex_search(result,match_p2,pattern_p2)){
        p2=std::stod(match_p2[1].str());
    }

    std::regex pattern_q2("q2([+-]?\\d+(\\.\\d+)?)");
    std::smatch match_q2;
    if(std::regex_search(result,match_q2,pattern_q2)){
        q2=std::stod(match_q2[1].str());
    }


}