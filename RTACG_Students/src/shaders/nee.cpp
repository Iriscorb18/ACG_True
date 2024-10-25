#include "shader.h"
#include "nee.h"
#include "../materials/phong.h"
#include "../core/utils.h"
#include "../lightsources/lightsource.h"
#include "../core/hemisphericalsampler.h"
#include "../core/scene.h"


Neeshader::Neeshader() :
    NEEColor(Vector3D(1, 0, 0))
{ }

Neeshader::Neeshader(Vector3D NEEColor_, Vector3D bgColor_) :
    Shader(bgColor_), NEEColor(NEEColor_)
{ }

Vector3D Neeshader::ComputeRadiance(const Ray& r, const std::vector<Shape*>& objList, const std::vector<LightSource*>& lsList, int depth, Intersection its) const {
    Intersection its_scene;
    // Find intersection of ray with scene objects
    if (!Utils::getClosestIntersection(r, objList, its_scene)) {
        return bgColor; // Background color if no hit
    }

    Vector3D Lr(0.0);
    Vector3D Le(0.0);

    int numSamples = 256;
    for (int i = 0; i < numSamples; i++) {

        // Reflected radiance from the surface (calls the reflected radiance function)
       
        Lr += ReflectedRadiance(its_scene, -r.d, depth, objList, lsList);
    }
    Lr /= (double)numSamples;
    Le = its_scene.shape->getMaterial().getEmissiveRadiance();

    // Return the sum of emitted and reflected radiance
    return (Le+Lr);
}


Vector3D Neeshader::ReflectedRadiance(const Intersection& its, const Vector3D& wo, int depth, const std::vector<Shape*>& objList, const std::vector<LightSource*>& lsList) const {
    // Compute direct illumination
    Vector3D Ldir = DirectRadiance(its, wo, objList, lsList);

    // Compute indirect illumination (via hemisphere sampling for global illumination)
    Vector3D Lind = IndirectRadiance(its, wo, depth, objList, lsList);

    // Return the sum of direct and indirect radiance
    return Ldir + Lind;
}

Vector3D Neeshader::DirectRadiance(const Intersection& its, const Vector3D& wo, const std::vector<Shape*>& objList, const std::vector<LightSource*>& lsList) const {
    Vector3D directIllumination(0.0);

    for (int j = 0; j < lsList.size(); j++) {
        // Sample random point on the light source
        Vector3D y = lsList.at(j)->sampleLightPosition();
        double pdf = 1.0 / lsList.at(j)->getArea(); // Assuming uniform area sampling

        // Compute direction to light source
        Vector3D wi = (y - its.itsPoint).normalized();

        // Visibility check
        Ray shadowRay(its.itsPoint, wi);
        shadowRay.maxT = (y - its.itsPoint).length() - 0.01;
        bool visible = !Utils::hasIntersection(shadowRay, objList); // Visibility test

        // If the light is visible, compute the contribution
        if (visible) {
            Vector3D Le = lsList.at(j)->getIntensity(); // Light intensity
            Vector3D brdf = its.shape->getMaterial().getReflectance(its.normal, wi, wo); // BRDF
            Vector3D ny = lsList.at(j)->getNormal(); // Light surface normal
            double G = (dot(its.normal, wi) * dot(-wi, ny)) / (y - its.itsPoint).lengthSq(); // Geometric term

            // Monte Carlo estimate for direct lighting
            directIllumination += Le * brdf * G / pdf;
        }
    }
    return directIllumination;
}


Vector3D Neeshader::IndirectRadiance(const Intersection& its, const Vector3D& wo, int depth, const std::vector<Shape*>& objList, const std::vector<LightSource*>& lsList) const {
    Vector3D Lind(0.0);
    
    const int maxDepth = 2; // Set a reasonable depth for recursive ray tracing
    
    // Sample a new direction from the hemisphere over the surface normal
    HemisphericalSampler hs;
    Vector3D wi = hs.getSample(its.normal);
    double pdf = 1.0 / (2.0 * 3.14); // Hemisphere sampling PDF

    // Create a new ray for indirect lighting
    Ray newRay(its.itsPoint, wi, depth + 1);

    // Intersect with the scene
    Intersection newIts;

    if (Utils::getClosestIntersection(newRay, objList, newIts)) {
        if (depth < maxDepth) { // Stop recursion at max depth
            // Recursively calculate indirect lighting
            Vector3D brdf = its.shape->getMaterial().getReflectance(its.normal, wi, wo);
            Lind += ReflectedRadiance(newIts, -wi, depth + 1, objList, lsList) * brdf * dot(its.normal, wi) / pdf;
        }
        return Lind;
    }
}


Vector3D Neeshader::computeColor(const Ray& r, const std::vector<Shape*>& objList, const std::vector<LightSource*>& lsList) const
{
    Intersection its;
    Scene scene;  

    if (Utils::getClosestIntersection(r, objList, its)) { //Loop through all objects to see their closest intersection
        Vector3D finalColor = Vector3D(0.0, 0.0, 0.0);

        // Handle specular materials
        if (its.shape->getMaterial().hasSpecular()) { //Check if material is specular
            Vector3D wi = -r.d.normalized();
            Vector3D N = its.normal.normalized();
            Vector3D wr = Utils::computeReflectionDirection(wi, N); // Calling function to compute reflection vector
            Ray reflectRay = Ray(its.itsPoint, wr, r.depth + 1);
            finalColor = computeColor(reflectRay, objList, lsList); //Adding it to the finalColor
        }

        // Handle transmissive materials (refraction)
        else if (its.shape->getMaterial().hasTransmission()) {
            Vector3D wt;
            Vector3D n = its.normal.normalized();
            Vector3D wo = r.d.normalized();
            double ratio = 0.7;

            // Invert ratio and normal for internal rays
            if (dot(wo, n) > 0) {
                ratio = 1 / ratio;
                n = -n;
            }

            // Check for total internal reflection
            if (1 - ratio * ratio * (1 - (pow(dot(-wo, n), 2))) >= 0) {  //radical is higher than zero
                Vector3D a1 = -sqrt(1 - ratio * ratio * (1 - (pow(dot(-wo, n), 2)))) + ratio * dot(-wo, n);
                Vector3D a2 = (-wo) * ratio;
                wt = n * a1 - a2;
            }
            else {
                wt = Utils::computeReflectionDirection(wo, n); //Total internal reflection if radical is smaller than zero
            }

            Ray refractray = Ray(its.itsPoint, wt, r.depth + 1);
            finalColor = computeColor(refractray, objList, lsList);
        }
        else if (its.shape->getMaterial().isEmissive()) {
            finalColor = its.shape->getMaterial().getEmissiveRadiance();
        }
        // Handle diffuse or glossy materials
        
        else if (its.shape->getMaterial().hasDiffuseOrGlossy()) {
            Vector3D reflectedRadiance = ComputeRadiance(r, objList, lsList, r.depth, its);
            finalColor =  reflectedRadiance;
        } 
        // Return the final color computed (emissive, reflective, or refracted)
        return finalColor;
    }
}
