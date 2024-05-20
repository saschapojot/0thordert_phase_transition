import numpy as np
import glob
import sys
import re
import matplotlib.pyplot as plt
from datetime import datetime
import json
from scipy.stats import ks_2samp
#This script verifies the distribution of U



pathData="../../version1Data/1d/funcLJPotPBC/row0"

TVals=[]
TFileNames=[]

for TFile in glob.glob(pathData+"/T*"):
    TFileNames.append(TFile)

    matchT=re.search(r"T(\d+(\.\d+)?)",TFile)

    TVals.append(float(matchT.group(1)))

#sort T files

sortedInds=np.argsort(TVals)
sortedTVals=[TVals[ind] for ind in sortedInds]
sortedTFiles=[TFileNames[ind] for ind in sortedInds]
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
def ksTestU(oneTFile):
    matchT=re.search(r"T(\d+(\.\d+)?)",oneTFile)
    TVal=float(matchT.group(1))
    UFilePath=oneTFile+"/jsonData/jsonU/UData.json"
    with open(UFilePath, 'r') as fptr:
        data = json.load(fptr)
    UVec=np.array(data["U"])

    lengthU=len(UVec)
    if lengthU%2==1:
        lengthU-=1
    USelected=UVec[-lengthU:]

    lengthSelected=int(len(USelected)/2)

    UPart1=USelected[0:lengthSelected]
    UPart2=USelected[-lengthSelected:]
    result = ks_2samp(UPart1, UPart2)

    outKsFile=oneTFile+"/ks.txt"
    mean1,hf1=Jackknife(UPart1)
    mean2,hf2=Jackknife(UPart2)
    with open(outKsFile,"w") as fptr:
        fptr.write("K-S statistic: "+str(result.statistic)+"\n")
        fptr.write("P-value:: "+str(result.pvalue)+"\n")
        fptr.write("mean1="+str(mean1)+", hf1="+str(hf1)+"\n")
        fptr.write("mean2="+str(mean2)+", hf1="+str(hf2)+"\n")



tStatsStart=datetime.now()
for oneTFile in sortedTFiles:
    ksTestU(oneTFile)

tStatsEnd=datetime.now()
print("stats total time: ",tStatsEnd-tStatsStart)