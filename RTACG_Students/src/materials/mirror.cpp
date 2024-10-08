#include "mirror.h"

#include <iostream>

Mirror::Mirror()
{ }

Mirror::Mirror(Vector3D Ke_, Vector3D rho_d_):
Ke(Ke_), rho_d(rho_d_){}

Vector3D Mirror::getReflectance(const Vector3D& n, const Vector3D& wo,
    const Vector3D& wi) const {
    return  rho_d/3.1416;
};

double Mirror::getIndexOfRefraction() const
{
    std::cout << "Warning! Calling \"Material::getIndexOfRefraction()\" for a non-transmissive material"
              << std::endl;

    return -1;
}


Vector3D Mirror::getEmissiveRadiance() const
{
    return Ke;
}


Vector3D Mirror::getDiffuseReflectance() const
{
    return rho_d;
}