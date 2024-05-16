./1d/ is the directory for code testing MC algorithm
for 1d:
1. ./run1d temperature a, to execute mc computations


./version1/ is the directory for computation version1
./version1/LJPot/ is the directory for Lennard-Jones potential
1. ./runV1LJ2Atom T rowNum, to execute mc computations
2. ./runV1LJ2AtomParseXML rowNum, to transform data to json
3. cd ./version1/LJPot, python  json2plt.py, to plot 