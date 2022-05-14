#I get this code from https://blog.csdn.net/m0_59485658/article/details/120453497 and recode it to apply to micropython
import micronumpy as np
import math
from casioplot import *

def normalize(x):
    return np.div(x , np.norm(x))
 
def intersect(origin, dir, obj):
    if obj['type'] == 'plane':
        return intersect_plane(origin, dir, obj['position'], obj['normal'])
    elif obj['type'] == 'sphere':
        return intersect_sphere(origin, dir, obj['position'], obj['radius'])
 
def intersect_plane(origin, dir, point, normal):
    dn = np.dot(dir, normal)
    if abs(dn) < 1e-6:
        return np.inf
    d = np.dot(np.minus(point , origin), normal) / dn
    return d if d>0 else np.inf
 
def intersect_sphere(origin, dir, center, radius):
    OC = np.minus(center , origin)
    if (np.norm(OC) < radius) or (np.dot(OC, dir) < 0):
        return np.inf
    l = np.norm(np.dot(OC, dir))
    m_square = np.norm(OC) * np.norm(OC) - l * l
    q_square = radius*radius - m_square
    return (l - math.sqrt(q_square)) if q_square >= 0 else np.inf
 
def get_normal(obj, point):
    if obj['type'] == 'sphere':
        return normalize(np.minus(point , obj['position']))
    if obj['type'] == 'plane':
        return obj['normal']
 
def get_color(obj, M):
    color = obj['color']
    if type(color)!=list:
        color = color(M)
    return color
 
def sphere(position, radius, color, reflection=.85, diffuse=1., specular_c=.6, specular_k=50):
    return dict(type='sphere', position=np.array(position), radius=np.array(radius), 
                color=np.array(color), reflection=reflection, diffuse=diffuse, specular_c=specular_c, specular_k=specular_k)
 
def plane(position, normal, color=np.array([1.,1.,1.]), reflection=.15 , diffuse=.75, specular_c=.3, specular_k=50):
    return dict(type='plane', position=np.array(position), normal=np.array(normal), 
                color=lambda M: (np.array([1.,1.,1.]) if (int(M[0]*2)%2) == (int(M[2]*2)%2) else (np.array([0.,0.,0.]))),
                reflection=reflection, diffuse=diffuse, specular_c=specular_c, specular_k=specular_k)
 
scene = [sphere([.75, .1, 1.], .6, [.8, .3, 0.]), #position, radius, and color of a sphere
         sphere([-.3, .01, .2], .2, [.0, .0, .9]),
         sphere([-2.75, .1, 3.5], .6, [.1, .572, .184]),
         plane([0., -.5, 0.], [0., 1., 0.])] #positon and normal vector of a plane
light_point = np.array([5., 5., -10.]) #position of pointlight
light_color = np.array([1., 1., 1.]) #color of pointlight
ambient = 0.05 #the level of ambient light

def intersect_color(origin, dir, intensity):
    min_distance = np.inf
    for i in range(len(scene)):
        obj= scene[i]
        current_distance = intersect(origin, dir, obj)
        #print(current_distance)
        if current_distance < min_distance:
            min_distance, obj_index = current_distance, i
    if (min_distance == np.inf) or (intensity < 0.01):
        return np.array([0., 0., 0.])
 
    obj = scene[obj_index]
    P = np.add(origin , np.mul(dir , min_distance))
    color = get_color(obj, P)
    N = get_normal(obj, P)
    PL = normalize(np.minus(light_point , P))
    PO = normalize(np.minus(origin , P))
 
    c = np.mul(color , ambient)
 
    l = [intersect(np.add(P , np.mul(N , .0001)), PL, scene[i])
            for i in range(len(scene)) if i != obj_index]
    if not (l and min(l) < np.norm(np.minus(light_point , P))):
        
        c =np.add(c, np.Dmul(np.mul(color , obj['diffuse'] * max(np.dot(N, PL), 0)) , light_color))
        c = np.add(c , np.mul(light_color, obj['specular_c'] * max(np.dot(N, normalize(PL + PO)), 0) ** obj['specular_k'] ))
        
    reflect_ray = np.minus(dir , np.mul(N,2 * np.dot(dir, N)))  #                   
    c =np.add(c,np.mul(intersect_color(np.add(P , np.mul(N , .0001)), reflect_ray, obj['reflection'] * intensity),obj['reflection']))
    #return c
    return np.clip(c, 0, 1)
 
w, h = 380, 190     #width and height
O = np.array([0., 0.35, -1.])   #the position of camera
Q = np.array([0., 0., 0.])      #the dir of camera
r = float(w) / h
S = (-1., -1. / r + .25, 1., 1. / r + .25)
rangelistx=np.linspace(S[0], S[2], w)

for i in range(len(rangelistx)):
    x=rangelistx[i]
    rangelisty=np.linspace(S[1], S[3], h)
    for j in range(len(rangelisty)):
        y = rangelisty[j]
        Q[:2] = (x, y)
        img= intersect_color(O, normalize(np.minus(Q , O)), 1)
        set_pixel(i,h-j-1,np.floor(np.mul(img,255)))
    show_screen()