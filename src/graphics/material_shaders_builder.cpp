//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2016 SuperTuxKart-Team
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

#include "graphics/material_shaders_builder.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/shader_files_manager.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "utils/cpp2011.hpp"

#include <sstream>

// ----------------------------------------------------------------------------
/** Return a string with the declaration of an attribute
 *  \param attribute The name of the vertex attributes to be used in shader
 */
void MaterialShadersBuilder::writeAttributeDeclaration(const std::string &attribute, std::ostringstream &code) const
{
    auto it = m_attributes.find(attribute);
    if(it == m_attributes.end())
    {
        Log::error("MaterialShadersBuilder", "Unknow attribute '%s'", attribute.c_str());
        return;
    }
    if (CVS->isARBExplicitAttribLocationUsable())
    {
        //write for example: "layout(location = 0) in vec3 Position;"
        code << "layout(location = " << it->second.first << ") in ";
        code << it->second.second  << " " << attribute << ";\n";
    }
    else
    {
        //write for example: "in vec3 Position;"
        code << "layout(location = " << it->second.first << ") in ";
        code << it->second.second  << " " << attribute << ";\n";
    }
} //writeAttributeDeclaration

// ----------------------------------------------------------------------------
std::string MaterialShadersBuilder::genFirstPassVertexShaderSource(
    const XMLNode &flags_node,
    const XMLNode &vertex_shader_node)
{
    std::ostringstream code;

    ShaderFilesManager::getInstance()->
        writePreprocessorDirectives(GL_VERTEX_SHADER, code);

    bool wind_sensitive;
    flags_node.get("wind_sensitive", &wind_sensitive);
    if(wind_sensitive)
        code << "#define Wind_sensitive;\n";
    
    bool has_normal_map;
    flags_node.get("normal_map", &has_normal_map);
    if(has_normal_map)
        code << "#define Has_normal_map;\n";    
    
    bool use_alpha;
    flags_node.get("alpha_ref", &use_alpha);
    if(use_alpha)
        code << "#define Use_alpha;\n"; 
    
    code << ShaderFilesManager::getInstance()->getHeader();
    
    writeAttributeDeclaration("Position", code);
    writeAttributeDeclaration("Normal", code);
    writeAttributeDeclaration("Texcoord", code);
    if(has_normal_map)
    {
        writeAttributeDeclaration("Tangent", code);
        writeAttributeDeclaration("Bitangent", code);       
    }
   
    if (CVS->isAZDOEnabled())
    {
        if(use_alpha)
            code << "layout(location = 11) in sampler2D Handle;\n";
        
        code << "layout(location = 12) in sampler2D SecondHandle;\n";
        
        if(has_normal_map)
            code << "layout(location = 14) in sampler2D FourthHandle;\n";
    }

    code << "\n";
    
    // Only read files first time
    if (m_generic_per_instance_vertex_data.empty())
    {
        m_generic_per_instance_vertex_data = ShaderFilesManager::getInstance()->
            readShaderFile("material_shaders/generic_per_instance_vertex_data.vert");
    }
            
    if (m_generic_solid_first_pass_vertex_shader.empty())
    {
        m_generic_solid_first_pass_vertex_shader = ShaderFilesManager::getInstance()->
            readShaderFile("material_shaders/generic_solid_first_pass.vert");
    }
    
    code << m_generic_per_instance_vertex_data;
    code << "\n";
    code << m_generic_solid_first_pass_vertex_shader;
    
    return code.str();
} //genFirstPassVertexShaderSource

// ----------------------------------------------------------------------------
std::string MaterialShadersBuilder::getMaterialShaderKey(const std::string &file,
                                                         const std::string &rendering_pass,
                                                        unsigned type)
{
    XMLNode *root = file_manager->createXMLTree(file_manager->getShader(file));
    const XMLNode *flags_node = root->getNode("flags");
    
    std::ostringstream shader_key;
    shader_key << "Generic_material_" << rendering_pass << "_" << type << "_";
    
    for(unsigned i=0;i<m_generic_shaders_parameters[rendering_pass].size();i++)
    {
        bool flag_value;
        flags_node->get(m_generic_shaders_parameters[rendering_pass][i], &flag_value);
        shader_key << flag_value;
    }
    
    return shader_key.str();
}

// ----------------------------------------------------------------------------
GLuint MaterialShadersBuilder::loadMaterialProgram(const XMLNode *flags_node,
                                                   const XMLNode *rendering_pass_node)
{
    //TODO
}
    

// ----------------------------------------------------------------------------
MaterialShadersBuilder::MaterialShadersBuilder()
{
    m_attributes["Position"]       = std::make_pair<int, std::string>(0,"vec3");
    m_attributes["Normal"]         = std::make_pair<int, std::string>(1,"vec3");
    m_attributes["Color"]          = std::make_pair<int, std::string>(2,"vec4");
    m_attributes["Texcoord"]       = std::make_pair<int, std::string>(3,"vec2");
    m_attributes["SecondTexcoord"] = std::make_pair<int, std::string>(4,"vec2");
    m_attributes["Tangent"]        = std::make_pair<int, std::string>(5,"vec3");
    m_attributes["Bitangent"]      = std::make_pair<int, std::string>(6,"vec3");
    
    m_generic_shaders_parameters["solid_first_pass"]
         = createVector<std::string>("normal_map", "alpha_ref", "wind_sensitive");
    
}

// ----------------------------------------------------------------------------
GLuint MaterialShadersBuilder::getMaterialProgram(const std::string &file,
                                                  const std::string &rendering_pass)
{
    auto it_file = m_material_programs.find(file);
    
    if (it_file != m_material_programs.end())
    {
        auto it_rendering_pass = it_file->second.find(rendering_pass);
        if (it_rendering_pass != it_file->second.end())
            return it_rendering_pass->second;
    }
        
    XMLNode *root = file_manager->createXMLTree(file_manager->getShader(file));
    const XMLNode *flags_node = root->getNode("flags");
    const XMLNode *rendering_passes_node = root->getNode("rendering_passes");
    if(!rendering_passes_node)
    {
        Log::error("MaterialShadersBuilder",
                    "Incomplete material file, no rendering_passes node: %s",
                    file);
        return 0;
    }
    return loadMaterialProgram(flags_node, rendering_passes_node->getNode(rendering_pass));
}

// ------------------------------------------------------------------------
void MaterialShadersBuilder::addMaterialShaders(const std::string &file)
{
    XMLNode *root = file_manager->createXMLTree(file_manager->getShader(file));
    const XMLNode *flags_node = root->getNode("flags");
    const XMLNode *rendering_passes_node = root->getNode("rendering_passes");
    
    if(!rendering_passes_node)
    {
        Log::error("MaterialShadersBuilder",
                    "Incomplete material file, no rendering_passes node: %s",
                    file);
        return;
    }
    
    loadMaterialProgram(flags_node, rendering_passes_node->getNode("solid_first_pass"));
    loadMaterialProgram(flags_node, rendering_passes_node->getNode("solid_second_pass"));
    loadMaterialProgram(flags_node, rendering_passes_node->getNode("shadow_pass"));
    
}   //addMaterialShaders

#endif   // !SERVER_ONLY
