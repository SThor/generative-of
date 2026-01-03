OF_GLSL_SHADER_HEADER

void main()
{
    gl_FrontColor = gl_Color;
    gl_Position = ftransform();
}