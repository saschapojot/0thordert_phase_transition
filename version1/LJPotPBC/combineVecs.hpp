//
// Created by polya on 5/22/24.
//

#ifndef T_PHASE_TRANSITION_COMBINEVECS_HPP
#define T_PHASE_TRANSITION_COMBINEVECS_HPP

#include "version1LJPotPBC2Atom.hpp"

// This subroutine loads pieces of U vectors and combine them to a single pickle file
class loader{

public:
    loader(const std::string &TDir){
        this->TDir=TDir;
        this->UPath=TDir+ "/UAllBin/";

    }

public:
    ///search U files
    void searchFiles();


    template<class T>
    std::vector<size_t> argsort(const std::vector<T> &v) {
        std::vector<size_t> idx(v.size());
        std::iota(idx.begin(), idx.end(), 0);
        std::stable_sort(idx.begin(), idx.end(), [&v](size_t i1, size_t i2) { return v[i1] <= v[i2]; });
        return idx;
    }

    ///
    /// @param path the path containing bin files
    /// @return sorted bin files by starting loop
    std::vector<std::string> sortOneDir(const std::vector<std::string> &allFiles);


    ///sort files by starting loop
    void sortFiles();

    void parseUFiles();

    ///
    /// @param filename bin file name
    /// @return vector
    static std::vector<double> readMsgBinVec(const std::string &filename);

    void toPickle(const std::string & outUPickleName);



public:
    std::string UPath;
    std::vector<std::string> UFilesAll;
    std::vector<std::string> sorted_UFilesAll;
    int moveNumInOneFlush = 3000;
    std::vector<double> UIn;
    std::string TDir;
};




#endif //T_PHASE_TRANSITION_COMBINEVECS_HPP
