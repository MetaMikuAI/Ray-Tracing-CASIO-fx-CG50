#include <fxcg/display.h>
#include <fxcg/keyboard.h>
#include <stdio.h>
#define INFINITY 65537
#define MIDDLE(x,y,z) ((x)<(y)?((y)<(z)?(y):(x)<(z)?(z):(x)):((y)>(z)?(y):(x)>(z)?(z):(x)))
#define ABS(x) (((x)<0)?(-(x)):(x))
#define num 4
float ambient = .05;
typedef struct{
    float x,y,z;
}Vector3D;
Vector3D array(float x, float y, float z){
    Vector3D res;
    res.x = x;
    res.y = y;
    res.z = z;
    return res;
}
typedef struct{
    Vector3D position;
    float radius;
    Vector3D color;
    float reflection;
    float diffuse;
    float specular_c;
    unsigned char specular_k;
}Sphere;
Sphere initSphere(float x,float y,float z,float r,float R,float G,float B){
    Sphere sphere;
    sphere.position = array(x,y,z);
    sphere.radius = r;
    sphere.color = array(R,G,B);
    sphere.reflection = .85;
    sphere.diffuse = 1.;
    sphere.specular_c = .6;
    sphere.specular_k = 50;
    return sphere;
}
typedef struct{
    Vector3D position;
    Vector3D normalVector;
    float reflection;
    float diffuse;
    float specular_c;
    unsigned char specular_k;

}Plane;
Plane initPlane(float x,float y,float z,float i,float j,float k){
    Plane plane;
    plane.position = array(x,y,z);
    plane.normalVector = array(i,j,k);
    plane.reflection = .15;
    plane.diffuse = .75;
    plane.specular_c =.3;
    plane.specular_k = 50;
    return plane;
}
typedef struct{
    Vector3D position;
    Vector3D color;
}Light;
Light initLight(float x,float y,float z,float r,float g,float b){
    Light light;
    light.position = array(x,y,z);
    light.color = array(r,g,b);
    return light;
}
typedef struct{
    void* obj;
    char type;
}Objects;
float sqrt_(float x){
    if (x <= 0.00001) 
        return 0;
    float last = 0;
    float res = 1;
    float tmp = 1;
    while (ABS(tmp) > 0.0001){
        last = res;
        res = (res + x / res) / 2;
        tmp = res - last;
    }
    return res;
}
float norm(Vector3D v){
    return sqrt_(v.x * v.x + v.y * v.y + v.z * v.z);
}
Vector3D normalize(Vector3D v){
    float norm_ = norm(v);
    v.x /= norm_;
    v.y /= norm_;
    v.z /= norm_;
    return v;
}
Vector3D add(Vector3D v, Vector3D w){
    v.x += w.x;
    v.y += w.y;
    v.z += w.z;
    return v;
}
Vector3D minus(Vector3D v, Vector3D w){
    v.x -= w.x;
    v.y -= w.y;
    v.z -= w.z;
    return v;
}
Vector3D mul(Vector3D v, float k){
    v.x *= k;
    v.y *= k;
    v.z *= k;
    return v;
}
Vector3D Dmul(Vector3D v, Vector3D w){
    v.x *= w.x;
    v.y *= w.y;
    v.z *= w.z;
    return v;
}
float dot(Vector3D v, Vector3D w){
    return v.x * w.x + v.y * w.y + v.z * w.z;
}
float max(float x, float y){
    return (x > y) ? x : y;
}
float linspace(const float lower, const float upper, const short length, const short n){
    return lower + (upper - lower) / length * n;
}
float pow_(float base, unsigned char power){
    float result = 1.0f;   
    while (power > 0){
        if (power & 1)          
            result *= base;     
        base *= base;           
        power >>= 1;            
    }
    return result;              
}
float pow32(float base){
    base *= base;
    base *= base;
    base *= base;
    base *= base;
    return base*base;              
}



Objects* Scene;
Light light;

int colorVectorToRGB565(Vector3D color){
    int RGB565;
    color.x = color.x < .96875 ? color.x : .96875;
    color.y = color.y < .984375 ? color.y : .984375;
    color.z = color.z < .96875 ? color.z : .96875;
    RGB565 = (int)(color.x * 32);
    RGB565 <<= 6;
    RGB565 += (int)(color.y * 64);
    RGB565 <<= 5;
    RGB565 += (int)(color.z * 32);
    return RGB565;
}
float intersect(Vector3D origin, Vector3D dir, Objects obj){
    switch (obj.type){
    case 's':{
        Sphere tmp_s = *(Sphere*)obj.obj;
        Vector3D OC = minus(tmp_s.position, origin);
        float normOC = norm(OC);
        if (normOC < tmp_s.radius || dot(OC,dir) < 0){
            return INFINITY;
        }
        float l = dot(OC,dir);
        float m_square = normOC * normOC - l * l;
        float q_square = tmp_s.radius * tmp_s.radius - m_square;
        return (q_square >= 0) ? (l - sqrt_(q_square)) : INFINITY;
    }
    case 'p':{
        Plane tmp_p = *(Plane*)obj.obj;
        float dn = dot(dir, tmp_p.normalVector);
        //printf("dn = %f\n", dn);
        if (ABS(dn) < .00001){
            return  INFINITY;
        }
        float d = dot(minus(tmp_p.position, origin), tmp_p.normalVector) / dn;
        if (d > 0){
        }
        return (d > 0) ? d : INFINITY;
    }
    default:
        return INFINITY;
    }
}
Vector3D intersectColor(Vector3D origin, Vector3D dir, float intensity){
    float minDistance = INFINITY;
    float currentDistance;
    unsigned char objIndex = 0;
    unsigned char i;
    for(i=0; i<num; i++){
        currentDistance = intersect(origin, dir, Scene[i]);
        if (currentDistance < minDistance){
            minDistance = currentDistance;
            objIndex = i;
        }
    }
    if(minDistance == INFINITY || intensity < .01){
        return array(0.f,0.f,0.f);
    }
    Objects obj = Scene[objIndex];
    float reflection,diffuse,specular_c,specular_k;
    Vector3D P = add(origin, mul(dir, minDistance));
    Vector3D color;// = getColor(obj, P);
    Vector3D N ;// = getNormal(obj, P);
    Sphere tmp_s;
    Plane tmp_p;

    switch (obj.type){
    case 's':
        tmp_s = *(Sphere*)obj.obj;
        reflection = tmp_s.reflection;
        diffuse = tmp_s.diffuse;
        specular_c = tmp_s.specular_c;
        specular_k = tmp_s.specular_k;
        color = tmp_s.color;
        N = normalize(minus(P,tmp_s.position));
        break;
    case 'p':
        tmp_p = *(Plane*)obj.obj;
        reflection = tmp_p.reflection;
        diffuse = tmp_p.diffuse;
        specular_c = tmp_p.specular_c;
        specular_k = tmp_p.specular_k;
        color = (((int)(P.x*2)&1) == ((int)(P.z*2)&1)) ? array(0,0,0) : array(1,1,1);
        N = tmp_p.normalVector;
        break;
    default:
        reflection = 0.;
        diffuse = 0.;
        specular_c = 0.;
        specular_k = 0.;
        N = array(0,0,0);
        color = array(0,0,0);
        break;
    }
    Vector3D PL = normalize(minus(light.position, P));
    Vector3D NP = add(P, mul(N, .0001));
    Vector3D c = mul(color, ambient);

    float l = INFINITY,temp;
    for (i=0; i<num; i++){
        if (i == objIndex){
            continue;
        }
        temp = intersect(NP, PL, Scene[i]);
        l = (l < temp) ? l : temp;
    }
    //if (l >= norm(minus(light.position, P))){
    if (l*l >= NP.x*NP.x + NP.y*NP.y + NP.z*NP.z){
        //printf("Light!\n");
        c = add(c, Dmul(mul(color, diffuse * max(dot(N,PL), 0)), light.color));
        c = add(c, mul(light.color, specular_c * pow_(max(dot(N, normalize(minus(PL,P))), 0) , specular_k)));
        //c = add(c, mul(light.color, specular_c * pow32(max(dot(N, normalize(minus(PL,P))), 0))));
    }
    Vector3D reflectRay = minus(dir, mul(N, 2 * dot(dir, N)));
    c = add(c, mul(intersectColor(NP, reflectRay, reflection * intensity), reflection));
    //color = array(MIDDLE(0,1,c.x),MIDDLE(0,1,c.y),MIDDLE(0,1,c.z));
    c.x = c.x < 1 ? c.x : 1;
    c.x = c.x > 0 ? c.x : 0;
    c.y = c.y < 1 ? c.y : 1;
    c.y = c.y > 0 ? c.y : 0;
    c.z = c.z < 1 ? c.z : 1;
    c.z = c.z > 0 ? c.z : 0;
    return c;
}
int main(void) {
    int key;
    EnableStatusArea(3); // Disable the status area
    Bdisp_EnableColor(1); // Enable 16-bit color
    
    short i,j;
    for (i = 0; i < 384; i++){
        for (j = 0; j < 216; j++){
            Bdisp_SetPoint_VRAM(i,j,0x3637);
        }
    }

    
    const unsigned short w = 384;
    const unsigned short h = 216;
    const float r = (float)w / h;
    const float S[4] = {-1.0f, -1.0f / r + .25f, 1.0f, 1.0f / r + .25f};
    Vector3D O = {0., 0.35, -1.};
    Vector3D Q = {0., 0., 0.};
    const Sphere sphere[3] = {initSphere(.75f, .1f, 1.f, .6f, .8f, .3f, 0.f),
                              initSphere(-.3f, .01f, .2f, .2f, 0.f, 0.f, .9f),
                              initSphere(-2.75f, .1f, 3.5f, .6f, .1f, .572f, .184f)};
    Plane plane = initPlane(0.f, -.5f, 0.f, 0.f, 1.f, 0.f);
    light = initLight(5.f, 10.f, -10.f, 1.f, 1.f, 1.f);
    Objects scene[4];
    scene[0].obj = (void*)(sphere + 0);
    scene[1].obj = (void*)(sphere + 1);
    scene[2].obj = (void*)(sphere + 2);
    scene[3].obj = (void*)(&plane);
    scene[0].type = 's';
    scene[1].type = 's';
    scene[2].type = 's';
    scene[3].type = 'p';
    Scene = scene;
    Vector3D pixel;

    //for (i = 0; i < w; i++){
    //    x = linspace(S[0], S[2], w, i);
    //    for (j = 0; j < h; j++){
    //        y = linspace(S[1], S[3], h, j);
    //        Q.x = x;
    //        Q.y = y;
    //        pixel = intersectColor(O, normalize(minus(Q,O)), 1);
    //        //Bdisp_SetPoint_VRAM(i,h-j-1,colorVectorToRGB565(pixel));
    //        printf("[%d,%d,%d],",i,h-j-1,colorVectorToRGB565(pixel));
    //    }
    //}

    i = -1;
    while (1) {
        GetKey(&key);
        switch (key) {
            case KEY_CTRL_LEFT:
                i -= 1;
                break;
            case KEY_CTRL_RIGHT:
                i += 1;
                break;
            case KEY_CTRL_EXIT:
                return 0;
        }
        //for (k = 0; k < 8; k++){
            Q.x = linspace(S[0], S[2], w, i);
            for (j = 0; j < h; j++){
                Q.y = linspace(S[1], S[3], h, j);
                pixel = intersectColor(O, normalize(minus(Q,O)), 1);
                Bdisp_SetPoint_VRAM(i,h-j-1,colorVectorToRGB565(pixel));
            }
        //}
    }
    return 0;
}
