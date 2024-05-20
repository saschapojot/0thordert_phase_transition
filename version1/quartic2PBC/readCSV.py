import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from pathlib import Path
import sys
from scipy.optimize import root

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



r1Min=(alpha1*p1/(beta1*q1))**(1/(p1-q1))

r2Min=(alpha2*p2/(beta2*q2))**(1/(p2-q2))
# print("r1Min="+str(r1Min))
# print("r2Min="+str(r2Min))

x10=0.5*r1Min
x11=r1Min
x12=1.5*r1Min
x13=2*r1Min
x14=2.5*r1Min

y10=1
y11=0.2
y12=0.6
y13=0
y14=1
xValsFor1=np.array([x10,x11,x12,x13,x14])
yValsFor1=np.array([y10,y11,y12,y13,y14])

V1Mat=np.ones((5,5),dtype=float)
for j in range(1,5):
    V1Mat[:,j]=xValsFor1**j

aValsFor1=np.linalg.inv(V1Mat)@yValsFor1

x20=0.5*r2Min
x21=r2Min
x22=1.5*r2Min
x23=2*r2Min
x24=2.5*r2Min

y20=1
y21=0.2
y22=0.6
y23=0
y24=1

xValsFor2=np.array([x20,x21,x22,x23,x24])
yValsFor2=np.array([y20,y21,y22,y23,y24])
V2Mat=np.ones((5,5),dtype=float)
for j in range(1,5):
    V2Mat[:,j]=xValsFor2**j
aValsFor2=np.linalg.inv(V2Mat)@yValsFor2


aStrFor1=["a10","a11","a12","a13","a14"]
aStrFor2=["a20","a21","a22","a23","a24"]

print_a1=""
for j in range(0,len(aStrFor1)):
    print_a1+=aStrFor1[j]+str(aValsFor1[j])
print(print_a1)

print_a2=""
for j in range(0,len(aStrFor2)):
    print_a2+=aStrFor2[j]+str(aValsFor2[j])
print(print_a2)

# def V1(r):
#     powers0=[1]
#     powers1=[r**j for j in range(1,5)]
#     powAll=powers0+powers1
#     powAll=np.array(powAll,dtype=float)
#     val1= np.dot(aValsFor1,powAll)
#     valInv=alpha1/r**p1-beta1/r**q1
#
#
#     return val1+valInv
#
#
# rValsAll=np.linspace(0.5,x14*1.1,100)
#
#
# V1ValsAll=[V1(r) for r in rValsAll]
# plt.figure()
# plt.plot(rValsAll,V1ValsAll,color="blue")
# plt.savefig("1.png")
# plt.close()


# def V2(r):
#     powers0=[1]
#     powers1=[r**j for j in range(1,5)]
#     powAll=powers0+powers1
#     powAll=np.array(powAll)
#     return np.dot(aValsFor2,powAll)+alpha2/r**p2-beta2/r**q2
#
# rValsAll=np.linspace(0.5,x24*1.1,100)
#
#
# V2ValsAll=[V2(r) for r in rValsAll]
# plt.figure()
# plt.plot(rValsAll,V2ValsAll,color="blue")
# plt.savefig("2.png")
# plt.close()