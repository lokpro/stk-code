//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 SuperTuxKart-Team
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

#include "graphics/texture_manager.hpp"

#include "graphics/central_settings.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/materials.hpp"
#include "graphics/stk_texture.hpp"
#include "utils/string_utils.hpp"

#if defined(USE_GLES2)
#define _IRR_COMPILE_WITH_OGLES2_
#include "../../lib/irrlicht/source/Irrlicht/COGLES2Texture.h"
#else
#include "../../lib/irrlicht/source/Irrlicht/COpenGLTexture.h"
#endif


#include <fstream>
#include <sstream>


GLuint getTextureGLuint(irr::video::ITexture *tex)
{
    if (tex == NULL)
        return 0;
    return tex->getOpenGLTextureName();
}


static std::set<irr::video::ITexture *> AlreadyTransformedTexture;
static std::map<int, video::ITexture*> unicolor_cache;
static std::vector<uint64_t> texture_handles;

void resetTextureTable()
{
#if !defined(USE_GLES2)
    if (CVS->isAZDOEnabled())
    {
        // Driver seems to crash if texture handles are not cleared...
        for (uint64_t& handle : texture_handles)
        {
            glMakeTextureHandleNonResidentARB(handle);
        }
        ObjectPass1Shader::getInstance()->recreateTrilinearSampler(0);
        texture_handles.clear();
    }
#endif
    AlreadyTransformedTexture.clear();
}

void insertTextureHandle(uint64_t handle)
{
    texture_handles.push_back(handle);
}

void cleanUnicolorTextures()
{
    for (std::pair<const int, video::ITexture*>& uc : unicolor_cache)
    {
        uc.second->drop();
    }
    unicolor_cache.clear();
}

void compressTexture(irr::video::ITexture *tex, bool srgb, bool premul_alpha)
{
    STKTexture* stk_tex = dynamic_cast<STKTexture*>(tex);
    if (stk_tex) return;

    if (AlreadyTransformedTexture.find(tex) != AlreadyTransformedTexture.end())
        return;
    AlreadyTransformedTexture.insert(tex);

    glBindTexture(GL_TEXTURE_2D, getTextureGLuint(tex));

    std::string cached_file;
    if (CVS->isTextureCompressionEnabled())
    {
        // Try to retrieve the compressed texture in cache
        std::string tex_name = irr_driver->getTextureName(tex);
        if (!tex_name.empty()) {
            cached_file = file_manager->getTextureCacheLocation(tex_name) + ".gltz";
            if (!file_manager->fileIsNewer(tex_name, cached_file)) {
                if (loadCompressedTexture(cached_file))
                    return;
            }
        }
    }

    size_t w = tex->getSize().Width, h = tex->getSize().Height;
    unsigned char *data = new unsigned char[w * h * 4];
    memcpy(data, tex->lock(), w * h * 4);
    tex->unlock();
    unsigned internalFormat, Format;
    Format = tex->hasAlpha() ? GL_BGRA : GL_BGR;
#if defined(USE_GLES2)
    if (!CVS->isEXTTextureFormatBGRA8888Usable())
    {
        Format = tex->hasAlpha() ? GL_RGBA : GL_RGB;
        
        for (unsigned int i = 0; i < w * h; i++)
        {
            char tmp_val = data[i*4];
            data[i*4] = data[i*4 + 2];
            data[i*4 + 2] = tmp_val;
        }
    }
#endif

    if (premul_alpha)
    {
        for (unsigned i = 0; i < w * h; i++)
        {
            float alpha = data[4 * i + 3];
            if (alpha > 0.)
                alpha = pow(alpha / 255.f, 1.f / 2.2f);
            data[4 * i] = (unsigned char)(data[4 * i] * alpha);
            data[4 * i + 1] = (unsigned char)(data[4 * i + 1] * alpha);
            data[4 * i + 2] = (unsigned char)(data[4 * i + 2] * alpha);
        }
    }

#if !defined(USE_GLES2)
    if (!CVS->isTextureCompressionEnabled())
    {
        if (srgb)
            internalFormat = (tex->hasAlpha()) ? GL_SRGB_ALPHA : GL_SRGB;
        else
            internalFormat = (tex->hasAlpha()) ? GL_RGBA : GL_RGB;
    }
    else
    {
        if (srgb)
            internalFormat = (tex->hasAlpha()) ? GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT : GL_COMPRESSED_SRGB_S3TC_DXT1_EXT;
        else
            internalFormat = (tex->hasAlpha()) ? GL_COMPRESSED_RGBA_S3TC_DXT5_EXT : GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
    }
#else
    internalFormat = (tex->hasAlpha()) ? GL_RGBA : GL_RGB;
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, Format, GL_UNSIGNED_BYTE, (GLvoid *)data);
    glGenerateMipmap(GL_TEXTURE_2D);
    delete[] data;

    if (CVS->isTextureCompressionEnabled() && !cached_file.empty())
    {
        // Save the compressed texture in the cache for later use.
        saveCompressedTexture(cached_file);
    }
}

//-----------------------------------------------------------------------------
/** Try to load a compressed texture from the given file name.
*   Data in the specified file need to have a specific format. See the
*   saveCompressedTexture() function for a description of the format.
*   \return true if the loading succeeded, false otherwise.
*   \see saveCompressedTexture
*/
bool loadCompressedTexture(const std::string& compressed_tex)
{
    std::ifstream ifs(compressed_tex.c_str(), std::ios::in | std::ios::binary);
    if (!ifs.is_open())
        return false;

    int internal_format;
    int w, h;
    int size = -1;
    ifs.read((char*)&internal_format, sizeof(int));
    ifs.read((char*)&w, sizeof(int));
    ifs.read((char*)&h, sizeof(int));
    ifs.read((char*)&size, sizeof(int));

    if (ifs.fail() || size == -1)
        return false;

    char *data = new char[size];
    ifs.read(data, size);
    if (!ifs.fail())
    {
        glCompressedTexImage2D(GL_TEXTURE_2D, 0, internal_format,
            w, h, 0, size, (GLvoid*)data);
        glGenerateMipmap(GL_TEXTURE_2D);
        delete[] data;
        ifs.close();
        return true;
    }
    delete[] data;
    return false;
}

//-----------------------------------------------------------------------------
/** Try to save the last texture sent to glTexImage2D in a file of the given
*   file name. This function should only be used for textures sent to
*   glTexImage2D with a compressed internal format as argument.<br>
*   \note The following format is used to save the compressed texture:<br>
*         <internal-format><width><height><size><data> <br>
*         The first four elements are integers and the last one is stored
*         on \c size bytes.
*   \see loadCompressedTexture
*/
void saveCompressedTexture(const std::string& compressed_tex)
{
#if !defined(USE_GLES2)
    int internal_format, width, height, size, compressionSuccessful;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, (GLint *)&internal_format);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, (GLint *)&width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, (GLint *)&height);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED, (GLint *)&compressionSuccessful);
    if (!compressionSuccessful)
        return;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, (GLint *)&size);

    char *data = new char[size];
    glGetCompressedTexImage(GL_TEXTURE_2D, 0, (GLvoid*)data);
    std::ofstream ofs(compressed_tex.c_str(), std::ios::out | std::ios::binary);
    if (ofs.is_open())
    {
        ofs.write((char*)&internal_format, sizeof(int));
        ofs.write((char*)&width, sizeof(int));
        ofs.write((char*)&height, sizeof(int));
        ofs.write((char*)&size, sizeof(int));
        ofs.write(data, size);
        ofs.close();
    }
    delete[] data;
#endif
}

video::ITexture* getUnicolorTexture(const video::SColor &c)
{
    std::map<int, video::ITexture*>::iterator it = unicolor_cache.find(c.color);
    if (it != unicolor_cache.end())
    {
        return it->second;
    }
    else
    {
        unsigned tmp[4] = {
            c.color,
            c.color,
            c.color,
            c.color
        };
        video::IImage *img = irr_driver->getVideoDriver()->createImageFromData(video::ECF_A8R8G8B8, core::dimension2d<u32>(2, 2), tmp);
        std::stringstream name;
        name << "color" << c.color;
        video::ITexture* tex = irr_driver->getVideoDriver()->addTexture(name.str().c_str(), img);
        tex->grab();
        // Only let our map hold the unicolor texture
        irr_driver->getVideoDriver()->removeTexture(tex);
        unicolor_cache[c.color] = tex;
        img->drop();
        return tex;
    }
}

core::stringw reloadTexture(const core::stringw& name)
{
    if (!CVS->isGLSL())
        return L"Use shader based renderer to reload textures.";
    if (CVS->isTextureCompressionEnabled())
        return L"Please disable texture compression for reloading textures.";

    if (name.empty())
    {
        for (video::ITexture* tex : AlreadyTransformedTexture)
            reloadSingleTexture(tex);
        return L"All textures reloaded.";
    }

    core::stringw result;
    core::stringw list = name;
    list.make_lower().replace(L'\u005C', L'\u002F');
    std::vector<std::string> names =
        StringUtils::split(StringUtils::wideToUtf8(list), ';');
    for (const std::string& fname : names)
    {
        for (video::ITexture* tex : AlreadyTransformedTexture)
        {
            std::string tex_path =
                StringUtils::toLowerCase(tex->getName().getPtr());
            std::string tex_name = StringUtils::getBasename(tex_path);
            if (fname == tex_name || fname == tex_path)
            {
                if (reloadSingleTexture(tex))
                {
                    result += tex_name.c_str();
                    result += L" ";
                    break;
                }
            }
        }
    }
    if (result.empty())
        return L"Texture(s) not found!";
    return result + "reloaded.";
}

bool reloadSingleTexture(video::ITexture* tex)
{
    video::IImage* tmp =
        irr_driver->getVideoDriver()->createImageFromFile(tex->getName());
    if (tmp == NULL) return false;

    const bool scaling = tmp->getDimension() != tex->getSize();
    video::IImage* new_texture =
        irr_driver->getVideoDriver()->createImage(video::ECF_A8R8G8B8,
        scaling ? tex->getSize() : tmp->getDimension());
    if (scaling)
        tmp->copyToScaling(new_texture);
    else
        tmp->copyTo(new_texture);
    tmp->drop();

    glBindTexture(GL_TEXTURE_2D, getTextureGLuint(tex));
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, new_texture->getDimension().Width,
        new_texture->getDimension().Height, GL_BGRA, GL_UNSIGNED_BYTE,
        new_texture->lock());
    new_texture->unlock();
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
#if defined(USE_GLES2)
    static_cast<irr::video::COGLES2Texture*>(tex)->setImage(new_texture);
#else
    static_cast<irr::video::COpenGLTexture*>(tex)->setImage(new_texture);
#endif
    Log::info("TextureManager", "%s reloaded", tex->getName().getPtr());
    return true;
}

#endif   // !SERVER_ONLY

