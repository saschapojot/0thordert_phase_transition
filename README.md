./1d/ is the directory for code testing MC algorithm
for 1d:
1. ./run1d temperature a, to execute mc computations


./version1/ is the directory for computation version1
./version1/LJPotPBC/ is the directory for Lennard-Jones potential with boundary condition PBC
1. ./runV1LJ2AtomPBC T rowNum, to execute mc computations
2. ./runV1LJ2AtomParseXMLPBC rowNum, to transform data to json
3. cd ./version1/LJPotPBC, python  json2plt.py, to plot 