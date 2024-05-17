import numpy as np

import matplotlib.pyplot as plt

#V1= LJ + quartic with 2 min

alpha1=1
beta1=0.8
p1=2
q1=1
alpha2=1.2
beta2=0.4
p2=3
q2=2

rMin=((alpha1*p1)/(beta1*q1))**(1/(p1-q1))


x0=0.8*rMin
y0=1

x1=rMin
y1=0.5

x2=2*rMin
y2=1

x3=3*rMin
y3=0

x4=4*rMin
y4=1

xValsAll=np.array([x0,x1,x2,x3,x4])

yValsAll=np.array([y0,y1,y2,y3,y4])

VMat=np.ones((5,5),dtype=float)

for i in range(1,5):
    VMat[:,i]=xValsAll**i

aVec=np.linalg.inv(VMat)@yValsAll

def p4(x):
    vec0=[1]
    vec1=[x**i for i in range(1,5)]
    xTmp=np.array(vec0+vec1)

    return np.dot(xTmp,aVec)

def V1(r):
   return alpha1*r**(-p1)-beta1*r**(-q1)+p4(r)
   #  return p4(r)



rValsAll=np.linspace(2,4.1*rMin,100)

V1ValsAll=[V1(r) for r in rValsAll]
print(rMin)
plt.figure()
plt.plot(rValsAll,V1ValsAll,color="black")
plt.title("$V_{1}$")
plt.savefig("tmpV1.png")
plt.close()
