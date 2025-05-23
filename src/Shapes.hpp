#ifndef SHAPESHEADER
#define SHAPESHEADER

#include "Point.hpp"
#include "Ray.hpp"
#include "Transform.hpp"
#include "AABB.hpp"

inline const float almostZero = 1e-8f;

class Shape
{
public:
    virtual double rayIntersect(Ray &ray) { return {}; }

    virtual Vector getNormal(Ray &ray, const double t) { return {}; }

    virtual Point getPoint() { return {}; }

    /*
    Para o funcionamento ideal desse método, aplique uma única operação
    a partir de um operador montado previamente por uma sequência de outros operadores.

    Caso seja aplicada diversas transformações, podem ocorrer erros de aproximação
    devido a normalização de vetores que ocorre após a aplicação da transformação.
    Quanto mais discrepantes as proporções entre os valores da matriz
    e os dos vetores, mais chance de erro após sucessivas transformações
    */
    virtual void applyTransform(const Matrix &transformMatrix) { return; }

    virtual Matrix getTransform() const { return Matrix(); }
    
    virtual AABB getBoundingBox() const { return AABB(); }
};

#ifndef SPHEREHEADER
#define SPHEREHEADER

class Sphere : public Shape
{
public:
    Point center;
    AABB boundingBox;
    double R;

    Sphere(Point center, double radius) : center(center)
    {
        R = radius;
        boundingBox = AABB(center - Vector(R, R, R), center + Vector(R, R, R));

    }

    // retorna centro da esfera
    Point getPoint()
    {
        return center;
    }

    Vector getNormal(Ray &ray, const double t)
    {
        Point P = ray.getPoint(t);
        Vector normalVec = (P - center).normalize();
        return normalVec;
    }

    double rayIntersect(Ray &ray)
    {
        Vector diffVec = ray.from - center;

        double B = -2 * diffVec.dot(ray.direction);
        double C = diffVec.dot(diffVec) - R * R;
        double delta = B * B - 4 * C;

        if (delta < almostZero)
            return -1;

        double t0 = (B + sqrt(delta)) / 2;
        double t1 = (B - sqrt(delta)) / 2;

        if (t1 > almostZero)
            return t1;
        if (t0 > almostZero)
            return t0;

        return -1;
    }
    
    AABB getBoundingBox() const override
    {
        return boundingBox;
    }

    void applyTransform(const Matrix &transformMatrix)
    {
        center = transformMatrix * center;

        /*
        Extraindo o fator de escala da matriz de transformação composta:
        Excluindo a quarta linha e a quarta coluna, as colunas
        da matriz resultante representam vetores de uma base do R3.
        Pegamos a magnitude de cada vetor para saber o fator de escala em cada direcao.
        */
        double sx = sqrt(pow(transformMatrix(0, 0), 2) + pow(transformMatrix(1, 0), 2) + pow(transformMatrix(2, 0), 2)); // vetor i transformado
        double sy = sqrt(pow(transformMatrix(0, 1), 2) + pow(transformMatrix(1, 1), 2) + pow(transformMatrix(2, 1), 2)); // vetor j transformado
        double sz = sqrt(pow(transformMatrix(0, 2), 2) + pow(transformMatrix(1, 2), 2) + pow(transformMatrix(2, 2), 2)); // vetor k transformado

        /*
        Aproximacao:
        Calculamos a media das escalas em cada eixo
        para que seja isotropica e mantenha a forma de esfera
        */
        double scaleFactor = (sx + sy + sz) / 3;

        R *= scaleFactor;
    }
};

#endif

#ifndef PLANEHEADER
#define PLANEHEADER

class Plane : public Shape
{
public:
    Vector normalVec;
    Point P0;
    AABB boundingBox;

    Plane(Vector normal, Point planePoint)
    {
        normalVec = normal.normalize();
        P0 = planePoint;
        boundingBox = AABB();
    }

    // retorna ponto usado para formar o plano
    Point getPoint()
    {
        return P0;
    }

    void setPlaneBB(const AABB& sceneBounds)
    {
        boundingBox = sceneBounds;
    }

    AABB getBoundingBox() const override
    {
        return boundingBox;
    }

    Vector getNormal(Ray &ray, const double t)
    {
        Vector align = ray.from - P0;
        if (normalVec.dot(align) > almostZero)
            return normalVec;
        return normalVec * -1;
    }

    double rayIntersect(Ray &ray)
    {
        double aux = ray.direction.dot(normalVec);
        if (abs(aux) < almostZero)
            return -1;

        double t = ((P0 - ray.from).dot(normalVec)) / aux;
        if (t > almostZero)
            return t;
        return -1;
    }

    inline Point pointIntersect(Ray ray) {
        double t = this->rayIntersect(ray);
        if (t > almostZero) {
            return ray.getPoint(t);
        }
        return ray.from;
    }

    void applyTransform(const Matrix &transformMatrix)
    {
        P0 = transformMatrix * P0;
        normalVec = (transformMatrix * normalVec).normalize();
    }
};

#endif

#ifndef TRIANGLEHEADER
#define TRIANGLEHEADER

class Triangle : public Plane
{
public:
    Point p0, p1, p2;
    AABB boundingBox;
    Vector normalVec;
    Vector edge0, edge1;
    double dot00, dot01, dot11, denom;

    Triangle(Point p0, Point p1, Point p2, Vector normalVec) : Plane(normalVec, p0), p0(p0), p1(p1), p2(p2)
    {
        edge0 = p1 - p0;
        edge1 = p2 - p0;
        dot00 = edge0.dot(edge0);
        dot01 = edge0.dot(edge1);
        dot11 = edge1.dot(edge1);
        denom = dot00 * dot11 - dot01 * dot01;

        Point small = minBound(minBound(p0, p1), p2);
        Point big = maxBound(maxBound(p0, p1), p2);
    
        boundingBox = AABB(small, big);
    }

    double rayIntersect(Ray &ray)
    {
        double t = Plane::rayIntersect(ray);

        if (t < almostZero)
            return -1;

        Point P = ray.getPoint(t);
        Vector C = P - p0;

        double dotC0 = C.dot(edge0);
        double dotC1 = C.dot(edge1);

        if (denom == 0)
            return -1;

        double beta = (dotC0 * dot11 - dotC1 * dot01) / denom;
        double gamma = (dotC1 * dot00 - dotC0 * dot01) / denom;
        double alpha = 1.0 - beta - gamma;

        if (alpha < almostZero || beta < almostZero || gamma < almostZero)
            return -1;

        return t;
    }

    AABB getBoundingBox() const override
    {
        return boundingBox;
    }

    void applyTransform(const Matrix &transformMatrix)
    {
        p0 = transformMatrix * p0;
        p1 = transformMatrix * p1;
        p2 = transformMatrix * p2;
        normalVec = (transformMatrix * normalVec).normalize();
    }
};

#endif

#endif // SHAPEHEADER