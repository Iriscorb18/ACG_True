
#include "shader.h"
#include "hemisphericaldirectshader.h"
#include "../materials/phong.h"
#include "../core/utils.h"
#include "../lightsources/lightsource.h"



HemisphericalDirectshader::HemisphericalDirectshader() :
    HDColor(Vector3D(1, 0, 0))
{ }

HemisphericalDirectshader::HemisphericalDirectshader(Vector3D HDColor_, Vector3D bgColor_) :
    Shader(bgColor_), HDColor(HDColor_)
{ }

Vector3D HemisphericalDirectshader::computeColor(const Ray& r, const std::vector<Shape*>& objList, const std::vector<LightSource*>& lsList) const
{
    Intersection its;
    
    if (Utils::getClosestIntersection(r, objList, its)) { //Loop through all objects to see their closest intersection
        Vector3D finalColor = Vector3D(0.0, 0.0, 0.0);

        // Handle specular materials
        if (its.shape->getMaterial().hasSpecular()) { //Check if material is specular
            Vector3D wi = -r.d.normalized();
            Vector3D N = its.normal.normalized(); 
            Vector3D wr = Utils::computeReflectionDirection(wi, N); // Calling function to compute reflection vector
            Ray reflectRay = Ray(its.itsPoint, wr, r.depth + 1);           
            finalColor += computeColor(reflectRay, objList, lsList); //Adding it to the finalColor
        }

        // Handle transmissive materials (refraction)
        else if (its.shape->getMaterial().hasTransmission()){
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
            if (1 - ratio * ratio * (1 - (pow(dot(-wo, n), 2))) >= 0){  //radical is higher than zero
                Vector3D a1 = -sqrt(1 - ratio * ratio * (1 - (pow(dot(-wo, n), 2)))) + ratio * dot(-wo, n); 
                Vector3D a2 = (-wo) * ratio; 
                wt = n * a1 - a2; 
            } else {
                wt = Utils::computeReflectionDirection(wo, n); //Total internal reflection if radical is smaller than zero
            }

            Ray refractray = Ray(its.itsPoint, wt, r.depth + 1);
            finalColor += computeColor(refractray, objList, lsList);
        }

        // Handle diffuse or glossy materials
        else if (its.shape->getMaterial().hasDiffuseOrGlossy()) {
            Vector3D at = Vector3D(0.15, 0.15, 0.15); //Ambient term
            Vector3D pd = its.shape->getMaterial().getDiffuseReflectance();
            int Vs = 1;
            
            for (int i = 0; i < lsList.size(); i++) { 
                Vector3D wi = (lsList.at(i)->sampleLightPosition() - its.itsPoint).normalized();
                Vector3D n = its.normal.normalized();
                Vector3D wo = -r.d;
                Vector3D emitted_radiance = lsList.at(i)->getIntensity();
                Vector3D reflectance = its.shape->getMaterial().getReflectance(n, wo, wi);   //Phong
                
                // Create shadow ray
                Ray robj;
                robj.o = its.itsPoint;
                robj.d = wi;
                robj.maxT = (robj.o - lsList.at(i)->sampleLightPosition()).length(); //We need to make a maximum to avoid this on to be infinite 
                if (Utils::hasIntersection(robj, objList)) {  //Not having visibility
                    Vs = 0;
                }
                finalColor += reflectance* lsList.at(i)->getIntensity() * Vs * dot(n, wi);
            } 
            finalColor += finalColor + at * pd;
        }

        // Handle emissive materials
        else if (its.shape->getMaterial().isEmissive()) {
            Vector3D finalColor(0.0, 0.0, 0.0);
            Vector3D n = -its.normal.normalized(); 
            Vector3D wo = -r.d.normalized(); 

            int N = 256;  // Number of samples
            double p_omega = 1.0 / (2.0 * 3.14);  // Probability

            for (int i = 0; i < lsList.size(); i++) {
                //int Vs = 1;

                // Sample the hemisphere
                Vector3D wi = (lsList.at(i)->sampleLightPosition() - its.itsPoint).normalized();

                // Create shadow ray
                Ray robj;
                robj.o = its.itsPoint;
                robj.d = wi;
                robj.maxT = (robj.o - lsList.at(i)->sampleLightPosition()).length(); //We need to make a maximum to avoid this on to be infinite 
                //if (Utils::hasIntersection(robj, objList)) {  //Not having visibility
                //    Vs = 0;
                //}
                

                for (int j = 0; j < N; ++j) {
                    if (!Utils::hasIntersection(robj, objList)) {
                        // Emitted radiance from the light source
                        Vector3D L_dir_i = lsList.at(i)->getIntensity();

                        // BRDF value at this point (Phong or other BRDF)
                        Vector3D brdf = its.shape->getMaterial().getReflectance(n, wo, wi);

                        // Accumulate the radiance using Monte Carlo integration
                        finalColor += (L_dir_i * brdf * dot(n, wi)) / (p_omega * N);
                    }
                }
            }
            
            return finalColor;
        }
    }
       
    else {
        return bgColor;
    }
}

