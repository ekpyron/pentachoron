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
#include "finalpass.h"
#include "gbuffer.h"
#include "renderer.h"

FinalPass::FinalPass (Renderer *parent)
	: renderer (parent), rendermode (0)
{
}

FinalPass::~FinalPass (void)
{
}

bool FinalPass::Init (void)
{
	std::vector<const char *> fprogram_sources = {
		"compose.txt", "normal.txt", "passthrough.txt", "diffuse.txt",
		"shadowmap.txt", "monochrome.txt"
	};
	for (const char *&filename : fprogram_sources)
	{
		gl::Shader obj (GL_FRAGMENT_SHADER);
		std::string source;
		if (!ReadFile (MakePath ("shaders", "finalpass", filename), source))
			 return false;
		obj.Source (source);
		if (!obj.Compile ())
		{
			(*logstream) << "Could not compile "
									 << MakePath ("shaders", "finalpass", filename)
									 << ": " << std::endl << obj.GetInfoLog () << std::endl;
			 return false;
		}

		fprograms.emplace_back ();
		fprograms.back ().Parameter (GL_PROGRAM_SEPARABLE, GL_TRUE);
		fprograms.back ().Attach (obj);
		if (!fprograms.back ().Link ())
		{
			(*logstream) << "Could not link the shader program "
									 << MakePath ("shaders", "finalpass", filename)
									 << ": " << std::endl << fprograms.back ().GetInfoLog ()
									 << std::endl;
			 return false;
		}
	}

	for (gl::Program &fprogram : fprograms)
	{
		pipelines.emplace_back ();
		pipelines.back ().UseProgramStages (GL_VERTEX_SHADER_BIT,
																				renderer->windowgrid.vprogram);
		pipelines.back ().UseProgramStages (GL_FRAGMENT_SHADER_BIT, fprogram);
	}

	return true;
}

void FinalPass::SetRenderMode (GLuint mode)
{
	rendermode = mode;
}

GLuint FinalPass::GetRenderMode (void)
{
	return rendermode;
}

void FinalPass::Render (void)
{
	glm::uvec2 viewport;
	viewport = renderer->camera.GetViewport ();
	for (gl::Program &program : fprograms)
	{
		program["viewport"] = viewport;
	}
	gl::Viewport (0, 0, viewport.x, viewport.y);

	gl::ClearColor (0, 0, 0, 0);
	gl::Clear (GL_COLOR_BUFFER_BIT);

	switch (rendermode)
	{
	case 0:
		renderer->windowgrid.sampler.Bind (0);
		renderer->composition.screen.Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		pipelines[2].Bind ();
		break;
	case 1:
		renderer->windowgrid.sampler.Bind (0);
		renderer->gbuffer.colorbuffer.Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		pipelines[2].Bind ();
		break;
	case 2:
		renderer->windowgrid.sampler.Bind (0);
		renderer->gbuffer.normalbuffer.Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		pipelines[1].Bind ();
		break;
	case 3:
		renderer->windowgrid.sampler.Bind (0);
		renderer->gbuffer.specularbuffer.Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		pipelines[2].Bind ();
		break;
	case 4:
		renderer->windowgrid.sampler.Bind (0);
		renderer->shadowpass.shadowmask.Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		pipelines[5].Bind ();
		break;
	case 5:
		renderer->windowgrid.sampler.Bind (0);
		renderer->shadowpass.shadowmap.shadowmap.Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		pipelines[4].Bind ();
		break;
	default:
		throw std::runtime_error ("Invalid render mode.");
		break;
	}

	gl::DepthMask (GL_FALSE);
	gl::Disable (GL_DEPTH_TEST);

	renderer->windowgrid.Render ();

	gl::DepthMask (GL_TRUE);

	GL_CHECK_ERROR;
}
