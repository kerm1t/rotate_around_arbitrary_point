R"(
#version 120
varying vec3 color;
//out Fragcolor;
void main()
{
//    gl_FragColor = vec4(color, 1.0);
    gl_FragColor = vec4(1,1,1, 1.0);
//  Fragcolor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
};
)"