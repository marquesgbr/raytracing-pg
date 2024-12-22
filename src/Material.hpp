#include <tuple>
#include "Scene.hpp"
class Material
{

public:
    Shape *shape;
    Vector color;
    double ka, kd, ks;
    int eta;
    double ior;
    double kr, kt;

    Material(Shape *shape, Vector color, double ka, double kd, double ks, double kr, double kt, int eta, double ior) : shape(shape), color(color / 255.0), ka(ka), kd(kd), ks(ks), kr(kr), kt(kt), eta(eta), ior(ior)
    {
    }

    Shape *getShape() const
    {
        return shape;
    }

    Vector shade(Point *point, Vector view, Vector *normal);

    static std::tuple<Material *, double> nearest(Ray ray);
};

std::vector<Material> objects;

Vector Material::shade(Point *point, Vector view, Vector *normal) // 112 128 144
{
    Vector resColor = color.elementWiseMultiplication(ambientLight * ka);

    for (Light light : lights)
    {
        Vector lightDirection = (light.position - *point).normalize();

        Vector r = ((*normal) * 2.0 * normal->dot(lightDirection)) - lightDirection;

        double t;
        Material *shadow;
        std::tie(shadow, t) = Material::nearest(Ray(*point, light.position));

        if (shadow == nullptr || lightDirection.dot(light.position - *point) > t)
        {
            double dotdiff = lightDirection.dot(*normal);
            if (dotdiff > 0)
            {
                resColor = resColor + color.elementWiseMultiplication(light.color) * kd * dotdiff * light.intensity;
            }

            double dotspec = r.dot(view);
            if (dotspec > 0)
            {
                resColor = resColor + light.color * ks * pow(dotspec, eta) * light.intensity;
            }
        }
    }
    return resColor;
}

std::tuple<Material *, double> Material::nearest(Ray ray)
{
    Material *hit = nullptr;
    double intersectT = INFINITY;

    for (Material &material : objects)
    {
        double t = material.getShape()->rayIntersect(ray);

        if (t > 0 && t < intersectT)
        {
            intersectT = t;
            hit = &material;
        }
    }
    if (intersectT != INFINITY)
    {
        return std::make_tuple(hit, intersectT);
    }
    return std::make_tuple(nullptr, INFINITY);
}