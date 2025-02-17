// Copyright NVIDIA Corporation 2007 -- Ignacio Castano <icastano@nvidia.com>
// 
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use,
// copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following
// conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.

#include <nvcore/StrLib.h>
#include <nvcore/StdStream.h>

#include <nvimage/Image.h>
#include <nvimage/nvtt/nvtt.h>

#include "cmdline.h"

#include <time.h> // clock

struct MyOutputHandler : public nvtt::OutputHandler
{
	MyOutputHandler() : total(0), progress(0), percentage(0), stream(NULL) {}
	MyOutputHandler(const char * name) : total(0), progress(0), percentage(0), stream(new nv::StdOutputStream(name)) {}
	virtual ~MyOutputHandler() { delete stream; }
	
	bool open(const char * name)
	{
		stream = new nv::StdOutputStream(name);
		percentage = progress = 0;
		if (stream->isError()) {
			printf("Error opening '%s' for writting\n", name);
			return false;
		}
		return true;
	}
	
	virtual void setTotal(int t)
	{
		total = t;
	}

	virtual void mipmap(int size, int width, int height, int depth, int face, int miplevel)
	{
		// ignore.
	}
	
	// Output data.
	virtual void writeData(const void * data, int size)
	{
		nvDebugCheck(stream != NULL);
		stream->serialize(const_cast<void *>(data), size);

		progress += size;
		int p = (100 * progress) / total;
		if (p != percentage)
		{
			percentage = p;
			printf("\r%d%%", percentage);
			fflush(stdout);
		}
	}
	
	int total;
	int progress;
	int percentage;
	nv::StdOutputStream * stream;
};

struct MyErrorHandler : public nvtt::ErrorHandler
{
	virtual void error(nvtt::Error e)
	{
		nvDebugBreak();
	}
};




// Set color to normal map conversion options.
void setColorToNormalMap(nvtt::InputOptions & inputOptions)
{
	inputOptions.setConvertToNormalMap(true);
	inputOptions.setHeightEvaluation(1.0f/3.0f, 1.0f/3.0f, 1.0f/3.0f, 0.0f);
	//inputOptions.setNormalFilter(1.0f, 0, 0, 0);
	//inputOptions.setNormalFilter(0.0f, 0, 0, 1);
	inputOptions.setGamma(1.0f, 1.0f);
	inputOptions.setNormalizeMipmaps(true);
}

// Set options for normal maps.
void setNormalMap(nvtt::InputOptions & inputOptions)
{
	inputOptions.setConvertToNormalMap(false);
	inputOptions.setGamma(1.0f, 1.0f);
	inputOptions.setNormalizeMipmaps(true);
}

// Set options for color maps.
void setColorMap(nvtt::InputOptions & inputOptions)
{
	inputOptions.setConvertToNormalMap(false);
	inputOptions.setGamma(2.2f, 2.2f);
	inputOptions.setNormalizeMipmaps(false);
}



int main(int argc, char *argv[])
{
	MyAssertHandler assertHandler;
	MyMessageHandler messageHandler;

	bool normal = false;
	bool color2normal = false;
	bool wrapRepeat = false;
	bool noMipmaps = false;
	bool fast = false;
	bool nocuda = false;
	nvtt::Format format = nvtt::Format_BC1;

	const char * externalCompressor = NULL;

	nv::Path input;
	nv::Path output;


	// Parse arguments.
	for (int i = 1; i < argc; i++)
	{
		// Input options.
		if (strcmp("-color", argv[i]) == 0)
		{
		}
		else if (strcmp("-normal", argv[i]) == 0)
		{
			normal = true;
		}
		else if (strcmp("-tonormal", argv[i]) == 0)
		{
			color2normal = true;
		}
		else if (strcmp("-clamp", argv[i]) == 0)
		{
		}
		else if (strcmp("-repeat", argv[i]) == 0)
		{
			wrapRepeat = true;
		}
		else if (strcmp("-nomips", argv[i]) == 0)
		{
			noMipmaps = true;
		}

		// Compression options.
		else if (strcmp("-fast", argv[i]) == 0)
		{
			fast = true;
		}
		else if (strcmp("-nocuda", argv[i]) == 0)
		{
			nocuda = true;
		}
		else if (strcmp("-rgb", argv[i]) == 0)
		{
			format = nvtt::Format_RGB;
		}
		else if (strcmp("-bc1", argv[i]) == 0)
		{
			format = nvtt::Format_BC1;
		}
		else if (strcmp("-bc2", argv[i]) == 0)
		{
			format = nvtt::Format_BC2;
		}
		else if (strcmp("-bc3", argv[i]) == 0)
		{
			format = nvtt::Format_BC3;
		}
		else if (strcmp("-bc3n", argv[i]) == 0)
		{
			format = nvtt::Format_BC3n;
		}
		else if (strcmp("-bc4", argv[i]) == 0)
		{
			format = nvtt::Format_BC4;
		}
		else if (strcmp("-bc5", argv[i]) == 0)
		{
			format = nvtt::Format_BC5;
		}

		// Undocumented option. Mainly used for testing.
		else if (strcmp("-ext", argv[i]) == 0)
		{
			if (i+1 < argc && argv[i+1][0] != '-') {
				externalCompressor = argv[i+1];
				printf("using %s\n", argv[i+1]);
				i++;
			}
		}

		else if (argv[i][0] != '-')
		{
			input = argv[i];

			if (i+1 < argc && argv[i+1][0] != '-') {
				output = argv[i+1];
			}
			else
			{
				output.copy(input.str());
				output.stripExtension();
				output.append(".dds");
			}

			break;
		}
	}

	if (input.empty())
	{
		printf("NVIDIA Texture Tools - Copyright NVIDIA Corporation 2007\n\n");
		
		printf("usage: nvcompress [options] infile [outfile]\n\n");
		
		printf("Input options:\n");
		printf("  -color   \tThe input image is a color map (default).\n");
		printf("  -normal  \tThe input image is a normal map.\n");
		printf("  -tonormal\tConvert input to normal map.\n");
		printf("  -clamp   \tClamp wrapping mode (default).\n");
		printf("  -repeat  \tRepeat wrapping mode.\n");
		printf("  -nomips  \tDisable mipmap generation.\n\n");

		printf("Compression options:\n");
		printf("  -fast    \tFast compression.\n");
		printf("  -nocuda  \tDo not use cuda compressor.\n");
		printf("  -rgb     \tRGBA format\n");
		printf("  -bc1     \tBC1 format (DXT1)\n");
		printf("  -bc2     \tBC2 format (DXT3)\n");
		printf("  -bc3     \tBC3 format (DXT5)\n");
		printf("  -bc3n    \tBC3 normal map format (DXT5n/RXGB)\n");
		printf("  -bc4     \tBC4 format (ATI1)\n");
		printf("  -bc5     \tBC5 format (3Dc/ATI2)\n\n");
		
		return 1;
	}

	nv::Image image;
	if (!image.load(input))
	{
		printf("The file '%s' is not a supported image type.\n", input.str());
		return 1;
	}


	MyErrorHandler errorHandler;
	MyOutputHandler outputHandler(output);
	if (outputHandler.stream->isError())
	{
		printf("Error opening '%s' for writting\n", output.str());
		return 1;
	}

	// Set input options.
	nvtt::InputOptions inputOptions;
	inputOptions.setTextureLayout(nvtt::TextureType_2D, image.width(), image.height());
	inputOptions.setMipmapData(image.pixels(), image.width(), image.height());

	if (fast)
	{
		inputOptions.setMipmapping(true, nvtt::MipmapFilter_Box);
	}
	else
	{
		inputOptions.setMipmapping(true, nvtt::MipmapFilter_Kaiser);
	}

	if (wrapRepeat)
	{
		inputOptions.setWrapMode(nvtt::WrapMode_Repeat);
	}
	else
	{
		inputOptions.setWrapMode(nvtt::WrapMode_Clamp);
	}

	if (normal)
	{
		setNormalMap(inputOptions);
	}
	else if (color2normal)
	{
		setColorToNormalMap(inputOptions);
	}
	else
	{
		setColorMap(inputOptions);
	}
	
	if (noMipmaps)
	{
		inputOptions.setMipmapping(false);
	}
	

	nvtt::CompressionOptions compressionOptions;
	compressionOptions.setFormat(format);
	if (fast)
	{
		compressionOptions.setQuality(nvtt::Quality_Fastest);
	}
	else
	{
		compressionOptions.setQuality(nvtt::Quality_Normal);
		//compressionOptions.setQuality(nvtt::Quality_Production, 0.5f);
		//compressionOptions.setQuality(nvtt::Quality_Highest);
	}
	compressionOptions.enableHardwareCompression(!nocuda);
	compressionOptions.setColorWeights(1, 1, 1);

	if (externalCompressor != NULL)
	{
		compressionOptions.setExternalCompressor(externalCompressor);
	}

	outputHandler.setTotal(nvtt::estimateSize(inputOptions, compressionOptions));

	nvtt::OutputOptions outputOptions(&outputHandler, &errorHandler);
	
	clock_t start = clock();

	nvtt::compress(inputOptions, outputOptions, compressionOptions);

	clock_t end = clock();
	printf("\rtime taken: %.3f seconds\n", float(end-start) / CLOCKS_PER_SEC);
	
	return 0;
}

