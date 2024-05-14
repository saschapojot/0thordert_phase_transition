#include "./version1/LJPot/version1LJPot2Atom.hpp"

//running version 1, Lennard-Jones



int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cout << "wrong arguments" << std::endl;
        std::exit(2);
    }
    double T = std::stod(argv[1]);
    int rowNum=std::stoi(argv[2]);

    double  alpha1;
    double alpha2;
    double beta1;
    double beta2;

    double p1;
    double q1;

    double p2;
    double  q2;
    version1dLJPot2Atom::parseCSV(rowNum,alpha1,beta1,p1,q1,alpha2,beta2,p2,q2);

//    std::cout<<"alpha1="<<alpha1<<", beta1="<<beta1
//    <<", p1="<<p1<<", q1="<<q1
//    <<", alpha2="<<alpha2<<", beta2="<<beta2
//    <<", p2="<<p2<<", q2="<<q2<<std::endl;
    std::cout.precision(11);
    auto LJFunc=LJPot(alpha1,alpha2,beta1,beta2,p1,p2,q1,q2);

    double  h=0.005;
    int cellNum = 2;
    auto v1Obj=version1dLJPot2Atom(rowNum,T,h,cellNum,std::make_shared<LJPot>(alpha1,alpha2,beta1,beta2,p1,p2,q1,q2));
    int lag=-1;
    int totalLoopEq=0;
    bool eq=false;
    bool same= false;
    std::vector<double> last_xA;
    std::vector<double>last_xB;

    v1Obj.readEqMc(lag,totalLoopEq,eq,same,last_xA,last_xB);
    if(!same and lag>0 and eq){
        v1Obj.executionMCAfterEq(lag,totalLoopEq,last_xA,last_xB);
    }

    return 0;
}