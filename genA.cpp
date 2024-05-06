#include "./1d/1d.hpp"

void printMat(const arma::dmat &mat,int n){
    for(int i=0;i<n;i++){
        for(int j=0;j<n-1;j++){
            std::cout<<mat(i,j)<<",";
        }
        std::cout<<mat(i,n-1)<<std::endl;
    }
}


int main(int argc, char *argv[]) {

    int pntNum=10;
    auto pd=pdQuadratic(pntNum);
    printMat(pd.B,pntNum);
}