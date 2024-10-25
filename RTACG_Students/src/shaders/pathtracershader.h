#include "shader.h"
//#include "../core/scene.h"
//#include "../core/hemisphericalsampler.h"


class Pathtracershader : public Shader
{
public:
    Pathtracershader();
    Pathtracershader(Vector3D PTColor, Vector3D bgColor_);

    virtual Vector3D computeColor(const Ray& r, const std::vector<Shape*>& objList, const std::vector<LightSource*>& lsList/*, int max_depth*/) const;

    virtual Vector3D ComputeRadiance(const Ray& r, const std::vector<Shape*>& objList, const std::vector<LightSource*>& lsList, int max_depth/*, Intersection itsscene, HemisphericalSampler hs, Vector3D wi*/) const;
    
    Vector3D PTColor;
};