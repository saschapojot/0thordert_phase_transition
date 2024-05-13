import xml.etree.ElementTree as ET
import numpy as np
import glob
import sys
import re
import statsmodels.api as sm
import matplotlib.pyplot as plt
# from copy import deepcopy
from pathlib import Path
from multiprocessing import Pool
from datetime import datetime
import pandas as pd
from copy import deepcopy

#this script computes statistics for 1d for each function, plots evolution

moveNumInOneFlush=3000

pathData="../../version1Data/1d/"

funcFileNames=[]
TValsForAllFuncs=[]
TFileNamesForAllFuncs=[]
sortedTFilesForAllFuncs=[]
sortedTValsForAllFuncs=[]

for funcfile in glob.glob(pathData+"/funcLJPot*"):
    #first search a values
    funcFileNames.append(funcfile)
    # match_a=re.search(r"a(\d+(\.\d+)?)",a_file)
    # aVals.append(float(match_a.group(1)))
    #for each a, search T values
    TFilesTmp=[]
    TValsTmp=[]
    for TFile in glob.glob(funcfile+"/T*"):
        TFilesTmp.append(TFile)
        # print(TFile)
        matchT=re.search(r"T(\d+(\.\d+)?)",TFile)
        TValsTmp.append(float(matchT.group(1)))

    TFileNamesForAllFuncs.append(TFilesTmp)
    TValsForAllFuncs.append(TValsTmp)


#sort T files for each func
for j in range(0,len(funcFileNames)):
    T_indsTmp=np.argsort(TValsForAllFuncs[j])
    TValsTmp=TValsForAllFuncs[j]
    sortedTValsTmp=[TValsTmp[i] for i in T_indsTmp]
    sortedTValsForAllFuncs.append(sortedTValsTmp)

    TFilesTmp=TFileNamesForAllFuncs[j]
    sortedTFilesTmp=[TFilesTmp[i] for i in T_indsTmp]
    sortedTFilesForAllFuncs.append(sortedTFilesTmp)


def parseSummaryBeforeEq(summaryFile):
    '''

    :param summaryFile: summary.txt
    :return: same, loopNumBeforeEq,lag,lastFileNum
    '''
    fptr=open(summaryFile,"r")
    contents=fptr.readlines()

    same=False
    loopNumBeforeEq=-1
    lastFileNum=-1
    lag=-1
    for line in contents:

        matchSame=re.search(r"same:\s*(\d+)",line)
        if matchSame:
            same=int(matchSame.group(1))

        matchLoopNum=re.search(r"total loop number\s*:\s*(\d+)",line)
        if matchLoopNum:
            loopNumBeforeEq=int(matchLoopNum.group(1))

        matchLag=re.search(r"lag=\s*(\d+)",line)

        if matchLag:
            lag=int(matchLag.group(1))


        matchLastFileNum=re.search(r"lastFileNum=\s*(\d+)",line)
        if matchLastFileNum:
            lastFileNum=int(matchLastFileNum.group(1))

    return same, loopNumBeforeEq,lag,lastFileNum


def searchsummaryAfterEqFile(oneTFile):
    """

    :param oneTFile: one T directory
    :return: a list containing the summaryAfterEq.txt file, the list will be length 0 if the
    file does not exist
    """
    file=glob.glob(oneTFile+"/summaryAfterEq.txt")

    return file




def parseAfterEqFile(oneTFile):
    """

    :param oneTFile: one T directory
    :return: a list containing the summaryAfterEq.txt file, and the total loop number after eq
    """
    fileList=searchsummaryAfterEqFile(oneTFile)
    loopNumAfterEq=-1
    if len(fileList)!=0:
        fileName=fileList[0]
        fptr=open(fileName,"r")
        contents=fptr.readlines()
        for line in contents:
            matchNum=re.search(r"total loop number\s*:\s*(\d+)",line)
            if matchNum:
                loopNumAfterEq=int(matchNum.group(1))


    return fileList,loopNumAfterEq


def UAndxFilesSelected(oneTFile):
    """

    :param oneTFile: one T directory
    :return: U files and xA, xB files to be parsed
    """
    smrFile=oneTFile+"/summary.txt"
    same, loopNumBeforeEq,lag,lastFileNum=parseSummaryBeforeEq(smrFile)
    fileAfterEqList,loopNumAfterEq=parseAfterEqFile(oneTFile)
    fileNumSelected=0#files' numbers to be parsed
    if same==1:
        fileNumSelected=1
    else:
        if len(fileAfterEqList)!=0:
            loopNumToInclude=moveNumInOneFlush*lastFileNum+loopNumAfterEq
            fileNumSelected=int(np.ceil(loopNumToInclude/moveNumInOneFlush))
        else:
            fileNumSelected=lastFileNum

    UAllDir=oneTFile+"/UAll/*.xml"
    xA_AllDir=oneTFile+"/xA_All/*.xml"
    xB_AllDir=oneTFile+"/xB_All/*.xml"
    inUAllFileNames=[]
    startUAllVals=[]

    for file in glob.glob(UAllDir):
        inUAllFileNames.append(file)
        matchUStart=re.search(r"loopStart(-?\d+(\.\d+)?)loopEnd",file)
        if matchUStart:
            startUAllVals.append(int(matchUStart.group(1)))


    start_U_inds=np.argsort(startUAllVals)

    sortedUAllFileNames=[inUAllFileNames[ind] for ind in start_U_inds]


    inxA_AllFileNames=[]
    startxA_AllVals=[]

    for file in glob.glob(xA_AllDir):
        inxA_AllFileNames.append(file)
        matchxA_AllStart=re.search(r"loopStart(-?\d+(\.\d+)?)loopEnd",file)
        if matchxA_AllStart:
            startxA_AllVals.append(int(matchxA_AllStart.group(1)))


    start_xA_All_inds=np.argsort(startxA_AllVals)
    sortedxA_AllFileNames=[inxA_AllFileNames[ind] for ind in start_xA_All_inds]

    inxB_AllFileNames=[]
    startxB_AllVals=[]
    for file in glob.glob(xB_AllDir):
        inxB_AllFileNames.append(file)
        matchxB_AllStart=re.search(r"loopStart(-?\d+(\.\d+)?)loopEnd",file)
        if matchxB_AllStart:
            startxB_AllVals.append(int(matchxB_AllStart.group(1)))


    start_xB_All_inds=np.argsort(startxB_AllVals)
    sortedxB_AllFileNames=[inxB_AllFileNames[ind] for ind in start_xB_All_inds]

    retUAllFileNames=sortedUAllFileNames[-fileNumSelected:]
    retxA_AllFileNames=sortedxA_AllFileNames[-fileNumSelected:]
    retxB_AllFileNames=sortedxB_AllFileNames[-fileNumSelected:]

    return same, retUAllFileNames,retxA_AllFileNames,retxB_AllFileNames,lag,fileNumSelected




def parseUFile(UFileName):
    """

    :param UFileName: xml file containing U
    :return: values of U in this file
    """

    tree=ET.parse(UFileName)
    root = tree.getroot()
    vec=root.find("vec")
    vec_items=vec.findall('item')
    vecValsAll=[float(item.text) for item in vec_items]
    # vecValsAll=np.array(vecValsAll)

    return vecValsAll



def parsexFile(xFileName):
    """

    :param xFileName: xml file containing x vectors
    :return: all vectors in this xml file
    """
    tree=ET.parse(xFileName)
    root = tree.getroot()
    first_level_items = root.find('vecvec').findall('item')
    vectors=[]
    for item in first_level_items:
        oneVec=[float(value.text) for value in item.findall('item')]
        vectors.append(oneVec)

    return np.array(vectors)




def combineValues(oneTFile):
    """

    :param oneTFile: corresponds to one temperature
    :return: combined values of U and xA, xB from each file, names of the parsed files
    """
    same, retUAllFileNames,retxA_AllFileNames,retxB_AllFileNames,lag,fileNumSelected=UAndxFilesSelected(oneTFile)

    UVecValsCombined=parseUFile(retUAllFileNames[0])
    for file in retUAllFileNames[1:]:
        UVecValsCombined+=parseUFile(file)


    xA_VecVecCombined=parsexFile(retxA_AllFileNames[0])
    for file in retxA_AllFileNames[1:]:
        xA_VecVecNext=parsexFile(file)

        xA_VecVecCombined=np.r_[xA_VecVecCombined,xA_VecVecNext]


    xB_VecVecCombined=parsexFile(retxB_AllFileNames[0])
    for file in retxB_AllFileNames[1:]:
        xB_VecVecNext=parsexFile(file)

        xB_VecVecCombined=np.r_[xB_VecVecCombined,xB_VecVecNext]

    return same, UVecValsCombined,xA_VecVecCombined,xB_VecVecCombined,lag,fileNumSelected


def meanAndVarForScalar(vec):
    """

    :param vec: a vector of float
    :return: mean, half length of confident interval
    """

    meanVal=np.mean(vec)
    varVal=np.var(vec,ddof=1)
    hfLength=1.96*np.sqrt(varVal/len(vec))

    return meanVal,hfLength


outRoot=pathData
def diagnosticsAndStats(oneTFile):
    """

    :param oneTFile: corresponds to one temperature
    :return: diagnostic plots and observable values
    """
    tOneFileStart=datetime.now()
    TTmpMatch=re.search(r"T(\d+(\.\d+)?)",oneTFile)
    if TTmpMatch:
        TTmp=float(TTmpMatch.group(1))
    same, UVecValsCombined,xA_VecVecCombined,xB_VecVecCombined,lag,fileNumSelected=combineValues(oneTFile)
    ##############diagnostics: not identical values ################################################################
    if same==0:
        #diagnostics for U
        USelected=UVecValsCombined[::lag]

        meanU=np.mean(USelected)
        varU=np.var(USelected,ddof=1)
        sigmaU=np.sqrt(varU)
        # print("varU="+str(varU))
        hfIntervalU=1.96*np.sqrt(varU/len(USelected))

        #diagnostics of U
        nbins=500

        #histogram of U
        fig=plt.figure()
        axU=fig.add_subplot()
        (n0,_,_)=axU.hist(USelected,bins=nbins)
        meanU=np.round(meanU,4)
        sigmaU=np.round(sigmaU,4)
        axU.set_title("T="+str(np.round(TTmp,3)))
        axU.set_xlabel("$U$")
        axU.set_ylabel("#")
        xPosUText=(np.max(USelected)-np.min(USelected))*1/2+np.min(USelected)
        yPosUText=np.max(n0)*2/3
        axU.text(xPosUText,yPosUText,"mean="+str(meanU)+"\nsd="+str(sigmaU)+"\nlag="+str(lag))
        plt.axvline(x=meanU,color="red",label="mean")
        axU.text(meanU*1.1,0.5*np.max(n0),str(meanU)+"$\pm$"+str(sigmaU),color="red")
        axU.hlines(y=0,xmin=meanU-sigmaU,xmax=meanU+sigmaU,color="green",linewidth=15)

        plt.legend(loc="best")

        EHistOut="T"+str(TTmp)+"UHist.png"
        plt.savefig(oneTFile+"/"+EHistOut)

        plt.close()

        ### test normal distribution for mean U
        USelectedAll=USelected

        #block mean
        def meanPerBlock(length):
            blockNum=int(np.floor(len(USelectedAll)/length))
            UMeanBlock=[]
            for blkNum in range(0,blockNum):
                blkU=USelectedAll[blkNum*length:(blkNum+1)*length]
                UMeanBlock.append(np.mean(blkU))
            return UMeanBlock

        fig=plt.figure(figsize=(20,20))
        fig.tight_layout(pad=5.0)
        lengthVals=[5,10,20,40]
        for i in range(0,len(lengthVals)):
            l=lengthVals[i]
            UMeanBlk=meanPerBlock(l)
            ax=fig.add_subplot(2,2,i+1)
            (n,_,_)=ax.hist(UMeanBlk,bins=100,color="aqua")
            xPosTextBlk=(np.max(UMeanBlk)-np.min(UMeanBlk))*1/7+np.min(UMeanBlk)
            yPosTextBlk=np.max(n)*3/4
            meanTmp=np.mean(UMeanBlk)
            meanTmp=np.round(meanTmp,3)
            sdTmp=np.sqrt(np.var(UMeanBlk))
            sdTmp=np.round(sdTmp,3)
            ax.set_title("L="+str(l))
            ax.text(xPosTextBlk,yPosTextBlk,"mean="+str(meanTmp)+", sd="+str(sdTmp))
        fig.suptitle("T="+str(TTmp))
        plt.savefig(oneTFile+"/T"+str(TTmp)+"UBlk.png")
        # plt.savefig(EBlkMeanDir+"/T"+str(TTmp)+"EBlk.png")
        plt.close()

    #diagnostics of xA, xB
    xA_VecVecSelected=xA_VecVecCombined[::lag,:]
    xA_ValsForEachPosition=[]
    _,nColx=xA_VecVecSelected.shape

    xB_VecVecSelected=xB_VecVecCombined[::lag,:]
    xB_ValsForEachPosition=[]

    #xA_VecVecSelected and xB_VecVecSelected have the same shape

    #take centers of xA and xB

    xACentered=deepcopy(xA_VecVecSelected)
    xAMeans=xACentered.mean(axis=1,keepdims=True)
    # xACentered=xACentered-xAMeans

    xBCentered=deepcopy(xB_VecVecSelected)
    xBMeans=xBCentered.mean(axis=1,keepdims=True)
    # xBCentered=xBCentered-xBMeans

    for j in range(0,nColx):
        xAFor1Point=xACentered[:,j]
        xA_ValsForEachPosition.append(xAFor1Point)

        xBFor1Point=xBCentered[:,j]
        xB_ValsForEachPosition.append(xBFor1Point)



    fig=plt.figure(figsize=(20,160))
    fig.tight_layout(pad=5.0)
    x_vertical_distance = 0.9
    xAMeanAll=[]
    xBMeanAll=[]
    for j in range(0,nColx):
        axx=fig.add_subplot(nColx,1,j+1,sharex=axx if j != 0 else None)
        #plot A
        xAValsTmp=xA_ValsForEachPosition[j]
        xAMeanTmp=np.mean(xAValsTmp)
        xAMeanAll.append(xAMeanTmp)
        xAVarTmp=np.var(xAValsTmp,ddof=1)
        xASigmaTmp=np.sqrt(xAVarTmp)
        nbins=500
        (nA,_,_)=axx.hist(xAValsTmp,bins=nbins,color = "blue", ec="blue")
        xAMeanTmp=np.round(xAMeanTmp,4)
        xASigmaTmp=np.round(xASigmaTmp,4)
        axx.set_title("Unit cell "+str(j)+", T="+str(np.round(TTmp,3)))
        plt.axvline(x=xAMeanTmp,color="red",label="mean A")
        axx.text(xAMeanTmp*1.1,0.5*np.max(nA),str(xAMeanTmp)+"$\pm$"+str(xASigmaTmp),color="red")

        #plot B
        xBValsTmp=xB_ValsForEachPosition[j]
        xBMeanTmp=np.mean(xBValsTmp)
        xBMeanAll.append(xBMeanTmp)
        xBVarTmp=np.var(xBValsTmp,ddof=1)
        xBSigmaTmp=np.sqrt(xBVarTmp)
        nbins=500
        (nB,_,_)=axx.hist(xBValsTmp,bins=nbins,color = "green", ec="green")
        plt.axvline(x=xBMeanTmp,color="magenta",label="mean B")
        axx.text(xBMeanTmp*1.1,0.5*np.max(nB),str(xBMeanTmp)+"$\pm$"+str(xBSigmaTmp),color="magenta")
        axx.set_ylabel("#")
        axx.set_xlabel("position")
    xHistOut="T"+str(TTmp)+"xHist.pdf"

    plt.subplots_adjust(hspace=x_vertical_distance)
    plt.savefig(oneTFile+"/"+xHistOut)
    plt.close()



tStatsStart=datetime.now()
for item in TFileNamesForAllFuncs:
    for oneTFile in item:
        diagnosticsAndStats(oneTFile)

tStatsEnd=datetime.now()
print("stats total time: ",tStatsEnd-tStatsStart)

