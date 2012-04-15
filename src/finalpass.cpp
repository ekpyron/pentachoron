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
		"shadowmap.txt", "monochrome.txt", "depth.txt"
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
		program["nearClipPlane"] = renderer->camera.GetNearClipPlane ();
		program["farClipPlane"] = renderer->camera.GetFarClipPlane ();
	}
	gl::Viewport (0, 0, viewport.x, viewport.y);

	gl::ClearColor (0, 0, 0, 0);
	gl::Clear (GL_COLOR_BUFFER_BIT);

	switch (rendermode)
	{
	case 0:
		renderer->windowgrid.sampler.Bind (0);
		renderer->composition.screen.Bind (GL_TEXTURE0, GL_TEXTURE_RECTANGLE);
		renderer->windowgrid.sampler.Bind (1);
		renderer->composition.glow.Bind (GL_TEXTURE1, GL_TEXTURE_RECTANGLE);
		pipelines[0].Bind ();
		break;
	case 1:
		renderer->windowgrid.sampler.Bind (0);
		renderer->gbuffer.colorbuffer[0].Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		pipelines[2].Bind ();
		break;
	case 2:
		renderer->windowgrid.sampler.Bind (0);
		renderer->gbuffer.colorbuffer[1].Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		pipelines[2].Bind ();
		break;
	case 3:
		renderer->windowgrid.sampler.Bind (0);
		renderer->gbuffer.colorbuffer[2].Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		pipelines[2].Bind ();
		break;
	case 4:
		renderer->windowgrid.sampler.Bind (0);
		renderer->gbuffer.colorbuffer[3].Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		pipelines[2].Bind ();
		break;
	case 5:
		renderer->windowgrid.sampler.Bind (0);
		renderer->gbuffer.normalbuffer[0].Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		pipelines[1].Bind ();
		break;
	case 6:
		renderer->windowgrid.sampler.Bind (0);
		renderer->gbuffer.normalbuffer[1].Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		pipelines[1].Bind ();
		break;
	case 7:
		renderer->windowgrid.sampler.Bind (0);
		renderer->gbuffer.normalbuffer[2].Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		pipelines[1].Bind ();
		break;
	case 8:
		renderer->windowgrid.sampler.Bind (0);
		renderer->gbuffer.normalbuffer[3].Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		pipelines[1].Bind ();
		break;
	case 9:
		renderer->windowgrid.sampler.Bind (0);
		renderer->gbuffer.specularbuffer[0].Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		pipelines[2].Bind ();
		break;
	case 10:
		renderer->windowgrid.sampler.Bind (0);
		renderer->gbuffer.specularbuffer[1].Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		pipelines[2].Bind ();
		break;
	case 11:
		renderer->windowgrid.sampler.Bind (0);
		renderer->gbuffer.specularbuffer[2].Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		pipelines[2].Bind ();
		break;
	case 12:
		renderer->windowgrid.sampler.Bind (0);
		renderer->gbuffer.specularbuffer[3].Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		pipelines[2].Bind ();
		break;
	case 13:
		renderer->windowgrid.sampler.Bind (0);
		renderer->gbuffer.depthtexture[0].Bind (GL_TEXTURE0, GL_TEXTURE_RECTANGLE);
		pipelines[6].Bind ();
		break;
  case 14:
		renderer->windowgrid.sampler.Bind (0);
		renderer->gbuffer.depthtexture[1].Bind (GL_TEXTURE0, GL_TEXTURE_RECTANGLE);
		pipelines[6].Bind ();
		break;
  case 15:
		renderer->windowgrid.sampler.Bind (0);
		renderer->gbuffer.depthtexture[2].Bind (GL_TEXTURE0, GL_TEXTURE_RECTANGLE);
		pipelines[6].Bind ();
		break;
  case 16:
		renderer->windowgrid.sampler.Bind (0);
		renderer->gbuffer.depthtexture[3].Bind (GL_TEXTURE0, GL_TEXTURE_RECTANGLE);
		pipelines[6].Bind ();
		break;
	case 17:
		renderer->windowgrid.sampler.Bind (0);
		renderer->shadowmap.shadowmap.Bind (GL_TEXTURE0,
																				GL_TEXTURE_RECTANGLE);
		pipelines[4].Bind ();
		break;
	case 18:
		renderer->windowgrid.sampler.Bind (0);
		renderer->composition.glow.Bind (GL_TEXTURE0, GL_TEXTURE_RECTANGLE);
		pipelines[5].Bind ();
		break;
	default:
		throw std::runtime_error ("Invalid render mode.");
		break;
	}

	gl::Disable (GL_DEPTH_TEST);

	renderer->windowgrid.Render ();

	GL_CHECK_ERROR;
}
