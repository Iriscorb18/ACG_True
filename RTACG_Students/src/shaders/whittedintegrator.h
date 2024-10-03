#include "shader.h"


class WhittedIntshader : public Shader
{
public:
    WhittedIntshader();
    WhittedIntshader(Vector3D whittedlColor, Vector3D bgColor_);

    virtual Vector3D computeColor(const Ray& r,
        const std::vector<Shape*>& objList,
        const std::vector<LightSource*>& lsList) const;

    Vector3D WhittedColor;
};
