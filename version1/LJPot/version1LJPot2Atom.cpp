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
void version1dLJPot2Atom::readEqMc(int& lag,int &loopTotal,bool &equilibrium, bool &same, arma::dcolvec& xALast,arma::dcolvec& xBLast){
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




}
