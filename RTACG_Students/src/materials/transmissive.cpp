#include "transmissive.h"

#include <iostream>

Transmissive::Transmissive(const double r) 
{ }

Transmissive::Transmissive(Vector3D Ke_, Vector3D rho_d_):
Ke(Ke_), rho_d(rho_d_){}

Vector3D Transmissive::getReflectance(const Vector3D& n, const Vector3D& wo,
    const Vector3D& wi) const {
    return  rho_d/3.1416;
};

double Transmissive::getIndexOfRefraction() const
{
    std::cout << "Warning! Calling \"Material::getIndexOfRefraction()\" for a non-transmissive material"
              << std::endl;


    return r;
}


Vector3D Transmissive::getEmissiveRadiance() const
{
    return Ke;
}


Vector3D Transmissive::getDiffuseReflectance() const
{
    return rho_d;
}
