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
#include "model/material.h"
#include <fstream>
#include <cstring>

GLenum TranslateFormat (const std::string &str);

Material::Material (void)
	: diffuse_enabled (false), normalmap_enabled (false),
		specularmap_enabled (false), transparent (false),
		heightmap_enabled (false), parametermap_enabled (false),
		displacementmap_enabled (false), doublesided (false)
{
}

Material::Material (Material &&material)
	: diffuse (std::move (material.diffuse)),
		diffuse_enabled (material.diffuse_enabled),
		normalmap (std::move (material.normalmap)),
		normalmap_enabled (material.normalmap_enabled),
		specularmap (std::move (material.specularmap)),
		specularmap_enabled (material.specularmap_enabled),
		parametermap (std::move (material.parametermap)),
		parametermap_enabled (material.parametermap_enabled),
		heightmap (std::move (material.heightmap)),
		heightmap_enabled (material.heightmap_enabled),
    displacementmap (std::move (material.displacementmap)),
    displacementmap_enabled (material.displacementmap_enabled),
		transparent (material.transparent),
		doublesided (material.doublesided)
{
	material.diffuse_enabled = false;
	material.normalmap_enabled = false;
	material.specularmap_enabled = false;
	material.parametermap_enabled = false;
	material.heightmap_enabled = false;
	material.displacementmap_enabled = false;
	material.transparent = false;
	material.doublesided = false;
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
	parametermap = std::move (material.parametermap);
	parametermap_enabled = material.parametermap_enabled;
	material.parametermap_enabled = false;
	heightmap = std::move (material.heightmap);
	heightmap_enabled = material.heightmap_enabled;
	material.heightmap_enabled = false;
	displacementmap = std::move (material.displacementmap);
	displacementmap_enabled = material.displacementmap_enabled;
	material.displacementmap_enabled = false;
	transparent = material.transparent;
	material.transparent = false;
	doublesided = material.doublesided;
	material.doublesided = false;
}

typedef struct {
	 uint8_t identifier[12];
	 uint32_t endianness;
	 uint32_t glType;
	 uint32_t glTypeSize;
	 uint32_t glFormat;
	 uint32_t glInternalFormat;
	 uint32_t glBaseInternalFormat;
	 uint32_t pixelWidth;
	 uint32_t pixelHeight;
	 uint32_t pixelDepth;
	 uint32_t numberOfArrayElements;
	 uint32_t numberOfFaces;
	 uint32_t numberOfMipmapLevels;
	 uint32_t bytesOfKeyValueData;
} ktx_header_t;

bool LoadTex (gl::Texture &texture, bool &result,
							const YAML::Node node)
{
	std::string filename;
	if (!node.IsScalar ())
	{
		result = false;
		return true;
	}

	result = true;

	filename = MakePath ("textures", node.as<std::string> ());

	ktx_header_t header;
	std::ifstream file (filename, std::ios_base::in|std::ios_base::binary);
	if (!file.is_open ())
		 return false;

	file.read (reinterpret_cast<char*> (&header), sizeof (ktx_header_t));

	const uint8_t id[12] = {
		0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A
	};
	if (file.gcount () != sizeof (ktx_header_t)
			|| memcmp (header.identifier, id, 12))
		 return false;
	if (header.endianness != 0x04030201)
		 return false;
	if (header.pixelDepth != 0)
		 return false;
	if (header.numberOfArrayElements)
		 return false;
	if (header.numberOfFaces != 1)
		 return false;
	if (header.numberOfMipmapLevels != 1)
		 return false;

	file.ignore (header.bytesOfKeyValueData);

	uint32_t size;
	file.read (reinterpret_cast<char*> (&size), sizeof (uint32_t));
	if (file.gcount () != sizeof (uint32_t))
		 return false;

	gl::Buffer data;
	data.Data (size, NULL, GL_STREAM_DRAW);

	void *ptr = data.Map (GL_WRITE_ONLY);
	file.read (reinterpret_cast<char*> (ptr), size);
	data.Unmap ();

	data.Bind (GL_PIXEL_UNPACK_BUFFER);
	if (header.glType == 0)
	{
		texture.CompressedImage2D (GL_TEXTURE_2D, 0, header.glInternalFormat,
															 header.pixelWidth, header.pixelHeight, 0,
															 size, NULL);
	}
	else
	{
		texture.Image2D (GL_TEXTURE_2D, 0, header.glInternalFormat,
										 header.pixelWidth, header.pixelHeight, 0,
										 header.glFormat, header.glType, NULL);
	}
	texture.GenerateMipmap (GL_TEXTURE_2D);
	gl::Buffer::Unbind (GL_PIXEL_UNPACK_BUFFER);

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

	transparent = desc["transparent"].as<bool> (false);
	doublesided = desc["doublesided"].as<bool> (false);

	if (!LoadTex (diffuse, diffuse_enabled,
								desc["textures"]["diffuse"]))
		 return false;
	if (!LoadTex (normalmap, normalmap_enabled,
								desc["textures"]["normalmap"]))
		 return false;
	if (!LoadTex (specularmap, specularmap_enabled,
								desc["textures"]["specularmap"]))
		 return false;
	if (!LoadTex (parametermap, parametermap_enabled,
								desc["textures"]["parametermap"]))
		 return false;
	if (!LoadTex (heightmap, heightmap_enabled,
								desc["textures"]["heightmap"]))
		 return false;
	if (!LoadTex (displacementmap, displacementmap_enabled,
								desc["textures"]["displacementmap"]))
		 return false;

	return true;
}

bool Material::IsTransparent (void) const
{
	return transparent;
}

bool Material::IsDoubleSided (void) const
{
	return doublesided;
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
	program["parametermap_enabled"] = parametermap_enabled;
	if (parametermap_enabled)
	{
		parametermap.Bind (GL_TEXTURE3, GL_TEXTURE_2D);
	}
	program["heightmap_enabled"] = heightmap_enabled;
	if (heightmap_enabled)
	{
		heightmap.Bind (GL_TEXTURE4, GL_TEXTURE_2D);
	}
	program["displacementmap_enabled"] = displacementmap_enabled;
	if (displacementmap_enabled)
	{
		displacementmap.Bind (GL_TEXTURE5, GL_TEXTURE_2D);
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
