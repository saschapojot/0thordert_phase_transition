import xml.etree.ElementTree as ET
import numpy as np
# from datetime import datetime
import statsmodels.api as sm
from scipy import stats
import glob
import sys
import re
from copy import deepcopy
import warnings
from scipy.stats import ks_2samp


#This script rechecks the lag value and whether equilibrium has been reached after the computation

sigNo="no"
sigYes="yes"
sigWrongArg="wrong number of arguments"
sigContinue="continue"

if (len(sys.argv)!=3):
    print(sigWrongArg)
    exit()

rootPath=str(sys.argv[1])
xmlFilesPath=rootPath+"/UAll/"
fileNumSelected=int(sys.argv[2])

#fetch files in the directory
inXMLFileNames=[]
startVals=[]

for file in glob.glob(xmlFilesPath+"/*UAll.xml"):
    inXMLFileNames.append(file)
    matchStart=re.search(r"loopStart(\d+)loopEnd",file)
    startVals.append(matchStart.group(1))


def str2int(valList):
    ret = [int(strTmp) for strTmp in valList]
    return ret

startVals = str2int(startVals)
startVals=np.array(startVals)
start_inds = np.argsort(startVals)
sortedStartVals=np.array([startVals[i] for i in start_inds])


#sort files by starting value
sortedXMLFileNames=[inXMLFileNames[ind] for ind in start_inds]


xmlFileToBeParsed=sortedXMLFileNames[-fileNumSelected:]

# print(len(xmlFileToBeParsed))
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



#combine all vectors
vecValsCombined=parse1File(xmlFileToBeParsed[0])

for file in xmlFileToBeParsed[1:]:
    vecValsCombined+=parse1File(file)


vecValsCombined=np.array(vecValsCombined)
#all values equal
meanU=np.mean(vecValsCombined)

diff=np.linalg.norm(vecValsCombined-meanU,ord=1)
if diff<1e-7:
    print("same"+", fileNum="+str(fileNumSelected))
    exit()

#check if the whole vector has the same value
with warnings.catch_warnings():
    warnings.filterwarnings("error")
    try:
        vecAutc=sm.tsa.acf(vecValsCombined)
    except Warning as w:
        print("same"+", fileNum="+str(fileNumSelected))
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
    print("same"+", fileNum="+str(fileNumSelected))
    exit()


elif same0==True and same1==False:
    # print("f0 True f1 False")
    print(sigContinue)
    exit()

elif same0==False and same1==True:
    # print("f0 False f1 True")
    print(sigContinue)
    exit()

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
    print("len(selectedFromPart0)="+str(len(selectedFromPart0)))
    print("len(selectedFromPart1)="+str(len(selectedFromPart1)))
    postCheckFile=rootPath+"/post.txt"
    msg="lag="+str(lagVal)+"\n"+"K-S statistic: "+str(result.statistic)+"\n"+"P-value:: "+str(result.pvalue)+"\n"
    print(msg)
    with open(postCheckFile,"w+") as fptr:
        fptr.write(msg)

