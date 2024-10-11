#include "phong.h"


#include <iostream>

Phong::Phong()
{ }

Phong::Phong(Vector3D Kd_, Vector3D Ks_, float alpha_):
rho_d(Kd_), Ks(Ks_), alpha(alpha_){}


/*Vector3D Phong::getReflectance(const Vector3D& n, const Vector3D& wo, const Vector3D& wi) const {
    
    // Compute the ideal reflection direction
    Vector3D wr = 2 * dot(n, wi);
    wr = wr * n;
    wr = wr - wi;
    
    // Calculate the diffuse and specular components
    Vector3D diffuse = (rho_d / 3.14);
   
    Vector3D specular = (Ks * pow(dot(wo, wr), alpha));
    // Return the reflectance
    
    return Vector3D(diffuse + specular);


};*/

Vector3D Phong::getReflectance(const Vector3D& n, const Vector3D& wo, const Vector3D& wi) const {
    
    // Compute the ideal reflection direction
    Vector3D wr = 2 * dot(n, wi);
    wr = wr * n;
    wr = wr - wi;
    
    // Calculate the diffuse and specular components
    Vector3D diffuse = (rho_d / 3.14);
    float constant_s = (2 * 3.14 / (alpha + 1));
    Vector3D specular = (Ks * pow(dot(wo, wr), alpha))*constant_s;
    // Return the reflectance
    
    return Vector3D(diffuse + specular);


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

