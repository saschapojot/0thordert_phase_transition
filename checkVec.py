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


#This script checks if a vector reaches equilibrium
sigWrongArg="wrong number of arguments"
sigEq="equilibrium"
sigContinue="continue"
sigStop="stop"

if (len(sys.argv)!=2):
    print(sigWrongArg)
    exit()


pklFilesPath=str(sys.argv[1])
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

#ensure the file number is a multiple of 3
if len(inSortedPKLFileNames)%3==0:
    pklFileToBeParsed=deepcopy(inSortedPKLFileNames)
elif len(inSortedPKLFileNames)%3==1:
    pklFileToBeParsed=deepcopy(inSortedPKLFileNames[1:])
else:
    pklFileToBeParsed=deepcopy(inSortedPKLFileNames[2:])

lastFileNum=2000 if len(pklFileToBeParsed)>3000 else int(len(pklFileToBeParsed)/3*2)

pklFileToBeParsed=pklFileToBeParsed[-lastFileNum:]

def parse1File(oneFileName):
    with open(oneFileName,"rb") as fptr:
        oneVec=list(pickle.load(fptr))
        return oneVec



#combine all vectors
vecValsCombined=parse1File(pklFileToBeParsed[0])

for file in pklFileToBeParsed[1:]:
    vecValsCombined+=parse1File(file)


vecValsCombined=np.array(vecValsCombined)




#check if the whole vector has the same value
with warnings.catch_warnings():
    warnings.filterwarnings("error")
    try:
        vecAutc=sm.tsa.acf(vecValsCombined)
    except Warning as w:
        print(sigStop+" same"+", fileNum="+str(lastFileNum))
        exit()


halfLength=int(len(vecValsCombined)/2)
part0=vecValsCombined[:halfLength]
part1=vecValsCombined[halfLength:]

same0=False
same1=False

#check if the part0 has the same value
with warnings.catch_warnings():
    warnings.filterwarnings("error")
    try:
        autc0=sm.tsa.acf(part0)
    except Warning as w:
        same0=True


#check if the part1 has the same value
with warnings.catch_warnings():
    warnings.filterwarnings("error")
    try:
        autc1=sm.tsa.acf(part1)
    except Warning as w:
        same1=True


if same0 and same1:
    print(sigStop+" same"+", fileNum="+str(lastFileNum))
    exit()


elif same0==True and same1==False:
    # print("f0 True f1 False")
    print(sigContinue)
    exit()

elif same0==False and same1==True:
    # print("f0 False f1 True")
    print(sigContinue)
    exit()

def Jackknife(vec):
    """

    :param vec:
    :return: the mean and half length  of 0.95 confidence interval computed by Jackkknife
    """

    psMean=np.mean(vec)

    psVar=np.var(vec,ddof=1)

    n=len(vec)

    hfLen=1.96*np.sqrt(psVar/n)
    return psMean,hfLen


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
    print("len(selectedFromPart0)="+str(len(selectedFromPart0)))
    print("len(selectedFromPart1)="+str(len(selectedFromPart1)))
    msg="lag="+str(lagVal)+"\n"+"K-S statistic: "+str(result.statistic)+"\n"+"P-value: "+str(result.pvalue)+"\n"\
    +"fileNum="+str(lastFileNum)+", nCountStart="+str(len(inPKLFileNames)-lastFileNum)
    if result.pvalue>0.1:
        print(sigEq)
        print(msg)
        exit()
    else:
        print(sigContinue)
        exit()