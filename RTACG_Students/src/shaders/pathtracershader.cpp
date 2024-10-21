
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

Vector3D Pathtracershader::ComputeRadiance(const Ray& r, Scene& scene, int max_depth, Intersection itsscene, HemisphericalSampler hs, Vector3D Lo, Vector3D wi) const {
    
    float pdf = (1/(dot(wi,itsscene.normal)/3.14));
    if (r.depth < max_depth) {
        Ray r_new = Ray(itsscene.itsPoint, wi, r.depth + 1);
        Vector3D BRDF = itsscene.shape->getMaterial().getReflectance(itsscene.normal, -wi, r.d) * (dot(itsscene.normal, wi));
        Lo = ComputeRadiance(r_new, scene, max_depth, itsscene, hs, Lo, wi) * BRDF*pdf;
    }
    return Lo;
}

Vector3D Pathtracershader::computeColor(const Ray& r, const std::vector<Shape*>& objList, const std::vector<LightSource*>& lsList) const
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
            Vector3D at = Vector3D(0.2); //Ambient term
            Vector3D pd = its.shape->getMaterial().getDiffuseReflectance();
            Vector3D intensity;
            Vector3D radiance;
            Vector3D reflectance;

            double numSamples = 128;
            HemisphericalSampler hs = HemisphericalSampler();

            for (int i = 0; i < numSamples; i++) {
                Vector3D wi = hs.getSample(its.normal);
                float cos_theta = dot(its.normal, wi);
                float pdf = cos_theta / 3.14;
                
                Vector3D n = its.normal.normalized();
                Vector3D wo = -r.d;

                Ray robj;
                robj.o = its.itsPoint;
                robj.d = wi;

                Intersection its_hemis;

                if (Utils::getClosestIntersection(robj, objList, its_hemis)) {
                    intensity = its_hemis.shape->getMaterial().getEmissiveRadiance();
                    
                    radiance += ComputeRadiance(robj,scene, 1,its_hemis,hs,intensity,wi);
                    
                    
                }
                reflectance = its.shape->getMaterial().getReflectance(n, -wi, r.d);   //Phong
                

                finalColor += ((reflectance * radiance * dot(n, wi)) / (1/pdf));
            }
            finalColor = finalColor * (1 / numSamples) + at * pd;

        }
        return finalColor;
    }
    else {
        return bgColor;
    }
}

/*if (r.depth < robj.depth) {
                    Ray r_new = Ray(its.itsPoint, wi, r.depth + 1);
                    BRDF = its.shape->getMaterial().getReflectance(its.normal, wi, -r.d) * (dot(its.normal, wi)) / pdf;
                    finalColor = computeColor(r_new, objList, lsList) * BRDF;
                }*/