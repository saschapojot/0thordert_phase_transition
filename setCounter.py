import xml.etree.ElementTree as ET
import numpy as np
# from datetime import datetime
import statsmodels.api as sm
# from scipy import stats
import glob
import sys
import re
from copy import deepcopy
import warnings
from scipy.stats import ks_2samp


#This scripts directs the computation of the second mc function
sigWrongArg="wrong number of arguments"
sigEq="equilibrium"
sigContinue="continue"

if (len(sys.argv)!=3):
    print(sigWrongArg)
    exit()


xmlFilesPath=str(sys.argv[1])

nCounterStart=int(sys.argv[2])

#fetch files in the directory
inXMLFileNames=[]
startVals=[]

for file in glob.glob(xmlFilesPath+"/*.xml"):
    inXMLFileNames.append(file)
    matchStart=re.search(r"loopStart(-?\d+(\.\d+)?)loopEnd",file)
    if matchStart:
        startVals.append(matchStart.group(1))

def str2int(valList):
    ret = [int(strTmp) for strTmp in valList]
    return ret


startVals = str2int(startVals)

start_inds = np.argsort(startVals)

#sort files by starting value
inSortedXMLFileNames=[inXMLFileNames[ind] for ind in start_inds]


def parse1File(fileName):
    """

    :param fileName: xml file name
    :return: the values in the vector
    """

    tree=ET.parse(fileName)
    root = tree.getroot()
    vec=root.find("vec")
    vec_items=vec.findall('item')
    vecValsAll=[float(item.text) for item in vec_items]
    # vecValsAll=np.array(vecValsAll)
    return vecValsAll



xmlFileToBeParsed=inSortedXMLFileNames[nCounterStart:]

#combine all vectors
vecValsCombined=parse1File(xmlFileToBeParsed[0])

for file in xmlFileToBeParsed[1:]:
    vecValsCombined+=parse1File(file)


vecValsCombined=np.array(vecValsCombined)
halfLength=int(len(vecValsCombined)/2)
part0=vecValsCombined[:halfLength]
part1=vecValsCombined[halfLength:]


#computation of auto-correlation
NLags=int(np.ceil(len(vecValsCombined)*5/6))
acfOfVec=sm.tsa.acf(vecValsCombined,nlags=NLags)
eps=1e-3

lagVal=0
if np.min(np.abs(acfOfVec))>eps:
    print("high correlation")
    # print(np.min(np.abs(acfOfVec)))

    print(sigContinue)
    exit()

else:
    lagVal=np.where(np.abs(acfOfVec)<=eps)[0][0]
    selectedFromPart0=part0[::lagVal]
    # print(len(selectedFromPart0))
    selectedFromPart1=part1[::lagVal]
    result = ks_2samp(selectedFromPart0, selectedFromPart1)

    numDataPoints=len(selectedFromPart0)+len(selectedFromPart0)
    p=result.pvalue

    msg="lag="+str(lagVal)+"\n"+"K-S statistic: "+str(result.statistic)+"\n"+"p="+str(result.pvalue)+"\n"\
    +"numDataPoints="+str(numDataPoints)
    print(msg)
    print("nCounterStart="+str(nCounterStart))
    exit()
