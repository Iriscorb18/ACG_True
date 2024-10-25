#include "shader.h"
#include "pathtracershader.h"
#include "../materials/phong.h"
#include "../core/utils.h"
#include "../lightsources/lightsource.h"
#include "../core/hemisphericalsampler.h"
#include "../core/scene.h"

Pathtracershader::Pathtracershader() :
    PTColor(Vector3D(1, 0, 0))
{ }

Pathtracershader::Pathtracershader(Vector3D PTColor_, Vector3D bgColor_) :
    Shader(bgColor_), PTColor(PTColor_)
{ }

Vector3D Pathtracershader::ComputeRadiance(const Ray& r, const std::vector<Shape*>& objList, const std::vector<LightSource*>& lsList, int max_depth) const {
    Intersection its;

    // Base case: if no intersection, return background color
    if (!Utils::getClosestIntersection(r, objList, its)) {
        return bgColor;
    }

    // Emissive materials directly contribute to the radiance
    Vector3D Lo = its.shape->getMaterial().getEmissiveRadiance();

    // Stop recursion if we've reached the maximum depth
    if (r.depth >= max_depth) {
        return Lo;
    }

    // Handle specular materials
    if (its.shape->getMaterial().hasSpecular()) {
        Vector3D wi = -r.d.normalized();
        Vector3D N = its.normal.normalized();
        Vector3D wr = Utils::computeReflectionDirection(wi, N); // Calling function to compute reflection vector
        Ray reflectRay = Ray(its.itsPoint, wr, r.depth + 1);
        return ComputeRadiance(reflectRay, objList, lsList, max_depth);
    }

    // Handle transmissive materials (refraction)
    if (its.shape->getMaterial().hasTransmission()) {
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
        return ComputeRadiance(refractray, objList, lsList, max_depth);
    }

    // Handle diffuse or glossy materials
    if (its.shape->getMaterial().hasDiffuseOrGlossy()) {
        // Sample a random direction on the hemisphere at the intersection point for diffuse/glossy
        HemisphericalSampler hs = HemisphericalSampler();
        Vector3D wi = hs.getSample(its.normal); // ωi: Incoming light direction
        float pdf = 1.0 / (2.0 * 3.14); // Assuming uniform hemisphere sampling

        // Create the new ray to trace in the sampled direction
        Ray newRay = Ray(its.itsPoint, wi, r.depth + 1);

        // Recursively compute the incoming radiance for the new ray
        Vector3D incomingRadiance = ComputeRadiance(newRay, objList, lsList, max_depth);

        // Compute BRDF, dot product, and accumulate the radiance
        Vector3D wo = -r.d;  // Outgoing direction (reverse of ray direction)
        Vector3D BRDF = its.shape->getMaterial().getReflectance(its.normal, wi, wo); // Assuming a BRDF function (e.g., Phong)
        Lo += incomingRadiance * BRDF * dot(its.normal, wi) / pdf;
    }

    return Lo;
}

Vector3D Pathtracershader::computeColor(const Ray& r, const std::vector<Shape*>& objList, const std::vector<LightSource*>& lsList) const {
    const int numSamples = 256; // Number of samples per pixel for Monte Carlo integration
    const int max_depth = 5;    // Maximum path depth
    Vector3D finalColor(0.0);

    for (int i = 0; i < numSamples; i++) {
        // Trace the path and accumulate the result
        finalColor += ComputeRadiance(r, objList, lsList, max_depth);
    }

    // Return the averaged color
    return finalColor * (1.0 / numSamples);
}
