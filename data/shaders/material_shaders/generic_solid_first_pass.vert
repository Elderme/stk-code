out vec3 nor;
out vec2 uv;

#ifdef Has_normal_map
out vec3 tangent;
out vec3 bitangent;
#endif Has_normal_map

#ifdef Use_Bindless_Texture
#ifdef Use_alpha
flat out sampler2D handle;
#endif
flat out sampler2D secondhandle;
#ifdef Has_normal_map
flat out sampler2D fourthhandle;
#endif
#endif

#ifdef Use_Instancing
#stk_include "utils/getworldmatrix.vert"
#endif

void main(void)
{
    /* Step 1: compute gl_Position and normal*/
#ifdef Wind_sensitive
#ifdef Use_Instancing
    mat4 ModelMatrix = getWorldMatrix(Origin + windDir * Color.r, Orientation, Scale);
    mat4 TransposeInverseModelView = transpose(getInverseWorldMatrix(Origin + windDir * Color.r, Orientation, Scale) * InverseViewMatrix);
    mat4 ModelViewProjectionMatrix = ProjectionViewMatrix * ModelMatrix;
#else    
    mat4 new_model_matrix = ModelMatrix;
    mat4 new_inverse_model_matrix = InverseModelMatrix;
    new_model_matrix[3].xyz += windDir * Color.r;

    // FIXME doesn't seem to make too much difference in pass 2, because this
    // affects "nor" which is later only * 0.1 by scattering
    new_inverse_model_matrix[3].xyz -= windDir * Color.r;

    mat4 ModelViewProjectionMatrix = ProjectionViewMatrix * new_model_matrix;
    mat4 TransposeInverseModelView = transpose(InverseViewMatrix * new_inverse_model_matrix);
#endif    

#else
#ifdef Use_Instancing
    mat4 ModelMatrix = getWorldMatrix(Origin, Orientation, Scale);
    mat4 TransposeInverseModelView = transpose(getInverseWorldMatrix(Origin, Orientation, Scale) * InverseViewMatrix);
#else
    mat4 TransposeInverseModelView = transpose(InverseModelMatrix * InverseViewMatrix);
#endif
    mat4 ModelViewProjectionMatrix = ProjectionViewMatrix * ModelMatrix;
#endif

    gl_Position = ModelViewProjectionMatrix * vec4(Position, 1.);
    
    // Keep orthogonality
    nor = (TransposeInverseModelView * vec4(Normal, 0.)).xyz;

    
    /* Step 2: compute tangent and bitangent if the material has a normal map */
#ifdef Has_normal_map
    // Keep direction
    tangent = (ViewMatrix * ModelMatrix * vec4(Tangent, 0.)).xyz;
    bitangent = (ViewMatrix * ModelMatrix * vec4(Bitangent, 0.)).xyz;
#endif


    /* Step 3: Compute texture coordinates */
#ifdef Use_Instancing 
    uv = vec2(Texcoord.x + misc_data.x, Texcoord.y + misc_data.y);
#else
    uv = vec2(Texcoord.x + texture_trans.x, Texcoord.y + texture_trans.y);
#endif


    /* Step 4: Pass handles to fragment shader*/
#ifdef Use_Bindless_Texture
#ifdef Use_alpha
    handle = Handle;
#endif
    secondhandle = SecondHandle;
#ifdef Has_normal_map
    fourthhandle = FourthHandle;
#endif
#endif
}
