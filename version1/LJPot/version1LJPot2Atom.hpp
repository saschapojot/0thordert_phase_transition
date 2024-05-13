//
// Created by polya on 5/10/24.
//

#ifndef T_PHASE_TRANSITION_VERSION1LJPOT2ATOM_HPP
#define T_PHASE_TRANSITION_VERSION1LJPOT2ATOM_HPP
#include <algorithm>
#include <armadillo>
#include <array>

#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/filesystem.hpp>
#include <boost/serialization/complex.hpp>
#include <boost/serialization/vector.hpp>

#include <cmath>
#include <chrono>
#include <cstdlib>
#include <cxxabi.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <random>
#include <regex>
#include <sstream>
#include <string>
#include <typeinfo>
#include <vector>

namespace fs = boost::filesystem;
//this subroutine computes the mc evolution for a 1d system, 2-atom, Lennard-Jones potential

class potentialFunction{
    //base class for potential function
public:
    virtual double operator() (const arma::dcolvec& xA, const arma::dcolvec & xB)const=0;
    virtual ~ potentialFunction(){};

};

class LJPot:public potentialFunction{
public:
    LJPot(const double& alpha1Val,const double& alpha2Val,const double& beta1Val,
          const double& beta2Val, const double& p1Val,const double&p2Val,const double& q1Val,const double&q2Val){

        this->alpha1=alpha1Val;
        this->alpha2=alpha2Val;
        this->beta1=beta1Val;
        this->beta2=beta2Val;
        this->p1=p1Val;
        this->p2=p2Val;
        this->q1=q1Val;
        this->q2=q2Val;

    }//end of constructor

public:
    ///
    /// @param xA positions of atom A
    /// @param xB positions of atom B
    /// @return potential energy
    double operator() (const arma::dcolvec& xA, const arma::dcolvec & xB)const override{
        return V1Total(xA,xB)+ V2Total(xA,xB);

    }



    ///
    /// @param xA positions of atom A
    /// @param xB positions of atom B
    /// @return the sum of all V1 energy
    double V1Total(const arma::dcolvec& xA, const arma::dcolvec & xB)const {
        arma::dcolvec rVec=arma::abs(xA-xB);



        arma::dcolvec vecPart1=alpha1*arma::pow(rVec,-p1);
        arma::dcolvec vecPart2=-beta1*arma::pow(rVec,-q1);

        double val=arma::sum(vecPart1)+arma::sum(vecPart2);
//        std::cout<<"V1Total="<<val<<std::endl;
        return val;


    }

    ///
    /// @param xA positions of atom A
    /// @param xB positions of atom B
    /// @return the sum of all V2 energy under OBC
    double V2Total(const arma::dcolvec& xA, const arma::dcolvec & xB)const{
    int N=xB.size();
    arma::dcolvec sliceA=xA.subvec(1,N-1);
//    std::cout<<"sliceA="<<sliceA<<std::endl;
    arma::dcolvec sliceB=xB.subvec(0,N-2);
    arma::dcolvec rVec=arma::abs(sliceA-sliceB);
//        std::cout<<"sliceB="<<sliceB<<std::endl;

//        std::cout<<"V2Total: rVec="<<rVec<<std::endl;

    arma::dcolvec vecPart1=alpha2*arma::pow(rVec,-p2);
    arma::dcolvec vecPart2=-beta2*arma::pow(rVec,-q2);

    double val=arma::sum(vecPart1)+arma::sum(vecPart2);
//        std::cout<<"V2Total="<<val<<std::endl;

        return val;

    }




public:
    double alpha1=0;
    double alpha2=0;
    double beta1=0;
    double beta2=0;
    double p1=0;
    double p2=0;
    double q1=0;
    double q2=0;

};


class version1dLJPot2Atom {
public:
    version1dLJPot2Atom(double temperature, double stepSize, int cellNum,
                        const std::shared_ptr<potentialFunction> &funcPtr) {
        this->T = temperature;
        this->beta = 1 / T;
        this->h = stepSize;
        this->potFuncPtr = funcPtr;

//        this->diag=isDiag;
        this->N = cellNum;
        this->stddev=std::sqrt(2.0*h);
    }


public:

    ///
    /// @param xA positions of atom A
    /// @param xB positions of atom B
    /// @return beta*potential
    double f(const arma::dcolvec& xA, const arma::dcolvec& xB);

    ///
    /// @param xACurr positions of atom A
    /// @param xBCurr positions of atom B
    /// @param zANext proposed positions of atom A
    /// @param zBNext proposed positions of atom B
    void proposal(const arma::dcolvec& xACurr, const arma::dcolvec& xBCurr,
                  arma::dcolvec& zANext, arma::dcolvec& zBNext);



    ///
    /// @param filename xml file name of vec
    ///@param vec vector to be saved
    static  void saveVecToXML(const std::string &filename,const std::vector<double> &vec);

    ///
    /// @param cmd python execution string
    /// @return signal from the python
    static std::string execPython(const char* cmd);


    ///
    /// @param xA current positions of atom A
    /// @param xB current positions of atom B
    /// @param zA proposed positions of atom A
    /// @param zB proposed positions of atom B
    /// @return
    double acceptanceRatio(const arma::dcolvec& xA,const arma::dcolvec& xB,
                           const arma::dcolvec& zA,const arma::dcolvec& zB);



    ///
    /// @param filename  xml file name of vecvec
    /// @param vecvec vector<vector> to be saved
    static void saveVecVecToXML(const std::string &filename,const std::vector<std::vector<double>> &vecvec);


    ///
    /// @param lag decorrelation length
    /// @param loopTotal total mc steps
    /// @param equilibrium whether equilibrium has reached
    /// @param same whether all values of potential are the same
    /// @param xALast last positions of atom A
    /// @param xBLast last positions of atom B
    void readEqMc(int& lag,int &loopTotal,bool &equilibrium, bool &same, std::vector<double>& xALast,std::vector<double>& xBLast);

    ///
    /// @param lag decorrelation length
    /// @param loopEq total loop numbers in reaching equilibrium
    /// @param xA_init xA from readEqMc
    /// @param xB_init xB from readEqMc
    void executionMCAfterEq(const int& lag,const int & loopEq,const std::vector<double>& xA_init,const std::vector<double>& xB_init);

    std::string demangle(const char* name) {
        int status = -1;
        char* demangled = abi::__cxa_demangle(name, NULL, NULL, &status);
        std::string result(name);
        if (status == 0) {
            result = demangled;
        }
        std::free(demangled);
        return result;
    }

    ///
    /// @param rowNum row number
    static void parseCSV(const int& rowNum,double &alpha1,double& beta1, double& p1,double&q1,
                         double &alpha2,double&beta2,double & p2, double& q2);

    double T;// temperature
    double beta;
    int moveNumInOneFlush = 3000;// flush the results to python every moveNumInOneFlush iterations
    int flushMaxNum = 700;
    int dataNumTotal = 8000;
    double h;// step size
//    double a=5.3;//stiffness
//    bool diag=true;// whether the quadratic form of energy is diagonal
    int N;//number of unit cells

    double lastFileNum = 0;
    std::shared_ptr<potentialFunction> potFuncPtr;
    double stddev;


};


#endif //T_PHASE_TRANSITION_VERSION1LJPOT2ATOM_HPP