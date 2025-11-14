#include "transform.h"

using namespace math;
using namespace core;

const mat4& TransformData::getMatrix() const
{
    if (_dirty)
    {
        _matrix = mat4::makeTRS(localPosition, localRotation, localScale);
        _dirty = false;
    }
    return _matrix;
}

void TransformData::translate(const vec3& vec)
{
    localPosition += vec;
    _dirty = true;
}

void TransformData::scale(const vec3& vec)
{
    localScale = compMul(localScale, vec);
    _dirty = true;
}

void TransformData::rotate(const quaternion& rot)
{
    localRotation = localRotation * rot;
    _dirty = true;
}
