import pandas as pd
from pathlib import Path
import sys


#python readCSV.py path rowNum
if len(sys.argv)!=3:
    print("wrong number of arguments")

fileName=sys.argv[1]
rowNum=int(sys.argv[2])

dfStr=pd.read_csv(fileName)

oneRow=dfStr.iloc[rowNum,:]

alpha1=float(oneRow.loc["alpha1"])
beta1=float(oneRow.loc["beta1"])
p1=float(oneRow.loc["p1"])
q1=float(oneRow.loc["q1"])


alpha2=float(oneRow.loc["alpha2"])
beta2=float(oneRow.loc["beta2"])
p2=float(oneRow.loc["p2"])
q2=float(oneRow.loc["q2"])

print("alpha1"+str(alpha1)+"beta1"+str(beta1)+"p1"+str(p1)+"q1"+str(q1)
      +"alpha2"+str(alpha2)+"beta2"+str(beta2)+"p2"+str(p2)+"q2"+str(q2))