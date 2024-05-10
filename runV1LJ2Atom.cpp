#include "./version1/LJPot/version1LJPot2Atom.hpp"

//running version 1, Lennard-Jones

int main(int argc, char *argv[]) {
arma::dcolvec xA{1.0,2.0,3.0,5.0};


    arma::dcolvec xB{-9.0,-8.0,-7.0,-6.0};
    double  alpha1=0.1;
    double alpha2=1.2;
    double beta1=8;
    double beta2=0.4;

    double p1=2;
    double q1=1;

    double p2=3;
    double  q2=2;
    std::cout.precision(11);
    auto LJFunc=LJPot(alpha1,alpha2,beta1,beta2,p1,p2,q1,q2);
    double T=0.7;
    double  h=0.005;
    auto v1Obj=version1dLJPot2Atom(T,h,10,std::make_shared<LJPot>(alpha1,alpha2,beta1,beta2,p1,p2,q1,q2));
    std::cout<<v1Obj.f(xA,xB)<<std::endl;
    return 0;
}