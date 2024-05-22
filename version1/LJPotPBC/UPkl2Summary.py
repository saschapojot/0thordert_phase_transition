import pickle
import numpy as np
from datetime import datetime
import statsmodels.api as sm
import sys
import re
from scipy.stats import ks_2samp


#This script produces summaryAfterEq.txt if there isn't one from U.pkl

if len(sys.argv)!=3:
    print("wrong number of arguments")
    exit(1)

pathRoot=sys.argv[1]
tReadStart=datetime.now()
nCounterStart=int(sys.argv[2])

with open(pathRoot+"/U.pkl","rb") as fptr:
    UVec=pickle.load(fptr)

moveNumInOneFlush = 3000

startingPosition=moveNumInOneFlush*nCounterStart

vecValsCombined=UVec[startingPosition:]

tReadEnd=datetime.now()

print("reading time: ",tReadEnd-tReadStart)

tStatsStart=datetime.now()
NLags=int(np.ceil(len(vecValsCombined)*5/6))
acfOfVec=sm.tsa.acf(vecValsCombined,nlags=NLags)
eps=1e-3
lagVal=0
if np.min(np.abs(acfOfVec))>eps:
    print("high correlation")
    # print(np.min(np.abs(acfOfVec)))
    exit(2)

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
    outSmrName=pathRoot+"/summaryAfterEq.txt"
    msg="nCounterStart="+str(nCounterStart)+"\n" \
        +"lag="+str(lagVal)+"\n"
    with open(outSmrName,"w+") as fptr:
        fptr.write(msg)
