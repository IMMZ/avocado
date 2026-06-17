#version 450

#extension GL_EXT_nonuniform_qualifier : enable

layout(location=0) in vec3 fragColor;
layout(location=1) in vec2 fragTexCoord;

layout(location=0) out vec4 outColor;

/*layout(push_constant) uniform Material {
    int id;
} material;*/

layout(set=1, binding=1) uniform sampler2D baseColorTextures[];

void main() {
    //outColor = texture(baseColorTextures[material.id], fragTexCoord);
    outColor = texture(baseColorTextures[0], fragTexCoord);
}

