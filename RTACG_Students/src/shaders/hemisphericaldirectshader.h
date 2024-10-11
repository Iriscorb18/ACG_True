#include "shader.h"


class HemisphericalDirectshader : public Shader
{
public:
    HemisphericalDirectshader();
    HemisphericalDirectshader(Vector3D HDColor, Vector3D bgColor_);

    virtual Vector3D computeColor(const Ray& r,
        const std::vector<Shape*>& objList,
        const std::vector<LightSource*>& lsList) const;

    Vector3D HDColor;
};
