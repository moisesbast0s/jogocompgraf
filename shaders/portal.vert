#version 120

varying vec2 vTexCoord;
varying vec3 vWorldPos;

void main()
{
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    vTexCoord   = gl_MultiTexCoord0.st;
    vWorldPos   = gl_Vertex.xyz;
}
