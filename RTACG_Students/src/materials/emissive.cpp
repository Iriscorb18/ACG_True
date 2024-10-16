#include "emissive.h"
#include "phong.h"
#include <iostream>

Emissive::Emissive()
{ }

Emissive::Emissive(Vector3D Ke_, Vector3D rho_d_):
Ke(Ke_), rho_d(rho_d_){}

Vector3D Emissive::getReflectance(const Vector3D& n, const Vector3D& wo,
    const Vector3D& wi) const {
    Vector3D wr = 2 * dot(n, wi);
    wr = wr * n;
    wr = wr - wi;
    Vector3D Ks = Vector3D(1.0, 1.0, 1.0);
    float alpha = 32;
    // Calculate the diffuse and specular components
    Vector3D diffuse = (rho_d / 3.14);
    float constant_s = (2 * 3.14 / (alpha+1));
    Vector3D specular = (Ks * pow(dot(wo, wr), alpha)) * constant_s;
    // Return the reflectance

    return Vector3D(diffuse + specular);
};

double Emissive::getIndexOfRefraction() const
{
    std::cout << "Warning! Calling \"Material::getIndexOfRefraction()\" for a non-transmissive material"
              << std::endl;

    return -1;
}


Vector3D Emissive::getEmissiveRadiance() const
{
    return Ke;
}


Vector3D Emissive::getDiffuseReflectance() const
{
    return rho_d;
}
