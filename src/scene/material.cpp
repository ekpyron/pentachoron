/*
 * This file is part of DRE.
 *
 * DRE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * DRE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with DRE.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "scene/material.h"
#include <assimp.hpp>
#include <aiScene.h>
#include <oglimg/oglimg.h>
#include <fstream>

GLenum TranslateFormat (const std::string &str);

Material::Material (void)
	: diffuse_enabled (false), normalmap_enabled (false),
		specularmap_enabled (false), opaque (false)
{
}

Material::Material (Material &&material)
	: diffuse (std::move (material.diffuse)),
		diffuse_enabled (material.diffuse_enabled),
		normalmap (std::move (material.normalmap)),
		normalmap_enabled (material.normalmap_enabled),
		specularmap (std::move (material.specularmap)),
		specularmap_enabled (material.specularmap_enabled),
		opaque (material.opaque)
{
	material.diffuse_enabled = false;
	material.normalmap_enabled = false;
	material.specularmap_enabled = false;
	material.opaque = false;
}

Material::~Material (void)
{
}

Material &Material::operator= (Material &&material)
{
	diffuse = std::move (material.diffuse);
	diffuse_enabled = material.diffuse_enabled;
	material.diffuse_enabled = false;
	normalmap = std::move (material.normalmap);
	normalmap_enabled = material.normalmap_enabled;
	material.normalmap_enabled = false;
	specularmap = std::move (material.specularmap);
	specularmap_enabled = material.specularmap_enabled;
	material.specularmap_enabled = false;
	opaque = material.opaque;
	material.opaque = false;
}

bool LoadTex (gl::Texture &texture, bool &result,
							const YAML::Node node, GLenum default_format)
{
	gl::Image image;
	std::string filename;
	GLenum format;
	if (node.IsSequence ())
	{
		filename = MakePath ("textures", node[0].as<std::string> ());
		format = TranslateFormat (node[1].as<std::string> ("AUTO"));
		if (!format)
			 format = default_format;
	}
	else if (node.IsScalar ())
	{
		filename = MakePath ("textures", node[0].as<std::string> ());
		format = default_format;
	}
	else
	{
		result = false;
		return true;
	}

	if (!image.Load (filename))
	{
		(*logstream) << "Could not load the texture " << filename
								 << "." << std::endl;
		return false;
	}
	image.GetBuffer ().Bind (GL_PIXEL_UNPACK_BUFFER);
	texture.Image2D (GL_TEXTURE_2D, 0, format, image.GetWidth (),
									 image.GetHeight (), 0, image.GetFormat (),
									 image.GetType (), NULL);
	texture.GenerateMipmap (GL_TEXTURE_2D);
	result = true;
	return true;
}

bool Material::Load (const std::string &name)
{
	YAML::Node desc;
	std::string filename = name + ".yaml";
	std::ifstream file (MakePath ("materials", filename), std::ifstream::in);
	if (!file.is_open ())
	{
		(*logstream) << "Cannot open material file " << filename << std::endl;
		return false;
	}
	desc = YAML::Load (file);
	if (!desc.IsMap ())
	{
		(*logstream) << "The material file " << filename
								 << " has an invalid format." << std::endl;
		return false;
	}

	if (desc["opaque"])
		 opaque = desc["opaque"].as<bool> ();

	if (!LoadTex (diffuse, diffuse_enabled,
								desc["textures"]["diffuse"],
								opaque ? GL_COMPRESSED_RGBA_BPTC_UNORM_ARB
								: GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB))
		 return false;
	if (!LoadTex (normalmap, normalmap_enabled,
								desc["textures"]["normalmap"],
								GL_COMPRESSED_RG_RGTC2))
		 return false;
	if (!LoadTex (specularmap, specularmap_enabled,
								desc["textures"]["specularmap"],
								GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB))
		 return false;

	return true;
}

bool Material::IsOpaque (void) const
{
	return opaque;
}

void Material::Use (const gl::Program &program) const
{
	program["diffuse_enabled"] = diffuse_enabled;
	if (diffuse_enabled)
	{
		diffuse.Bind (GL_TEXTURE0, GL_TEXTURE_2D);
	}
	program["normalmap_enabled"] = normalmap_enabled;
	if (normalmap_enabled)
	{
		normalmap.Bind (GL_TEXTURE1, GL_TEXTURE_2D);
	}
	program["specularmap_enabled"] = specularmap_enabled;
	if (specularmap_enabled)
	{
		specularmap.Bind (GL_TEXTURE2, GL_TEXTURE_2D);
	}
}

GLenum TranslateFormat (const std::string &str)
{
#define F(x) { #x, x }
	struct {
		 const char *name;
		 GLenum value;
	} formats[] = {
// TODO: Check this list for completeness
//       and for double entries
		{ "AUTO", 0 },
		F (GL_COMPRESSED_RED),
		F (GL_COMPRESSED_RG),
		F (GL_COMPRESSED_RGB),
		F (GL_COMPRESSED_RGBA),
		F (GL_COMPRESSED_RED_RGTC1),
		F (GL_COMPRESSED_SIGNED_RED_RGTC1),
		F (GL_COMPRESSED_RG_RGTC2),
		F (GL_COMPRESSED_SIGNED_RG_RGTC2),
/*		F (GL_COMPRESSED_RGB_S3TC_DXT1_EXT),
		F (GL_COMPRESSED_RGBA_S3TC_DXT1_EXT),
		F (GL_COMPRESSED_RGBA_S3TC_DXT3_EXT),
		F (GL_COMPRESSED_RGBA_S3TC_DXT5_EXT),
		F (GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT),
		F (GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT),
		F (GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT),*/
		F (GL_COMPRESSED_RGBA_BPTC_UNORM_ARB),
		F (GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB),
		F (GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB),
		F (GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB),
		F (GL_R8_SNORM),
		F (GL_R16_SNORM),
		F (GL_RG8_SNORM),
		F (GL_R3_G3_B2),
		F (GL_RGB4),
		F (GL_RGB5),
		F (GL_RGB8),
		F (GL_RGB10),
		F (GL_RGB12),
		F (GL_RGB16),
		F (GL_RGBA2),
		F (GL_RGBA4),
		F (GL_RGB5_A1),
		F (GL_RGBA8),
		F (GL_RGB10_A2),
		F (GL_RGBA12),
		F (GL_RGBA16),
		F (GL_R8),
		F (GL_R16),
		F (GL_RG8),
		F (GL_RG16),
		F (GL_R16F),
		F (GL_R32F),
		F (GL_RG16F),
		F (GL_RG32F),
		F (GL_R8I),
		F (GL_R8UI),
		F (GL_R16I),
		F (GL_R16UI),
		F (GL_R32I),
		F (GL_R32UI),
		F (GL_RG8I),
		F (GL_RG8UI),
		F (GL_RG16I),
		F (GL_RG16UI),
		F (GL_RG32I),
		F (GL_RG32UI),
		F (GL_R8_SNORM),
		F (GL_RG8_SNORM),
		F (GL_RGB8_SNORM),
		F (GL_RGBA8_SNORM),
		F (GL_R16_SNORM),
		F (GL_RG16_SNORM),
		F (GL_RGB16_SNORM),
		F (GL_RGBA16_SNORM),
		F (GL_SRGB8),
		F (GL_SRGB8_ALPHA8),
		F (GL_R11F_G11F_B10F),
		F (GL_RGB9_E5)
	};
#undef F

	for (auto i = 0; i < sizeof (formats) / sizeof (formats[0]); i++)
	{
		if (!str.compare (formats[i].name))
			 return formats[i].value;
	}

	return 0;
}
