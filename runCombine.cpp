#include "version1/LJPotPBC/combineVecs.hpp"

int main(int argc, char *argv[]) {

    if (argc != 2) {
        std::cerr << "wrong number of arguments" << std::endl;
        exit(1);
    }

    std::string inTDir=std::string(argv[1]);
    loader ld=loader(inTDir);
    ld.searchFiles();
    ld.sortFiles();
    ld.parseUFiles();
    ld.toPickle(inTDir+"/U.pkl");

}