import math

#maybe only fit my code 
#WHY DOES NOT MICROPYTHON HAVE NUMPY!!! 

inf = 99999999.

def array(x): #just a vector
    return x

def floor(x): #get a floor from a 3Dvector
    return [ math.floor(x[0]) , math.floor(x[1]) , math.floor(x[2] )]

def zeros(arr): #get a zeros matrix
    x,y,z = arr
    return [[[0 for k in range(0,z)] for j in range(0,y)]for i in range(0,x)]

def linspace(a,b,n): #divide the range from a to b into n
    output=[]
    i = a
    step = (b-a) / (n-1)
    while i <= b:
        output.append(i)
        i += step
    return output

def sgn(x):
    if x > 0:
        return 1
    if x == 0:
        return 0
    if x < 0:
        return -1

def add(x,y): 
    return [x[0] + y[0],x[1] + y[1],x[2] + y[2]]

def minus(x,y):
    return [x[0] - y[0] , x[1] - y[1] , x[2] - y[2]]

def mul(x,y):
    return [x[0] * y , x[1] * y , x[2] * y ]

def Dmul(x,y): #just like ".*" in MATLAB
    return [x[0] * y[0] , x[1] * y[1] , x[2] * y[2]]

def dot(x,y): #dot product
    return x[0]*y[0] + x[1]*y[1] + x[2]*y[2]

def div(x,y): #just like "./"in MATLAB
    if y == 0:
        return [sgn(x[0])*inf , sgn(x[1])*inf , sgn(x[2])*inf]
    else:
        return [x[0]/y , x[1]/y , x[2]/y]

def pow(x,y):
    return [math.pow(x[0],y),math.pow(x[1],y),math.pow(x[2],y)]

def norm(x): #norm if 3Dvector or abs if float
    if type(x) == list:
        result = math.sqrt(dot(x,x))
    else:
        result = abs(x)
    return result

def clip(x,y,z):
    return [sorted([x[0],y,z])[1],sorted([x[1],y,z])[1],sorted([x[2],y,z])[1]]
