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

#ifndef HEADER_SHADER_FILES_MANAGER_HPP
#define HEADER_SHADER_FILES_MANAGER_HPP

#include "graphics/gl_headers.hpp"
#include "utils/no_copy.hpp"
#include "utils/singleton.hpp"

#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

class XMLNode;

class ShaderFilesManager : public Singleton<ShaderFilesManager>, NoCopy
{
private:
    /**
     * Map from a vertex shader attribute to attribute location and GLSL type.
     * Used to generate material shaders.
     */
    static std::unordered_map<std::string, std::pair<int, std::string>> m_attributes;

    /**
     * Map from a filename to a shader indentifier. Used for caching shaders.
     */
    std::unordered_map<std::string, GLuint> m_shader_files_loaded;

    // ------------------------------------------------------------------------
    std::string readShaderFile(const std::string& file) const;
    // ------------------------------------------------------------------------
    const std::string& getHeader() const;
    // ------------------------------------------------------------------------
    const std::string& getGenericPerInstanceVertexData() const;
    // ------------------------------------------------------------------------
    const std::string& getGenericSolidFirstPassVertexShader() const;
    // ------------------------------------------------------------------------    
    void writePreprocessorDirectives(unsigned type, std::ostringstream &code) const;
    // ------------------------------------------------------------------------    
    void writeAttributeDeclaration(const std::string &attribute,
                                   std::ostringstream &code) const;
    // ------------------------------------------------------------------------    
    std::string getShaderSourceFromFile(const std::string &file, unsigned type) const;
    // ------------------------------------------------------------------------    
    GLuint loadShaderFromSource(const std::string &source, unsigned type) const;
    // ------------------------------------------------------------------------    
    std::string genFirstPassVertexShaderSource(const XMLNode &flags_node,
                                               const XMLNode &vertex_shader_node);

public:
    // ------------------------------------------------------------------------
    ShaderFilesManager();
    // ------------------------------------------------------------------------
    ~ShaderFilesManager()                                          { clean(); }
    // ------------------------------------------------------------------------
    void clean()                             { m_shader_files_loaded.clear(); }
    // ------------------------------------------------------------------------
    GLuint loadShader(const std::string &file, unsigned type);
    // ------------------------------------------------------------------------
    GLuint addShaderFile(const std::string &file, unsigned type);
    // ------------------------------------------------------------------------
    GLuint getShaderFile(const std::string &file, unsigned type);

};   // ShaderFilesManager

#endif

#endif   // !SERVER_ONLY
