uniform int layer;

#ifndef Use_Instancing
uniform mat4 ModelMatrix;
#endif

#if __VERSION__ >= 330
layout(location = 0) in vec3 Position;
layout(location = 3) in vec2 Texcoord;

#ifdef Use_Instancing
layout(location = 7) in vec3 Origin;
layout(location = 8) in vec3 Orientation;
layout(location = 9) in vec3 Scale;

#ifdef Use_Bindless_Texture
layout(location = 11) in uvec2 Handle;
#endif

#endif //Use_Instancing

#else
in vec3 Position;
in vec2 Texcoord;

#ifdef Use_Instancing
in vec3 Origin;
in vec3 Orientation;
in vec3 Scale;
#endif //Use_Instancing

#endif //__VERSION__ >= 330

#ifdef VSLayer
out vec2 uv;
#ifdef Use_Bindless_Texture
flat out uvec2 handle;
#endif
#else
out vec2 tc;
out int layerId;
#ifdef Use_Bindless_Texture
flat out uvec2 hdle;
#endif
#endif //VSLayer

#ifdef Use_Instancing
#stk_include "utils/getworldmatrix.vert"
#endif

void main(void)
{
#ifdef Use_Instancing
    mat4 ModelMatrix = getWorldMatrix(Origin, Orientation, Scale);
#endif
    
#ifdef VSLayer
    gl_Layer = layer;
    uv = Texcoord;
    gl_Position = ShadowViewProjMatrixes[gl_Layer] * ModelMatrix * vec4(Position, 1.);
#ifdef Use_Bindless_Texture
    handle = Handle;
#endif

#else
    layerId = layer;
    tc = Texcoord;
    gl_Position = ShadowViewProjMatrixes[layerId] * ModelMatrix * vec4(Position, 1.);
#ifdef Use_Bindless_Texture
    hdle = Handle;
#endif
#endif
}
