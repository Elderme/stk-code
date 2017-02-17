//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2017 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef SERVER_ONLY

#ifndef HEADER_MATERIAL_SHADERS_BUILDER_HPP
#define HEADER_MATERIAL_SHADERS_BUILDER_HPP

#include "graphics/gl_headers.hpp"
#include "utils/no_copy.hpp"
#include "utils/singleton.hpp"

#include <string>
#include <unordered_map>
#include <vector>

class XMLNode;

class MaterialShadersBuilder: public Singleton<MaterialShadersBuilder>, NoCopy
{
private:
    /**
     * Map from a vertex shader attribute to attribute location and GLSL type.
     * Used to generate material shaders.
     */
    std::unordered_map<std::string, std::pair<int, std::string>> m_attributes;
    
    /**
     * Generic shader parts: stored in order to load them only once
     */
    std::string m_generic_per_instance_vertex_data;
    std::string m_generic_solid_first_pass_vertex_shader;
    
    /**
     * Map from a rendering pass (solid_first_pass, solid_second_pass, shadow_pass, rsm_pass)
     * to a parameter list (normal_map, alpha_ref, wind_sensitive, etc.)
     * Used to generate "shader_key" in order to reuse the same shader 
     * for different materials when it is possible
     */
    std::unordered_map<std::string, std::vector<std::string>> m_generic_shaders_parameters;
    
    /**
    * Map from a shader key to a shader indentifier.
    */
    std::unordered_map<std::string, GLuint> m_material_shaders;
    
    /**
    * Map to a program indentifier.
    * First key is material file name, second key is rendering pass
    */
    std::unordered_map<std::string, std::unordered_map<std::string,GLuint>> m_material_programs;
    
    
    // ------------------------------------------------------------------------
    void writeAttributeDeclaration(const std::string &attribute,
                                   std::ostringstream &code) const;
    // ------------------------------------------------------------------------
    std::string genFirstPassVertexShaderSource(const XMLNode &flags_node,
                                               const XMLNode &vertex_shader_node);
    // ------------------------------------------------------------------------
    std::string getMaterialShaderKey(const std::string &file,
                                     const std::string &rendering_pass,
                                     unsigned type);
    // ------------------------------------------------------------------------
    GLuint loadMaterialProgram(const XMLNode *flags_node,
                               const XMLNode *rendering_pass_node);
    
public:
    MaterialShadersBuilder();

    GLuint      getMaterialProgram(  const std::string &file,
                                     const std::string &rendering_pass);
    
    // ------------------------------------------------------------------------
    void addMaterialShaders(const std::string &file);
};

#endif   //HEADER_MATERIAL_SHADERS_BUILDER_HPP

#endif   // !SERVER_ONLY
