#include <vector>
#include <cmath>
#define _USE_MATH_DEFINES
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <algorithm>

//-----------------------------------------------------------------VEC4
class vec4{
public:

    double e[4];

    vec4()
    {
        e[0]=0;
        e[1]=0;
        e[2]=0;
        e[3]=0;
    }

    vec4(double x, double y, double z, double w=0)
    {
        e[0]=x;
        e[1]=y;
        e[2]=z;
        e[3]=w;
    }

    double x() const { return e[0]; }
    double y() const { return e[1]; }
    double z() const { return e[2]; }
    double w() const { return e[3]; }

    double& operator[](int i)
    {
        return e[i];
    }

    double operator[](int i) const
    {
        return e[i];
    }

    vec4 operator+(const vec4& v) const
    {
        return vec4(
            x()+v.x(),
            y()+v.y(),
            z()+v.z(),
            w()+v.w()
        );
    }

    vec4 operator-(const vec4& v) const
    {
        return vec4(
            x()-v.x(),
            y()-v.y(),
            z()-v.z(),
            w()-v.w()
        );
    }

    vec4 operator*(double t) const
    {
        return vec4(
            x()*t,
            y()*t,
            z()*t,
            w()*t
        );
    }

    vec4 operator/(double t) const
    {
        return (*this)*(1/t);
    }

    double length() const{
        return std::sqrt(length_squared());
    }

    double length_squared() const{
        return e[0]*e[0] + e[1]*e[1] + e[2]*e[2];
    }

    bool near_zero() const{
        const double s = 1e-8;

        return (std::fabs(e[0]) < s) && (std::fabs(e[1]) < s) && (std::fabs(e[2]) < s);
    }

    vec4 operator-() const{
        return vec4( -e[0], -e[1], -e[2], -e[3]);
    }

    vec4& operator+=(const vec4& v){
        e[0] += v.e[0];
        e[1] += v.e[1];
        e[2] += v.e[2];
        e[3] += v.e[3];

        return *this;
    }

    vec4& operator*=(double t){
        e[0] *= t;
        e[1] *= t;
        e[2] *= t;
        e[3] *= t;

        return *this;
    }

    vec4& operator/=(double t){
        return *this *= 1/t;
    }

};

//----------------------------------------------operador global
inline vec4 operator*(double t, const vec4& v){
    return v * t;
}

inline vec4 operator*(const vec4& u, const vec4& v)
{
    return vec4(
        u.x()*v.x(),
        u.y()*v.y(),
        u.z()*v.z(),
        u.w()*v.w()
    );
}

//----------------------------------------------produto escalar
inline double dot(const vec4& u, const vec4& v){
    return
        u.x()*v.x() +
        u.y()*v.y() +
        u.z()*v.z();
}

//-----------------------------------------------produto vetorial
inline vec4 cross(const vec4& u, const vec4& v){
    return vec4(
        u.y()*v.z() - u.z()*v.y(),
        u.z()*v.x() - u.x()*v.z(),
        u.x()*v.y() - u.y()*v.x(),
        0.0
    );
}

//-----------------------------------------------vetor unitario
inline vec4 unit_vector(vec4 v){
    return v / v.length();
}

inline vec4 reflect(const vec4& v,const vec4& n)
{
    return v - 2.0*dot(v,n)*n;
}


inline vec4 refract(const vec4& uv,const vec4& n,double eta)
{
    double cos_theta = std::min(dot(-uv,n),1.0);

    vec4 r_out_perp = eta*(uv + cos_theta*n);

    vec4 r_out_parallel =
        -sqrt(fabs(1.0-r_out_perp.length_squared()))*n;

    return r_out_perp + r_out_parallel;
}

using point4 = vec4;
using direction4 = vec4;
using color = vec4;

//------------------------------------------------------------------------MAT4
class mat4{
public:

    double m[4][4];

    mat4()
    {
        for(int i=0;i<4;i++)
        {
            for(int j=0;j<4;j++)
            {
                m[i][j]=0;
            }
        }
    }

    static mat4 identity()
    {
        mat4 I;

        for(int i=0;i<4;i++)
            I.m[i][i]=1;

        return I;
    }

    static mat4 translate(double tx,double ty,double tz)
    {
        mat4 T = identity();

        T.m[0][3]=tx;
        T.m[1][3]=ty;
        T.m[2][3]=tz;

        return T;
    }

    static mat4 scale(double sx,double sy,double sz)
    {
        mat4 S = identity();

        S.m[0][0]=sx;
        S.m[1][1]=sy;
        S.m[2][2]=sz;

        return S;
    }
};

inline vec4 operator*(const mat4& A,const vec4& v)
{
    vec4 r;

    for(int i=0;i<4;i++)
    {
        r[i]=0;

        for(int j=0;j<4;j++)
        {
            r[i]+=A.m[i][j]*v[j];
        }
    }

    return r;
}

inline mat4 operator*(const mat4& A,const mat4& B)
{
    mat4 R;

    for(int i=0;i<4;i++)
    {
        for(int j=0;j<4;j++)
        {
            R.m[i][j]=0;

            for(int k=0;k<4;k++)
            {
                R.m[i][j]+=A.m[i][k]*B.m[k][j];
            }
        }
    }

    return R;
}

//-------------------------------------------------------------------------RAIO
class ray {
public:
    vec4 orig;
    vec4 dir;

    ray() {}

    ray(const vec4& origin, const vec4& direction)
        : orig(origin), dir(direction) {}

    vec4 origin() const { return orig; }
    vec4 direction() const { return dir; }

    vec4 at(double t) const{
        return orig + t * dir;
    }
};

//-------------------------------------------------------------------------textura
class Texture{
public:

    unsigned char* data;

    int width;
    int height;
    int channels;

    Texture(const char* filename){
        data = nullptr;
        width = 0;
        height = 0;
        channels = 0;

        data = stbi_load(
            filename,
            &width,
            &height,
            &channels,
            3
        );

        if(data == nullptr)
        {
            std::cout << "Erro ao carregar: "
                << filename
                << std::endl;

            std::cout << "Motivo: "
                << stbi_failure_reason()
                << std::endl;
        }
    }

    ~Texture()
    {
        if(data)
            stbi_image_free(data);
    }

    color sample(double u,double v) const
    {
        if(!data)
            return color(1,0,1);

        u = std::clamp(u,0.0,1.0);
        v = std::clamp(v,0.0,1.0);

        int x = int(u*(width-1));
        int y = int((1-v)*(height-1));

        int index = (y*width+x)*3;

        return color(
            data[index]/255.0,
            data[index+1]/255.0,
            data[index+2]/255.0
        );
    }

    double sampleHeight(double u,double v) const{
        color c = sample(u,v);

        return (c.x() + c.y() + c.z()) / 3.0;
    }

};

//-------------------------------------------------------------------------tipo Material
enum MaterialType{
    PLASTIC,
    METAL,
    GLASS,
    EARTH
};

//-------------------------------------------------------------------------Material
class Material{
public:

    color albedo;

    MaterialType type;

    Texture* texture;
    Texture* bump_texture;

    Material(){
        albedo = color(1,1,1);

        type = PLASTIC;

        texture = nullptr;
        bump_texture = nullptr;
    }

    bool isReflective() const{
        return type == METAL || type == GLASS;
    }
    bool isRefractive() const{
        return type == GLASS;
    }
    bool hasTexture() const{
        return texture != nullptr;
    }
    bool hasBump() const{
        return bump_texture != nullptr;
    }

    static Material Plastic(color c){
    Material m;

    m.albedo = c;

    m.type = PLASTIC;

    return m;
    }

    static Material Metal(color c){
        Material m;

        m.albedo = c;

        m.type = METAL;

        return m;
    }

    static Material Glass() {
        Material m;

        m.albedo = color(0.9, 0.95, 1.0);

        m.type = GLASS;

        return m;
    }

    static Material Earth( Texture* tex, Texture* bump = nullptr){
        Material m;

        m.albedo = color(1,1,1);

        m.type = EARTH;

        m.texture = tex;
        m.bump_texture = bump;

        return m;
    }

    static Material Brick( Texture* tex, Texture* bump = nullptr){
        Material m;

        m.albedo = color(1,1,1);

        m.type = PLASTIC;

        m.texture = tex;
        m.bump_texture = bump;

        return m;
    }
};

//-------------------------------------------------------------------------calcular UV
void get_sphere_uv(const point4& p, double& u, double& v)
{
    double theta = acos(-p.y());
    double phi = atan2(-p.z(), p.x()) + M_PI;

    u = phi / (2.0 * M_PI);
    v = theta / M_PI;
}

//-------------------------------------------------------------------------OBJ triangulo
class Triangle {
public:

    vec4 v0;
    vec4 v1;
    vec4 v2;
    Material material;

    Triangle(){}

    Triangle(vec4 a, vec4 b, vec4 c, Material mat = Material())    {
        v0 = a;
        v1 = b;
        v2 = c;
        material = mat;
    }

    bool hit(const ray& r, double& t) const;

    vec4 normal() const{//-----------------------------------normal do triângulo
        vec4 edge1 = v1 - v0;
        vec4 edge2 = v2 - v0;

        return unit_vector(cross(edge1, edge2));
    }

};

bool Triangle::hit(const ray& r, double& t) const{//---------------------- Möller-Trumbore (calculo raio-triangulo)
    const double EPSILON = 0.000001;

    vec4 edge1 = v1 - v0;
    vec4 edge2 = v2 - v0;

    vec4 h = cross(r.direction(), edge2);

    double a = dot(edge1, h);

    if (fabs(a) < EPSILON)
        return false;

    double f = 1.0 / a;

    vec4 s = r.origin() - v0;

    double u = f * dot(s, h);

    if (u < 0.0 || u > 1.0)
        return false;

    vec4 q = cross(s, edge1);

    double v = f * dot(r.direction(), q);

    if (v < 0.0 || u + v > 1.0)
        return false;

    t = f * dot(edge2, q);

    if (t > EPSILON)
        return true;

    return false;
}

//-------------------------------------------------------------------------ESFERA
class sphere {
public:
    vec4 center;
    double radius;
    Material material;

    sphere(vec4 cen, double r, Material mat = Material()){
        center = cen;
        radius = r;
        material = mat;
    }

    bool hit(const ray& r, double& t) const{
        vec4 oc = r.origin() - center;

        auto a = dot(r.direction(), r.direction());
        if(r.direction().length() < 0.9 || r.direction().length() > 1.5){
            std::cout << "Direcao estranha: "
                    << r.direction().length()
                    << std::endl;
        }
        auto b = 2.0 * dot(oc, r.direction());
        auto c = dot(oc, oc) - radius*radius;

        auto discriminant = b*b - 4*a*c;

        if (discriminant < 0)
            return false;


        auto root = (-b - sqrt(discriminant)) / (2*a);

        if (root < 0){
            root = (-b + sqrt(discriminant)) / (2*a);

            if (root < 0)
                return false;
        }

        t = root;

        return true;
    }
};

class AABB{
public:

    vec4 minimum;
    vec4 maximum;

    AABB(){}

    AABB(vec4 min, vec4 max)
    {
        minimum = min;
        maximum = max;
    }

    bool hit(const ray& r) const;
};

bool AABB::hit(const ray& r) const
{
    double tmin = 0.0;
    double tmax = 1e30;

    for(int axis = 0; axis < 3; axis++)
    {
        double invD = 1.0 / r.direction()[axis];

        double t0 =
            (minimum[axis] - r.origin()[axis]) * invD;

        double t1 =
            (maximum[axis] - r.origin()[axis]) * invD;

        if(invD < 0.0)
            std::swap(t0,t1);

        tmin = std::max(tmin,t0);
        tmax = std::min(tmax,t1);

        if(tmax <= tmin)
            return false;
    }

    return true;
}

//-------------------------------------------------------------------------Nó
class Node{
public:
    mat4 transform;

    std::vector<sphere> spheres;

    std::vector<Triangle> triangles;

    std::vector<Node> children;

    Node(){
        transform = mat4::identity();
    }

    void setPosition(double x,double y,double z){
        transform = mat4::translate(x,y,z);
    }

    void addSphere(const sphere& s)
    {
        spheres.push_back(s);
    }

    void addTriangle(const Triangle& t)
    {
        triangles.push_back(t);
    }

    void addChild(const Node& n)
    {
        children.push_back(n);
    }
};

//-------------------------------------------------------------------------CAMERA
class camera {
public:
    point4 origin;
    point4 lower_left_corner;
    vec4 horizontal;
    vec4 vertical;

    camera(double aspect_ratio, double vfov) {

        auto theta = vfov * M_PI / 180.0;
        auto h = tan(theta/2);

        auto viewport_height = 2.0 * h;
        auto viewport_width = aspect_ratio * viewport_height;

        origin = point4(0,0,0,1);

        horizontal = vec4(viewport_width,0,0,0);
        vertical = vec4(0,viewport_height,0,0);

        lower_left_corner = origin - horizontal/2 - vertical - vec4(0,0,1,0);
    }

    ray get_ray(double u, double v) const {
        return ray(
            origin,
            lower_left_corner + u*horizontal + v*vertical - origin
        );
    }
};

//-------------------------------------------------------------------------COR
inline void write_color(unsigned char* img, int index, color pixel_color) {
    img[index]     = (int)(255.999 * pixel_color.x());
    img[index + 1] = (int)(255.999 * pixel_color.y());
    img[index + 2] = (int)(255.999 * pixel_color.z());
}

//--------------------------------------------------------------------------------------- load obj
bool load_obj( const std::string& filename, std::vector<Triangle>& triangles, point4 position, double scale, Material mat = Material()){
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cout << "Erro ao abrir: "
                  << filename << std::endl;
        return false;
    }

    std::vector<vec4> vertices;

    std::string line;

    while (std::getline(file, line))
    {
        std::stringstream ss(line);

        std::string type;
        ss >> type;

        //-------------------------
        // VÉRTICES
        //-------------------------

        if (type == "v")
        {
            double x, y, z;

            ss >> x >> y >> z;

            vertices.push_back(
                vec4(x, y, z, 1)
            );
        }

        //-------------------------
        // FACES
        //-------------------------

        else if (type == "f")
        {
            std::string s1, s2, s3;

            ss >> s1 >> s2 >> s3;

            auto get_index =
            [](const std::string& s)
            {
                return std::stoi(
                    s.substr(
                        0,
                        s.find('/')
                    )
                );
            };

            int i1 = get_index(s1);
            int i2 = get_index(s2);
            int i3 = get_index(s3);

            mat4 transform =
            mat4::translate(position.x(), position.y(), position.z()) *
            mat4::scale(scale, scale, scale);

            vec4 v1 = transform * vertices[i1-1];
            vec4 v2 = transform * vertices[i2-1];
            vec4 v3 = transform * vertices[i3-1];

            triangles.push_back(
                Triangle(v1, v2, v3, mat)
            );
        }
    }

    std::cout << "Vertices: "
              << vertices.size()
              << std::endl;

    std::cout << "Triangulos: "
              << triangles.size()
              << std::endl;

    return true;
}

//---------------------------------------------------------------FUNÇÃO PARA PERCORRER A ARVORE
void collectObjects( const Node& node, std::vector<sphere>& world, std::vector<Triangle>& triangles, mat4 accumulated_transform){

    mat4 current = accumulated_transform * node.transform;

    for(auto s : node.spheres)
    {
        s.center =
            s.center = current * s.center;

        world.push_back(s);
    }

    for(auto t : node.triangles)
    {
        t.v0 = current * t.v0;
        t.v1 = current * t.v1;
        t.v2 = current * t.v2;

        triangles.push_back(t);
    }

    for(const auto& child : node.children)
    {
        collectObjects(
            child,
            world,
            triangles,
            current
        );
    }
}

//---------------------------------------------------------------------------------------SOMBRA
bool in_shadow( vec4 p, vec4 normal, const std::vector<sphere>& world, const std::vector<Triangle>& triangles, vec4 light_position){

    vec4 origin = p + 0.001 * normal;

    vec4 light_dir = light_position - p;

    light_dir = light_dir / light_dir.length();

    ray shadow_ray(p + normal * 0.001, light_dir);

    for (const auto& obj : world){
        double t;

        if (obj.hit(shadow_ray, t)){
            return true;
        }
    }

    for(const auto& tri : triangles){
        double t;

        if(tri.hit(shadow_ray,t))
            return true;
    }

    return false;
}
//------------------------------------------------------------------------------------

const int MAX_DEPTH = 8;

color ray_color( const ray& r, const std::vector<sphere>& world, const std::vector<Triangle>& triangles, int depth, const AABB& scene_box) {

    if(!scene_box.hit(r)){
        vec4 unit_direction = r.direction()/r.direction().length();

        auto t = 0.5*(unit_direction.y()+1.0);

        return (1.0-t)*color(1,1,1) + t*color(0.5,0.7,1);
    }

    if(depth <= 0)
        return color(0,0,0);//-------------evitar recursão infinita

    point4 light_position(0,1,0,1); //------------ adicionando uma luz

    double closest_t = 1e30;

    const sphere* sphere_hit = nullptr;//--------esfera mais proxima
    const Triangle* triangle_hit = nullptr;//----triangulo mais proximo

    for(const auto& s : world){//-------------guarda o objeto mais proximo - esfera
        double t;

        if(s.hit(r,t)){
            if(t < closest_t){
                closest_t = t;
                sphere_hit = &s;
                triangle_hit = nullptr;
            }
        }
    }

    for(const auto& tri : triangles){//-------------guarda o objeto mais proximo - triangulo
        double t;

        if(tri.hit(r,t))
        {
            if(t < closest_t)
            {
                closest_t = t;
                triangle_hit = &tri;
                sphere_hit = nullptr;
            }
        }
    }

    if(sphere_hit != nullptr){//--------------------------------------------------ILUMINAÇÃO ESFERA
        const Material& material = sphere_hit->material;
        vec4 p = r.at(closest_t);

        vec4 normal = (p - sphere_hit->center) / sphere_hit->radius;

        double u, v;

        get_sphere_uv(normal, u, v);//-----------------------------------------conversão para uv

        //---------------------------------------------------------vê se tem bump
        double h = 0.5;
        double hL = 0.5;
        double hR = 0.5;
        double hD = 0.5;
        double hU = 0.5;
        double dx = hR - hL;
        double dy = hU - hD;
        //-----------------------------------------------------so se o material tiver bump
        if(material.hasBump()){
            double du = 1.0 / material.bump_texture->width;
            double dv = 1.0 / material.bump_texture->height;

            h  = material.bump_texture->sampleHeight(u,v);

            hL = material.bump_texture->sampleHeight(u-du,v);
            hR = material.bump_texture->sampleHeight(u+du,v);

            hD = material.bump_texture->sampleHeight(u,v-dv);
            hU = material.bump_texture->sampleHeight(u,v+dv);

            dx = hR - hL;
            dy = hU - hD;
        }

        vec4 tangent( -normal.z(), 0, normal.x(), 0);

        if(tangent.length() < 0.0001)
            tangent = vec4(1,0,0,0);

        tangent = tangent / tangent.length();

        vec4 bitangent = cross(normal, tangent);
        bitangent = bitangent / bitangent.length();

        double strength = 5.0;

        normal = normal + strength * dx * tangent + strength * dy * bitangent;

        normal = normal / normal.length();
        //---------------------------------------------------------------------------------------

        vec4 reflected = reflect(r.direction(), normal);

        vec4 unit_direction = r.direction() / r.direction().length();

        double refraction_index = 1.0 / 1.5;

        vec4 refracted = refract( unit_direction, normal, refraction_index);

        ray refracted_ray( p - normal*0.001, refracted);

        color refracted_color(0,0,0);//----------------------------------------------------------REFRAÇÃO

        if(material.isRefractive()){
            refracted_color = ray_color( refracted_ray, world, triangles, depth-1, scene_box);
        }

        ray reflected_ray( p + normal*0.001, reflected);

        vec4 light_dir = light_position - p;

        light_dir = light_dir / light_dir.length();

        //--------------------------------------------------iluminação de phong
        double ambient = 0.15;

        double diffuse =std::max(0.0, dot(normal, light_dir));

        vec4 view_dir = -p;

        view_dir =view_dir / view_dir.length();

        vec4 reflect_dir = reflect(-light_dir, normal);

        reflect_dir = reflect_dir / reflect_dir.length();

        double specular = pow(std::max(dot(view_dir, reflect_dir), 0.0), 32);

        if (in_shadow(p, normal, world, triangles, light_position)){
            diffuse *= 0.25;
            specular *= 0.10;
        }

        double phong = ambient + diffuse + specular;

        if (phong > 1.0){
            phong = 1.0;
        }

        color reflected_color(0,0,0);
        //-------------------------------------------------------------------------------REFLEXÃO
        if(material.isReflective()){
            reflected_color = ray_color( reflected_ray, world, triangles, depth-1, scene_box);
        }

        color surface_color;
        //--------------------------------------------------------------------------------TEXTURA
        if(material.hasTexture()){
            surface_color = material.texture->sample(u, v);
        }
        else{
            surface_color = material.albedo;
        }

        if(material.isRefractive()){
            return 0.25 * surface_color + 0.35 * reflected_color + 0.40 * refracted_color;
        }
        else{
            if(material.isReflective()){
                return 0.6 * (phong * surface_color) + 0.4 * reflected_color;
            }
            else{
                return 0.9 * (phong * surface_color) + 0.1 * reflected_color;
            }
        }
    }

    if(triangle_hit != nullptr){//----------------------------------------------------------ILUMINAÇÃO DO TRIÂNGULO
        
        const Material& material = triangle_hit->material;
        
        point4 p = r.at(closest_t);

        vec4 normal = triangle_hit->normal();

        double ambient = 0.15;

        vec4 light_dir = light_position - p;
        light_dir = light_dir / light_dir.length();

        double diffuse =
            std::max(
                0.0,
                dot(normal, light_dir)
           );

        vec4 view_dir = -p;
        view_dir = view_dir / view_dir.length();

        vec4 reflect_dir = reflect(-light_dir, normal);
        reflect_dir = reflect_dir / reflect_dir.length();

        double specular = pow(std::max(dot(view_dir, reflect_dir),0.0),32);

        if (in_shadow(p, normal, world, triangles, light_position)){
            diffuse *= 0.25;
            specular *= 0.10;
        }

        double phong = ambient + diffuse + specular;

        if(phong > 1.0)
            phong = 1.0;

        // ----------------------------------------------------------------------------------- reflexão 

        vec4 reflected = reflect(r.direction(), normal);

        ray reflected_ray( p + normal*0.001, reflected);

        color reflected_color(0,0,0);

        if(material.isReflective()){
            reflected_color = ray_color( reflected_ray, world, triangles, depth-1, scene_box);
        }

        // ---------------------------------------------------------------------------------- refração 

        vec4 unit_direction = r.direction() / r.direction().length();

        double refraction_index = 1.0 / 1.5;

        vec4 refracted =refract( unit_direction, normal, refraction_index);

        ray refracted_ray( p - normal*0.001, refracted);

        color refracted_color(0,0,0);

        if(material.isRefractive()){
            refracted_color = ray_color(refracted_ray, world, triangles, depth-1, scene_box);
        }

        color surface_color = material.albedo;
        // ---------------- RETORNO ----------------


        if(material.isRefractive()){
            return 0.35 * surface_color + 0.35 * reflected_color + 0.30 * refracted_color;
        }
        else{
            if(material.isReflective()){
                return 0.6 * (phong * surface_color) + 0.4 * reflected_color;
            }
            else{
                return 0.9 * (phong * surface_color) + 0.1 * reflected_color;
            }
        }

    }
    

    vec4 unit_direction = r.direction() / r.direction().length();

    auto t = 0.5 * (unit_direction.y() + 1.0);

    return (1.0 - t)*color(1.0, 1.0, 1.0)
         + t*color(0.5, 0.7, 1.0);
}

int main() {

    const int image_width = 800;
    const int image_height = 600;


    const double aspect_ratio = double(image_width)/ double(image_height);

    std::vector<unsigned char> image(
        image_width * image_height * 3
    );

    //------------------------------------CAMERA
    camera cam(aspect_ratio, 45);

    
    Texture brick_texture("red_brick_4k.jpg");
    Texture brick_bump("red_brick_bump_4k.jpg");

    Texture orange_texture("food_0022_scattering_1k.jpg");
    Texture orange_bump("food_0022_roughness_1k.jpg");
    //Texture texture("terra.jpg");
    //Texture bump_texture("terra_bump.jpg");

    //-------------------------------------------------material
    Material red = Material::Plastic(color(1,0,0));
    Material white = Material::Plastic(color(1,1,1));
    Material green = Material::Plastic(color(0,1,0));
    Material grass = Material::Plastic(color(0,1,0.5));
    Material blue = Material::Plastic(color(0,0,1));
    Material yellow = Material::Plastic(color(1,1,0));
    Material ciano = Material::Plastic(color(0,1,1));
    Material magenta = Material::Plastic(color(1,0,1));
    Material orange_color = Material::Plastic(color(1,0.65,0));
    Material roxo = Material::Plastic(color(0.5,0,0.5));
    Material mirror = Material::Metal(color(0.7,0.7,0.7));
    Material mirror2 = Material::Metal(color(1,1,1));
    Material glass = Material::Glass();

    Material brick = Material::Brick(&brick_texture, &brick_bump);
    Material orange = Material::Brick(&orange_texture);//, &orange_bump

    //------------------------------------MÚLTIPLAS ESFERAS
    std::vector<sphere> world;
    std::vector<Triangle> triangles;

    Node root;
    Node group;
    Node child;

    group.setPosition(0,-2,-2);

    //group.addSphere(
        //sphere(vec4(0,-0.25,-4, 1), 1.0, glass)
    //);

    group.addSphere(
        sphere(vec4(0,-1,-6, 1), 3, orange)
    );

    //group.addSphere(
        //sphere(vec4(0,-21,-5,1), 20, white)
    //);

    //child.setPosition(3,-2,-7);

    //child.addSphere(
        //sphere(vec4(0,-110,-6, 1), 100, mirror2)
    //);

    //load_obj( "cubo.obj", triangles, vec4(0,0,-10,1), 3, Material::Brick(&brick_texture, &brick_bump));
    //load_obj( "cubo.obj", triangles, vec4(0,-6,-3,1), 4, Material::Plastic(color(1,1,1)));//---------------------OBJETO

    //for(const auto& t : triangles)
        //group.addTriangle(t);

    group.addChild(child);

    root.addChild(group);

    ray teste(
        vec4(0,0,0,1),
        vec4(0,0,-1, 0)
    );

    double t;

    //for(size_t i = 0; i < triangles.size(); i++){//-------------------------VERIFICAÇÃO OBJETO
        //if(triangles[i].hit(teste,t)){
            //std::cout << "Acertou triangulo "<< i<< std::endl;
        //}
    //}

    world.clear();
    triangles.clear();

    collectObjects(
        root,
        world,
        triangles,
        mat4::identity()
    );

    AABB scene_box(
        vec4(-101,-101,-20, 0),
        vec4(101,101,2, 0)
    );


    int index = 0;

    for (int j = image_height - 1; j >= 0; --j) {

        for (int i = 0; i < image_width; ++i) {

            auto u = double(i) / (image_width - 1);
            auto v = double(j) / (image_height - 1);

            ray r = cam.get_ray(u, v);

            color pixel_color = ray_color(r, world, triangles, MAX_DEPTH, scene_box);

            write_color(image.data(), index, pixel_color);

            index += 3;
        }
    }

    stbi_write_png(
        "render.png",
        image_width,
        image_height,
        3,
        image.data(),
        image_width * 3
    );

    return 0;
}