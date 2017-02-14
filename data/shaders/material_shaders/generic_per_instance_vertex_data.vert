#ifdef GL_ES
uniform mat4 ModelMatrix;
uniform mat4 InverseModelMatrix;
uniform vec2 texture_trans;
#else
#ifdef Use_Instancing
layout(location = 7) in vec3 Origin;
layout(location = 8) in vec3 Orientation;
layout(location = 9) in vec3 Scale;
layout(location = 10) in vec4 misc_data;
#else
uniform mat4 ModelMatrix =
    mat4(1., 0., 0., 0.,
         0., 1., 0., 0.,
         0., 0., 1., 0.,
         0., 0., 0., 1.);
uniform mat4 InverseModelMatrix =
    mat4(1., 0., 0., 0.,
         0., 1., 0., 0.,
         0., 0., 1., 0.,
         0., 0., 0., 1.);

uniform vec2 texture_trans = vec2(0., 0.);
#endif //Use_Instancing
#endif //GL_ES
