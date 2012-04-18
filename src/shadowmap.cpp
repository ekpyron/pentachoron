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
#include "shadowmap.h"
#include "renderer.h"

ShadowMap::ShadowMap (Renderer *parent)
	: renderer (parent), soft_shadows (true)
{
}

ShadowMap::~ShadowMap (void)
{
}

bool ShadowMap::Init (void)
{
	{
		gl::Shader vshader (GL_VERTEX_SHADER),
			 fshader (GL_FRAGMENT_SHADER);
		std::string src;
		
		if (!ReadFile (MakePath ("shaders", "shadowmap", "vshader.txt"), src))
			 return false;
		vshader.Source (src);
		if (!vshader.Compile ())
		{
			(*logstream) << "Cannot compile "
									 << MakePath ("shaders", "shadowmap", "vshader.txt")
									 << ":" << std::endl << vshader.GetInfoLog () << std::endl;
			return false;
		}

		if (!ReadFile (MakePath ("shaders", "shadowmap", "fshader.txt"), src))
			 return false;
		fshader.Source (src);
		if (!fshader.Compile ())
		{
			(*logstream) << "Cannot compile "
									 << MakePath ("shaders", "shadowmap", "fshader.txt")
									 << ":" << std::endl << vshader.GetInfoLog () << std::endl;
			return false;
		}

		program.Attach (vshader);
		program.Attach (fshader);
		if (!program.Link ())
		{
			(*logstream) << "Cannot link the shadow map shader program:" << std::endl
									 << vshader.GetInfoLog () << std::endl;
			return false;
		}		
	}

	width = config["shadowmap"]["width"].as<GLuint> ();
	height = config["shadowmap"]["height"].as<GLuint> ();

	gl::Buffer::Unbind (GL_PIXEL_UNPACK_BUFFER);

	shadowmap.Image2D (GL_TEXTURE_2D, 0, GL_RG32F, width, height,
										 0, GL_RG, GL_UNSIGNED_BYTE, NULL);

	shadowmapmem = renderer->clctx.CreateFromGLTexture2D
		 (CL_MEM_READ_WRITE, GL_TEXTURE_RECTANGLE, 0, shadowmap);

	depthbuffer.Storage (GL_DEPTH_COMPONENT32, width, height);

	framebuffer.Texture2D (GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
												 shadowmap, 0);
	framebuffer.Renderbuffer (GL_DEPTH_ATTACHMENT, depthbuffer);
	framebuffer.DrawBuffers ({ GL_COLOR_ATTACHMENT0 });

	blur = renderer->filters.CreateBlur (shadowmapmem, width, height, 8);

	return true;
}

void ShadowMap::Render (GLuint shadowid, Geometry &geometry,
												const Shadow &shadow)
{
	vmat = glm::lookAt (glm::vec3 (shadow.position),
											glm::vec3 (shadow.position + shadow.direction),
											glm::vec3 (1, 0, 0));
	projmat = glm::perspective (2 * shadow.spot.angle * 180.0f / float (DRE_PI),
															(float) width / (float) height,
															3.0f, 500.0f);
	program["projmat"] = projmat;
	renderer->culling.SetProjMatrix (projmat);

	framebuffer.Bind (GL_FRAMEBUFFER);

	program.Use ();

	gl::DepthMask (GL_TRUE);

	gl::ClearBufferfv (GL_COLOR, 0, (float[]) {1.0f, 1.0f, 1.0f, 1.0f});
	gl::ClearBufferfv (GL_DEPTH, 0, (float[]) {1.0f});
	gl::Viewport (0, 0, width, height);

	gl::Enable (GL_DEPTH_TEST);

	GL_CHECK_ERROR;

	geometry.Render (Geometry::Pass::ShadowMap + shadowid * 0x00010000,
									 program, vmat, true, false);
	geometry.Render (Geometry::Pass::ShadowMap + shadowid * 0x00010000,
		program, vmat, true, true);

	gl::DepthMask (GL_FALSE);

	gl::Program::UseNone ();
	
	gl::Framebuffer::Unbind (GL_FRAMEBUFFER);

	if (soft_shadows)
		 blur.Apply ();

	GL_CHECK_ERROR;
}

GLuint ShadowMap::GetWidth (void) const
{
	return width;
}

GLuint ShadowMap::GetHeight (void) const
{
	return height;
}

bool ShadowMap::GetSoftShadows (void) const
{
	return soft_shadows;
}

void ShadowMap::SetSoftShadows (bool s)
{
	soft_shadows = s;
}
