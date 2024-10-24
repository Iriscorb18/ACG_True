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

Vector3D Pathtracershader::ComputeRadiance(const Ray& r, const std::vector<Shape*>& objList, const std::vector<LightSource*>& lsList, int max_depth, Intersection itsscene, HemisphericalSampler hs, Vector3D wi) const {

    Vector3D Lo(0.0);
    float pdf = (1.0 / (2 * 3.14));
    Vector3D radiance(0.0);

    if (r.depth <= max_depth) {
        // If the material is emissive
        if (itsscene.shape->getMaterial().isEmissive()) {
            Lo = itsscene.shape->getMaterial().getEmissiveRadiance();
        }

        // If the material is specular
        else if (itsscene.shape->getMaterial().hasSpecular()) {
            Vector3D wi_new = -r.d.normalized();
            Vector3D wr = Utils::computeReflectionDirection(wi_new, itsscene.normal.normalized());
            Ray r_new = Ray(itsscene.itsPoint, wr, r.depth + 1);

            Intersection new_int;
            if (Utils::getClosestIntersection(r_new, objList, new_int)) {
                Lo = ComputeRadiance(r_new, objList, lsList, max_depth, new_int, hs, wr);
            }
        }

        // If the material has transmission (refraction)
        else if (itsscene.shape->getMaterial().hasTransmission()) {
            Vector3D wo = r.d.normalized();
            Vector3D n = itsscene.normal.normalized();
            float ratio = 0.7;

            // Invert ratio and normal for internal rays
            if (dot(wo, n) > 0) {
                ratio = 1 / ratio;
                n = -n;
            }

            // Compute refraction direction or handle total internal reflection
            Vector3D wt;
            if (1 - ratio * ratio * (1 - pow(dot(-wo, n), 2)) >= 0) {
                Vector3D a1 = -sqrt(1 - ratio * ratio * (1 - pow(dot(-wo, n), 2))) + ratio * dot(-wo, n);
                Vector3D a2 = (-wo) * ratio;
                wt = n * a1 - a2;
            }
            else {
                wt = Utils::computeReflectionDirection(wo, n);  // Total internal reflection
            }

            Ray refractRay = Ray(itsscene.itsPoint, wt, r.depth + 1);
            Intersection new_int;
            if (Utils::getClosestIntersection(refractRay, objList, new_int)) {
                Lo = ComputeRadiance(refractRay, objList, lsList, max_depth, new_int, hs, wt);
            }
        }

        // Handle diffuse or glossy materials
        else if (itsscene.shape->getMaterial().hasDiffuseOrGlossy()) {
            Lo = itsscene.shape->getMaterial().getEmissiveRadiance();
            HemisphericalSampler hs_new = HemisphericalSampler();
            Vector3D wi_new = hs_new.getSample(itsscene.normal);
            Ray r_new = Ray(itsscene.itsPoint, wi_new, r.depth + 1);
            Vector3D BRDF = itsscene.shape->getMaterial().getReflectance(itsscene.normal, wi_new, -r_new.d) * dot(itsscene.normal, wi_new);
            Intersection new_int;
            if (Utils::getClosestIntersection(r_new, objList, new_int)) {
                Lo += ComputeRadiance(r_new, objList, lsList, max_depth, new_int, hs, wi_new) * BRDF / pdf;
            }

        }

    }
    return Lo;
}

Vector3D Pathtracershader::computeColor(const Ray& r, const std::vector<Shape*>& objList, const std::vector<LightSource*>& lsList) const
{
    Intersection its;
    Vector3D Ltotal(0.0);

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

        // If the material is emissive
        else if (its.shape->getMaterial().isEmissive()) {
            finalColor = its.shape->getMaterial().getEmissiveRadiance();
        }

        // Handle diffuse or glossy materials
        else if (its.shape->getMaterial().hasDiffuseOrGlossy()) {
            Vector3D at = Vector3D(0.2); //Ambient term
            Vector3D pd = its.shape->getMaterial().getDiffuseReflectance();
            Vector3D intensity;
            Vector3D radiance;
            Vector3D reflectance;

            double numSamples = 256;
            HemisphericalSampler hs = HemisphericalSampler();
            float pdf = 1.0 / (2.0 * 3.14);

            Vector3D n = its.normal.normalized();
            Vector3D wo = -r.d;

            for (int i = 0; i < numSamples; i++) {
                Vector3D wi = hs.getSample(its.normal);

                Ray robj;
                robj.o = its.itsPoint;
                robj.d = wi;

                Intersection its_hemis;

                if (Utils::getClosestIntersection(robj, objList, its_hemis)) {
                    intensity = its_hemis.shape->getMaterial().getEmissiveRadiance();

                    radiance = ComputeRadiance(robj, objList, lsList, 2, its_hemis, hs, wi);

                    Ltotal = radiance + intensity;
                }
                reflectance = its.shape->getMaterial().getReflectance(n, -wi, r.d);   //Phong

                finalColor += ((reflectance * Ltotal * dot(n, wi)) / pdf);
            }
            finalColor = finalColor * (1.0 / (double)numSamples);

        }
        return finalColor;
    }
    else {
        return bgColor;
    }
}