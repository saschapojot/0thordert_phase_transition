import pickle
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


pklFilesPath=str(sys.argv[1])

nCounterStart=int(sys.argv[2])

#fetch files in the directory
inPKLFileNames=[]
startVals=[]

for file in glob.glob(pklFilesPath+"/*.pkl"):
    inPKLFileNames.append(file)
    matchStart=re.search(r"loopStart(-?\d+(\.\d+)?)loopEnd",file)
    if matchStart:
        startVals.append(matchStart.group(1))

def str2int(valList):
    ret = [int(strTmp) for strTmp in valList]
    return ret


startVals = str2int(startVals)

start_inds = np.argsort(startVals)

#sort files by starting value
inSortedPKLFileNames=[inPKLFileNames[ind] for ind in start_inds]


def parse1File(oneFileName):
    with open(oneFileName,"rb") as fptr:
        oneVec=list(pickle.load(fptr))
        return oneVec



pklFileToBeParsed=inSortedPKLFileNames[nCounterStart:]

#combine all vectors
vecValsCombined=parse1File(pklFileToBeParsed[0])

for file in pklFileToBeParsed[1:]:
    vecValsCombined+=parse1File(file)


# vecValsCombined=np.array(vecValsCombined)
# halfLength=int(len(vecValsCombined)/2)
# part0=vecValsCombined[:halfLength]
# part1=vecValsCombined[halfLength:]


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
    vecValsSelected=vecValsCombined[::lagVal]
    lengthTmp=len(vecValsSelected)
    if lengthTmp%2==1:
        lengthTmp-=1
    vecValsToCompute=vecValsSelected[:lengthTmp]
    lenPart=int(len(vecValsToCompute)/2)
    selectedFromPart0=vecValsToCompute[:lenPart]
    selectedFromPart1=vecValsToCompute[lenPart:]
    result = ks_2samp(selectedFromPart0, selectedFromPart1)

    numDataPoints=len(selectedFromPart0)+len(selectedFromPart1)
    p=result.pvalue

    msg="lag="+str(lagVal)+"\n"+"K-S statistic: "+str(result.statistic)+"\n"+"p="+str(result.pvalue)+"\n"\
    +"numDataPoints="+str(numDataPoints)
    print(msg)
    print("nCounterStart="+str(nCounterStart))
    exit()
