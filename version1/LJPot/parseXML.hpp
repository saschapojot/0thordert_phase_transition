//
// Created by polya on 5/13/24.
//

#ifndef T_PHASE_TRANSITION_PARSEXML_HPP
#define T_PHASE_TRANSITION_PARSEXML_HPP
#include "version1LJPot2Atom.hpp"
//this subroutine parses xml or bin files, we parse bin files for speed

class reader{

public:
    reader(const int &rowNum, const std::string &TDir){
        this->rowNum=rowNum;
        this->TDir ="./version1Data/1d/row"+std::to_string(rowNum)+"/funcLJPot/"+TDir;
        std::regex TPattern("T([+-]?\\d*(\\.\\d+)?)");
        std::smatch T_match;
        if (std::regex_search(TDir, T_match, TPattern)) {

            this->T = std::stod(T_match.str(1));
//            std::cout<<T<<std::endl;
            this->beta = 1.0 / T;
        }
//                std::cout<<TDir<<std::endl;
//        std::cout<<T<<std::endl;
    }//end of constructor

public:

    ///UAll, xA_All, xB_All folder's files
    void searchFiles();

    ///
    /// @param path the path containing xml files
    /// @return sorted xml files by starting loop
    std::vector <std::string> sortOneDir(const std::vector <std::string> &allFiles);

    template<class T>
    std::vector <size_t> argsort(const std::vector <T> &v) {
        std::vector <size_t> idx(v.size());
        std::iota(idx.begin(), idx.end(), 0);
        std::stable_sort(idx.begin(), idx.end(), [&v](size_t i1, size_t i2) { return v[i1] <= v[i2]; });
        return idx;
    }

    template<class T>
    static void printVec(const std::vector <T> &vec) {
        for (int i = 0; i < vec.size() - 1; i++) {
            std::cout << vec[i] << ",";
        }
        std::cout << vec[vec.size() - 1] << std::endl;
    }

    ///sort files by starting loop
    void sortFiles();



    void parseSummary();

    std::string searchSummaryAfterEq();

    void parseSummaryAfterEq(const std::string &afterEqPath);

    void UAndxFilesSelected();

    ///
    /// @param filename bin file name
    /// @return vector
    static std::vector<double> readMsgBinVec(const std::string& filename);

    ///
    /// @param filename bin file name
    /// @return vector<vector<double>>
    static std::vector<std::vector<double>> readMsgBinVecVec(const std::string& filename);


    void parseUFiles();

    void parsexAxB();





public:
    int rowNum;
    std::string TDir;
    std::string UPath;
    std::string xAPath;
    std::string xBPath;
    int lag = 0;
    int lastFileNum = 0;
    double T = 0;
    double beta = 0;
    int moveNumInOneFlush=3000;
    int loopNumAfterEq=0;
    int cellNum=0;
    std::vector <std::string> UFilesSelected;
    std::vector <std::string> xAFilesSelected;
    std::vector <std::string> xBFilesSelected;

    std::vector<double> xASelectedFlat;
    std::vector<double> xBSelectedFlat;
    std::vector<double> USelected;


    arma::dcolvec armaU;
    arma::dmat arma_xA;
    arma::dmat  arma_xB;

    std::vector <std::vector<double>> xAIn;
    std::vector <std::vector<double>> xBIn;
    std::vector<double> UIn;

    std::vector <std::string> UFilesAll;
    std::vector <std::string> xAFilesAll;
    std::vector <std::string> xBFilesAll;
    std::vector <std::string> sorted_UFilesAll;
    std::vector <std::string> sorted_xAFilesAll;
    std::vector <std::string> sorted_xBFilesAll;
    int loopNumToInclude;
    int fileNumSelected;


};



#endif //T_PHASE_TRANSITION_PARSEXML_HPP
