//
// Created by polya on 5/13/24.
//

#ifndef T_PHASE_TRANSITION_PARSEXML_HPP
#define T_PHASE_TRANSITION_PARSEXML_HPP
#include "version1LJPot2Atom.hpp"

class reader{

public:
    reader(const int &rowNum, const std::string &TDir){
        this->rowNum=rowNum;
        this->TDir ="./version1Data/1d/row"+std::to_string(rowNum)+"/funcLJPot/initstd::string/"+TDir;
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

    ///sort files by starting loop
    void sortFiles();



    void parseSummary();





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
    std::vector <std::vector<double>> xASelected;
    std::vector <std::vector<double>> xBSelected;
    std::vector<double> ESelected;

    std::vector <std::string> UFilesAll;
    std::vector <std::string> xAFilesAll;
    std::vector <std::string> xBFilesAll;
    std::vector <std::string> sorted_UFilesAll;
    std::vector <std::string> sorted_xAFilesAll;
    std::vector <std::string> sorted_xBFilesAll;


};



#endif //T_PHASE_TRANSITION_PARSEXML_HPP
