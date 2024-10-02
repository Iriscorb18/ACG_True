#include "phong.h"


#include <iostream>

Phong::Phong()
{ }

Phong::Phong(Vector3D Kd_, Vector3D Ks_, float alpha_):
rho_d(Kd_), Ks(Ks_), alpha(alpha_){}


Vector3D Phong::getReflectance(const Vector3D& n, const Vector3D& wo,
    const Vector3D& wi) const {
    Vector3D dot_1 = dot(n, wi);
    Vector3D dot_2 = dot_1 * 2;
    Vector3D dot_normal = dot_2 * n;
    Vector3D wr = dot_normal - wi;
    Vector3D diffuse = rho_d / 3.14;
    double wrdotwo = dot(wo, wr);
    Vector3D specular = Ks * pow(wrdotwo, alpha);
    //FILL(...)

    // Calculate the specular component ks * (wo . wr)^alpha
    //double wr_dot_wo = std::max(dot(wo, wr), 0.0);
    //Vector3D specular = Ks * pow(wr_dot_wo, alpha);

    return diffuse+specular;

};

double Phong::getIndexOfRefraction() const
{
    std::cout << "Warning! Calling \"Material::getIndexOfRefraction()\" for a non-transmissive material"
              << std::endl;

    return -1;
}


Vector3D Phong::getEmissiveRadiance() const
{
    return Vector3D(0.0);
}


Vector3D Phong::getDiffuseReflectance() const
{
    return rho_d;
}

