#version 330 core

// Mesh layout
// location 0: position
// location 1: normal
// location 2: uv
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Proj;
uniform mat3 NormalMatrix;

out VS_OUT
{
    vec3 WorldPos;
    vec3 WorldNormal;
    vec2 UV;
} vs_out;

void main()
{
    vec4 world = Model * vec4(aPos, 1.0);
    vs_out.WorldPos = world.xyz;
    vs_out.WorldNormal = normalize(NormalMatrix * aNormal);
    vs_out.UV = aUV;

    gl_Position = Proj * View * world;
}
