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
#include <common.h>
#include <iostream>
#include <fstream>
#include <cstring>

const char *glErrorString (GLenum err)
{
	switch (err)
	{
	case GL_NO_ERROR:
		return "GL_NO_ERROR";
	case GL_INVALID_ENUM:
		return "GL_INVALID_ENUM";
	case GL_INVALID_VALUE:
		return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION:
		return "GL_INVALID_OPERATION";
	case GL_OUT_OF_MEMORY:
		return "GL_OUT_OF_MEMORY";
	default:
		return "UNKNOWN ERROR";
	}
}

void glCheckError (const std::string &file, unsigned int line)
{
	GLenum err;
	err = gl::GetError ();
	if (err != 0)
	{
		(*logstream) << "OpenGL Error detected in " << file
								 << " line " << std::dec << line << ": "
								 << glErrorString (err) << " (" << std::hex
								 << err << std::dec << ")" << std::endl;
	}
}

std::string MakePath (const std::string &subdir,
											const std::string &filename)
{
	std::string path;
	path = config["directories"]["base"].as<std::string> () + DIR_SEPARATOR
		 + config["directories"][subdir].as<std::string> ()
		 + DIR_SEPARATOR + filename;
	return path;
}

bool ReadFile (const std::string &filename, std::string &str)
{
	std::ifstream file (filename, std::ios::in);
	if (!file.is_open ())
	{
		(*logstream) << "Cannot open " << filename << std::endl;
		return false;
	}
	str.clear ();
	while (file.good ())
	{
		char buffer[256];
		file.read (buffer, 256);
		str.append (buffer, file.gcount ());
	}
	return true;
}

float ComputeWeight (unsigned long n, unsigned long k)
{
	long double tmp;
	/* scale down by the sum of all coefficients except the
	 * two smallest */
	tmp = 1.0 / (powl (2, n) - 2 * (1 + n));
	/* multiply by the binomial coefficient */
	for (int i = 1; i <= k; i++)
	{
		tmp *= (n - k + i);
		tmp /= i;
	}
	return tmp;
	
}

void ComputeWeightOffsets (std::vector<float> &data, GLuint size)
{
	std::vector<float> weights_data;
	std::vector<float> offsets_data;
	
	/* computes binomial coefficients */
	for (int i = 0; i < (size+1)/2; i++)
	{
		weights_data.push_back
			 (ComputeWeight (size + 3, ((size - 1) / 2) - i + 2));
	}

	/* push first weight and offset */
	data.push_back (weights_data[0]);
	data.push_back (0);

	/* compute weights and offsets for linear sampling */
	for (int i = 1; i <= weights_data.size() >> 1; i++)
	{
		float weight;
		/* compute and push combined weight */
		weight = weights_data[i * 2 - 1] +
			 weights_data[i * 2];
		data.push_back (weight);
		/* compute and push combined offset */
		data.push_back (((i * 2 - 1) /* discrete offset */
										 * weights_data[i * 2 - 1] /* discrete weight */
										 + i * 2 /* discrete offset */
										 * weights_data[i * 2]) /* discrete weight */
										/ weight); /* scale */
	}
	return;
}

bool LoadProgramBinary (gl::Program &program, const std::string &filename,
												uint64_t *hash)
{
	std::ifstream file (filename, std::ios_base::in|std::ios_base::binary);
	if (!file.is_open ())
		 return false;

	struct {
		 char magic[8];
		 uint64_t hash[3];
		 GLenum binaryFormat;
		 GLsizei length;
	} header ;

	const char magic[8] = { 'G', 'L', 'S', 'L', 'B', 'I', 'N', 0x00 };

	file.read (reinterpret_cast<char*> (&header), sizeof (header));
	if (file.gcount () != sizeof (header))
		 return false;

	if (memcmp (magic, header.magic, 8))
		 return false;

	if (hash[0] != header.hash[0]
			|| hash[1] != header.hash[1]
			|| hash[2] != header.hash[2])
		 return false;

	std::vector<uint8_t> binary;
	binary.resize (header.length);

	file.read (reinterpret_cast<char*> (&binary[0]), header.length);
	if (file.gcount () != header.length)
		 return false;

	return program.Binary (header.binaryFormat, &binary[0], header.length);
}

void SaveProgramBinary (gl::Program &program, const std::string &filename,
												uint64_t *hash)
{
	std::ofstream file (filename, std::ios_base::out|std::ios_base::binary
											|std::ios_base::trunc);
	if (!file.is_open ())
		 return;

	std::vector<uint8_t> binary;
	GLint len;
	GLsizei length;
	GLenum format;
	program.Get (GL_PROGRAM_BINARY_LENGTH, &len);
	binary.resize (len);
	program.GetBinary (len, &length, &format, &binary[0]);

	const char magic[8] = { 'G', 'L', 'S', 'L', 'B', 'I', 'N', 0x00 };
	file.write (magic, 8);
	file.write (reinterpret_cast<char*> (hash), sizeof (uint64_t) * 3);
	file.write (reinterpret_cast<char*> (&format), sizeof (format));
	file.write (reinterpret_cast<char*> (&length), sizeof (length));
	file.write (reinterpret_cast<char*> (&binary[0]), binary.size ());
}

bool LoadProgram (gl::Program &program, const std::string &filename,
									GLenum type, const std::string &definitions,
									const std::vector<std::string> &filenames)
{
	Tiger2 tiger2;
	std::vector<std::string> sources;
	if (!definitions.empty ())
	{
		 sources.push_back (definitions);
		 tiger2.consume (definitions.data (), definitions.length ());
	}
	for (const std::string &file : filenames)
	{
		sources.emplace_back ();
		if (!ReadFile (file, sources.back ()))
			 return false;
		tiger2.consume (sources.back ().data (), sources.back ().length ());
	}

	uint64_t hash[3];
	tiger2.finalize ();
	tiger2.get (hash);

	if (LoadProgramBinary (program, filename, hash))
	{
		return true;
	}

	program.Parameter (GL_PROGRAM_SEPARABLE, GL_TRUE);
	gl::Shader obj (type);
	obj.Source (sources);
	if (!obj.Compile ())
	{
		(*logstream) << "Could not compile " << filename << ": "
								 << obj.GetInfoLog () << std::endl;
		return false;
	}
	program.Attach (obj);

	if (!program.Link ())
	{
		(*logstream) << "Could not link " << filename << ": "
								 << program.GetInfoLog () << std::endl;
		return false;
	}

	SaveProgramBinary (program, filename, hash);

	return true;
}

bool LoadProgram (gl::Program &program, const std::string &filename,
									const std::vector<std::string> &definitions,
									const std::vector<std::pair<GLenum, std::string>>
									&filenames)
{
	Tiger2 tiger2;

	for (const std::string &def : definitions)
		 tiger2.consume (def.data (), def.length ());

	std::vector<std::string> sources;
	for (const std::pair<GLenum, std::string> &file : filenames)
	{
		sources.emplace_back ();
		if (!ReadFile (file.second, sources.back ()))
			 return false;
		tiger2.consume (sources.back ().data (), sources.back ().length ());
	}

	uint64_t hash[3];
	tiger2.finalize ();
	tiger2.get (hash);

	if (LoadProgramBinary (program, filename, hash))
	{
		return true;
	}

	for (auto i = 0; i < filenames.size (); i++)
	{
		gl::Shader obj (filenames[i].first);
		if (i < definitions.size ())
			 obj.Source (std::vector<std::string> ({ definitions[i], sources[i] }));
		else
			 obj.Source (std::vector<std::string> ({ sources[i] }));
		if (!obj.Compile ())
		{
			(*logstream) << "Could not compile " << filenames[i].second
									 << ": " << obj.GetInfoLog () << std::endl;
			return false;
		}
		program.Attach (obj);
	}

	program.Parameter (GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);

	if (!program.Link ())
	{
		(*logstream) << "Could not link " << filename << ": "
								 << program.GetInfoLog () << std::endl;
		return false;
	}

	SaveProgramBinary (program, filename, hash);

	return true;
}
