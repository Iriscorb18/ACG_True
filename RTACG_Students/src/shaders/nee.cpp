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

//COMPUTING INDIRECT RADIANCE
Vector3D Neeshader::ComputeRadiance(const Ray& r, const std::vector<Shape*>& objList, const std::vector<LightSource*>& lsList, int max_depth, Intersection itsscene, HemisphericalSampler hs, Vector3D wi, double A) const {

    Vector3D Lo(0.0);

    Vector3D radiance(0.0);
    if (r.depth <= max_depth) {
        Lo = itsscene.shape->getMaterial().getEmissiveRadiance();
        HemisphericalSampler hs_new = HemisphericalSampler();
        Vector3D wi_new = hs_new.getSample(itsscene.normal);
        Ray r_new = Ray(itsscene.itsPoint, wi_new, r.depth + 1);
        Vector3D BRDF = itsscene.shape->getMaterial().getReflectance(itsscene.normal, wi_new, -r_new.d) * (dot(itsscene.normal, wi_new));
        Intersection new_int;
        if (Utils::getClosestIntersection(r_new, objList, new_int)) {
            Lo += ComputeRadiance(r_new, objList, lsList, max_depth, new_int, hs, wi_new, A) * BRDF / (1 / A);
        }
        //Review if its transmissive or mirror view recursive iteration compute color
    }
    return Lo;
}
//ENDING OF FUCNTION

Vector3D Neeshader::computeColor(const Ray& r, const std::vector<Shape*>& objList, const std::vector<LightSource*>& lsList) const
{
    Intersection its;
    Scene scene;

    if (Utils::getClosestIntersection(r, objList, its)) { //Loop through all objects to see their closest intersection
        Vector3D finalColor = Vector3D(0.0, 0.0, 0.0);
        Vector3D reflectedradiance(0.0);
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

            Vector3D at = Vector3D(0.15); // Ambient term
            Vector3D pd = its.shape->getMaterial().getDiffuseReflectance(); // Diffuse reflectance
            Vector3D final_color(0.0);
            Vector3D finalDirectIllumination(0.0);
            Vector3D indirectillumination(0.0);
            Intersection its_hemis;
            HemisphericalSampler hs = HemisphericalSampler();
            Vector3D Le(0.0);

            double numSamples = 256;

            for (int j = 0; j < lsList.size(); j++) {
                // Monte Carlo integration: sample points on the area light

                double A = lsList.at(j)->getArea();  // Area of the light source
                for (int i = 0; i < numSamples; i++) {
                    // Sample a random point on the area light
                    Vector3D y = (lsList.at(j)->sampleLightPosition());

                    int Vs = 1;//keep changing the value at each sample since we have initialized before the usage of samples, making the first time being zero, always zero

                    // Compute wi (direction from intersection point x to sampled light point y)
                    Vector3D wi = (y - its.itsPoint).normalized();
                    double distanceSquared = (y - its.itsPoint).lengthSq();

                    // Compute visibility (check if the point on the light is visible from the intersection point)
                    Ray shadowRay(its.itsPoint, wi);

                    //(denoted as its.itsPoint) towards a sampled light position y, you want to know whether any object obstructs the line of sight between the two points.
                    shadowRay.maxT = (y - its.itsPoint).length() - 0.01; //We need to make a maximum to avoid this on to be infinite

                    if (Utils::hasIntersection(shadowRay, objList)) {  //Not having visibility
                        Vs = 0;
                    }
                    if (Vs == 1) {
                        //COMPUTING DIRECT RADIANCE
                        // Get the radiance emitted from the light at point y
                        Vector3D Le = lsList.at(j)->getIntensity();
                        // Compute the geometric term G(x, y)
                        Vector3D n = its.normal.normalized();
                        Vector3D ny = lsList.at(j)->getNormal();  // Normal of the light's surface
                        double G = (dot(n, wi) * dot(-wi, ny)) / distanceSquared;

                        // Get BRDF (Phong reflection or other)
                        Vector3D wo = -r.d;  // Outgoing direction
                        Vector3D reflectance = its.shape->getMaterial().getReflectance(n, -wi, wo);  // BRDF function

                        // Monte Carlo estimator for direct illumination
                        finalDirectIllumination = Le * reflectance * G * Vs / (1 / A);
                        //ENDING DIRECT RADIANCE

                        //REFLECTED RADIANCE
                        Vector3D wj = hs.getSample(its.normal);

                        indirectillumination = ComputeRadiance(shadowRay, objList, lsList, 2, its, hs, wj, A);
                        reflectedradiance += finalDirectIllumination + indirectillumination;
                    }
                }
            }
            final_color = Le + reflectedradiance;
            finalColor = final_color * (1 / numSamples) + at * pd;
        }
        return finalColor;
    }
    else {
        return bgColor;
    }
}