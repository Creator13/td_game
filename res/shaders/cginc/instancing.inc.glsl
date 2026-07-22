#version 460 core

struct CustomData {
    uint c0, c1, c2, c3;
};

struct InstanceData {
    mat4 transform;
    CustomData customData;
};

layout (std430, binding = 2) readonly buffer instanceSSBO {
    InstanceData instances[];
};

#define INSTANCE_TRANSFORM (instances[gl_BaseInstance + gl_InstanceID].transform)
#define INSTANCE_DATA (instances[gl_BaseInstance + gl_InstanceID].customData)
