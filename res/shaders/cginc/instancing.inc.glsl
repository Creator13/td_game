#version 460 core

struct CustomData {
    uint c0, c1, c2, c3;
};

struct InstanceData {
    mat4 transform;
    mat4 invTransform;
    CustomData customData;
};

layout (std430, binding = 2) readonly buffer instanceSSBO {
    InstanceData instances[];
};

#define INSTANCE_TRANSFORM (instances[gl_BaseInstance + gl_InstanceID].transform)
#define INSTANCE_TRANSFORM_INVERSE (instances[gl_BaseInstance + gl_InstanceID].invTransform)
#define INSTANCE_DATA (instances[gl_BaseInstance + gl_InstanceID].customData)
