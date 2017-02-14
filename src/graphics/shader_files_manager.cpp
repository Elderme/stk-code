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

#include "graphics/shader_files_manager.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/shared_gpu_objects.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "utils/log.hpp"

#include <fstream>
#include <sstream>


std::unordered_map<std::string, std::pair<int, std::string>> ShaderFilesManager::m_attributes;

// ----------------------------------------------------------------------------
std::string ShaderFilesManager::readShaderFile(const std::string& file) const
{
    std::string shader_source;
    std::ifstream stream(file_manager->getShader(file), std::ios::in);
    if (stream.is_open())
    {
        std::string line = "";
        while (std::getline(stream, line))
            shader_source += "\n" + line;
        stream.close();
    }
    return shader_source;
}

// ----------------------------------------------------------------------------
/** Returns a string with the content of header.txt (which contains basic
 *  shader defines).
 */
const std::string& ShaderFilesManager::getHeader() const
{
    // Stores the content of header.txt, to avoid reading this file repeatedly.
    static std::string shader_header;

    // Only read file first time
    if (shader_header.empty())
        shader_header = readShaderFile("header.txt");

    return shader_header;
}   // getHeader

// ----------------------------------------------------------------------------
/** Returns a string with the content of generic_per_instance_vertex_data.vert
 */
const std::string& ShaderFilesManager::getGenericPerInstanceVertexData() const
{
    // Stores the content of generic_per_instance_vertex_data.vert, to avoid reading this file repeatedly.
    static std::string per_instance_data;

    // Only read file first time
    if (per_instance_data.empty())
        per_instance_data = readShaderFile("material_shaders/generic_per_instance_vertex_data.vert");

    return per_instance_data;    
}

// ------------------------------------------------------------------------
const std::string& ShaderFilesManager::getGenericSolidFirstPassVertexShader() const
{
    // Stores the content of generic_solid_first_pass.vert, to avoid reading this file repeatedly.
    static std::string per_instance_data;

    // Only read file first time
    if (per_instance_data.empty())
        per_instance_data = readShaderFile("material_shaders/generic_solid_first_pass.vert");

    return per_instance_data;    
}

// ----------------------------------------------------------------------------
/** Write GLSL preprocessor directives depending on hardware
 *  \param type Type of the shader.
 */
void ShaderFilesManager::writePreprocessorDirectives(unsigned type, std::ostringstream &code) const
{
#if !defined(USE_GLES2)
    code << "#version " << CVS->getGLSLVersion()<<"\n";
#else
    if (CVS->isGLSL())
        code << "#version 300 es\n";
#endif

#if !defined(USE_GLES2)
    // Some drivers report that the compute shaders extension is available,
    // but they report only OpenGL 3.x version, and thus these extensions
    // must be enabled manually. Otherwise the shaders compilation will fail
    // because STK tries to use extensions which are available, but disabled
    // by default.
    if (type == GL_COMPUTE_SHADER)
    {
        if (CVS->isARBComputeShaderUsable())
            code << "#extension GL_ARB_compute_shader : enable\n";
        if (CVS->isARBImageLoadStoreUsable())
            code << "#extension GL_ARB_shader_image_load_store : enable\n";
        if (CVS->isARBArraysOfArraysUsable())
            code << "#extension GL_ARB_arrays_of_arrays : enable\n";
    }
#endif

    if (CVS->isAMDVertexShaderLayerUsable())
        code << "#extension GL_AMD_vertex_shader_layer : enable\n";

    if (CVS->isARBExplicitAttribLocationUsable())
    {
        code << "#extension GL_ARB_explicit_attrib_location : enable\n";
        code << "#define Explicit_Attrib_Location_Usable\n";
    }

    if (CVS->isAZDOEnabled())
    {
        code << "#extension GL_ARB_bindless_texture : enable\n";
        code << "#define Use_Bindless_Texture\n";
    }

    if(CVS->supportsIndirectInstancingRendering())
        code << "#define Use_Instancing\n";

    if (!CVS->isARBUniformBufferObjectUsable())
        code << "#define UBO_DISABLED\n";
    if (CVS->isAMDVertexShaderLayerUsable())
        code << "#define VSLayer\n";
    if (CVS->needsRGBBindlessWorkaround())
        code << "#define SRGBBindlessFix\n";

#if !defined(USE_GLES2)
    // shader compilation fails with some drivers if there is no precision
    // qualifier
    if (type == GL_FRAGMENT_SHADER)
        code << "precision mediump float;\n";
#else
    int range[2], precision;
    glGetShaderPrecisionFormat(GL_FRAGMENT_SHADER, GL_HIGH_FLOAT, range,
        &precision);

    if (precision > 0)
        code << "precision highp float;\n";
    else
        code << "precision mediump float;\n";
#endif
    code << "#define MAX_BONES " << SharedGPUObjects::getMaxMat4Size() << "\n";    
} //writePreprocessorDirectives

// ----------------------------------------------------------------------------
/** Return a string with the declaration of an attribute
 *  \param attribute The name of the vertex attributes to be used in shader
 */
void ShaderFilesManager::writeAttributeDeclaration(const std::string &attribute, std::ostringstream &code) const
{
    auto it = m_attributes.find(attribute);
    if(it == m_attributes.end())
    {
        Log::error("ShaderFilesManager", "Unknow attribute '%s'", attribute.c_str());
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
std::string ShaderFilesManager::getShaderSourceFromFile(const std::string &file, unsigned type) const
{
    std::ostringstream code;
    
    writePreprocessorDirectives(type, code);
    code << getHeader();

    std::ifstream stream(file_manager->getShader(file), std::ios::in);
    if (stream.is_open())
    {
        const std::string stk_include = "#stk_include";
        std::string line;

        while (std::getline(stream, line))
        {
            const std::size_t pos = line.find(stk_include);

            // load the custom file pointed by the #stk_include directive
            if (pos != std::string::npos)
            {
                // find the start "
                std::size_t pos = line.find("\"");
                if (pos == std::string::npos)
                {
                    Log::error("ShaderFilesManager", "Invalid #stk_include"
                        " line: '%s'.", line.c_str());
                    continue;
                }

                std::string filename = line.substr(pos + 1);

                // find the end "
                pos = filename.find("\"");
                if (pos == std::string::npos)
                {
                    Log::error("ShaderFilesManager", "Invalid #stk_include"
                        " line: '%s'.", line.c_str());
                    continue;
                }

                filename = filename.substr(0, pos);

                // read the whole include file
                std::ifstream include_stream(file_manager->getShader(filename), std::ios::in);
                if (!include_stream.is_open())
                {
                    Log::error("ShaderFilesManager", "Couldn't open included"
                        " shader: '%s'.", filename.c_str());
                    continue;
                }

                std::string include_line = "";
                while (std::getline(include_stream, include_line))
                {
                    code << "\n" << include_line;
                }
                include_stream.close();
            }
            else
            {
                code << "\n" << line;
            }
        }

        stream.close();
    }
    else
    {
        Log::error("ShaderFilesManager", "Can not open '%s'.", file.c_str());
    }    
    
    return code.str();
} //getShaderSourceFromFile

// ----------------------------------------------------------------------------    
GLuint ShaderFilesManager::loadShaderFromSource(const std::string &source, unsigned type) const
{
    const GLuint id = glCreateShader(type);

    char const *source_pointer = source.c_str();
    int len                    = source.size();
    glShaderSource(id, 1, &source_pointer, &len);
    glCompileShader(id);

    GLint result = GL_FALSE;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        // failed to compile
        int info_length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &info_length);
        if (info_length < 0)
            info_length = 1024;
        char *error_message = new char[info_length];
        error_message[0] = 0;
        glGetShaderInfoLog(id, info_length, NULL, error_message);
        Log::error("ShaderFilesManager", error_message);
        delete[] error_message;
        return 0;
    }
    glGetError();

    return id;    
} //loadShaderFromSource

// ----------------------------------------------------------------------------
std::string ShaderFilesManager::genFirstPassVertexShaderSource(
    const XMLNode &flags_node,
    const XMLNode &vertex_shader_node)
{
    std::ostringstream code;

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
    
    code << getHeader();
    
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
    code << getGenericPerInstanceVertexData();
    code << "\n";
    code << getGenericSolidFirstPassVertexShader();
    
    return code.str();
}

// ----------------------------------------------------------------------------
ShaderFilesManager::ShaderFilesManager()
{
    m_attributes["Position"]       = std::make_pair<int, std::string>(0,"vec3");
    m_attributes["Normal"]         = std::make_pair<int, std::string>(1,"vec3");
    m_attributes["Color"]          = std::make_pair<int, std::string>(2,"vec4");
    m_attributes["Texcoord"]       = std::make_pair<int, std::string>(3,"vec2");
    m_attributes["SecondTexcoord"] = std::make_pair<int, std::string>(4,"vec2");
    m_attributes["Tangent"]        = std::make_pair<int, std::string>(5,"vec3");
    m_attributes["Bitangent"]      = std::make_pair<int, std::string>(6,"vec3");
}

// ----------------------------------------------------------------------------
/** Loads a single shader. This is NOT cached, use addShaderFile for that.
 *  \param file Filename of the shader to load.
 *  \param type Type of the shader.
 */
GLuint ShaderFilesManager::loadShader(const std::string &file, unsigned type)
{
    Log::info("ShaderFilesManager", "Compiling shader : %s", file.c_str());
    GLuint id = loadShaderFromSource(getShaderSourceFromFile(file, type), type);
    if(!id)
        Log::error("ShaderFilesManager", "Error in shader %s", file.c_str());
    return id;
} // loadShader

// ----------------------------------------------------------------------------
/** Loads a single shader file, and add it to the loaded (cached) list
 *  \param file Filename of the shader to load.
 *  \param type Type of the shader.
 */
GLuint ShaderFilesManager::addShaderFile(const std::string &file, unsigned type)
{
#ifdef DEBUG
    // Make sure no duplicated shader is added somewhere else
    std::unordered_map<std::string, GLuint>::const_iterator i =
        m_shader_files_loaded.find(file);
    assert(i == m_shader_files_loaded.end());
#endif

    const GLuint id = loadShader(file, type);
    m_shader_files_loaded[file] = id;
    return id;
}   // addShaderFile

// ----------------------------------------------------------------------------
/** Get a shader file. If the shader is not already in the cache it will be loaded and cached.
 *  \param file Filename of the shader to load.
 *  \param type Type of the shader.
 */
GLuint ShaderFilesManager::getShaderFile(const std::string &file, unsigned type)
{
    // found in cache
    auto it = m_shader_files_loaded.find(file);
    if (it != m_shader_files_loaded.end())
        return it->second;

   // add to the cache now
   return addShaderFile(file, type);
}   // getShaderFile

#endif   // !SERVER_ONLY
