R"(
#version 120
uniform mat4 MVP;
attribute vec3 vCol;
//layout (location = 0) attribute vec3 vPos;
attribute vec3 vPos;
varying vec3 color;
uniform vec3 T; // translate
void main()
{
///    vPos = vPos + vec3(-10,0,0);
    gl_Position = MVP * vec4((vPos + T), 1.0);
///    gl_Position = gl_Position + vec3(-10,0,0);
//    color = vCol;
};
)"