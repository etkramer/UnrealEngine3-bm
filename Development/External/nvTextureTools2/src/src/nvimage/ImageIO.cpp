// This code is in the public domain -- castanyo@yahoo.es

#include <nvcore/Ptr.h>
#include <nvcore/Containers.h>
#include <nvcore/StrLib.h>
#include <nvcore/StdStream.h>

#include <nvmath/Color.h>

#include "ImageIO.h"
#include "Image.h"
#include "FloatImage.h"
#include "TgaFile.h"

// Extern
#if defined(HAVE_JPEG)
extern "C" {
#	include <jpeglib.h>
}
#endif

#if defined(HAVE_PNG)
#	include <png.h>
#endif

#if defined(HAVE_TIFF)
#	define _TIFF_DATA_TYPEDEFS_
#	include <tiffio.h>
#endif

using namespace nv;

namespace {

	// Array of image load plugins.
//	static HashMap<String, ImageInput_Plugin> s_plugin_load_map;

	// Array of image save plugins.
//	static HashMap<String, ImageOutput_Plugin> s_plugin_save_map;
	
	struct Color555 {
		uint16 b : 5;
		uint16 g : 5;
		uint16 r : 5;
	};
	
} // namespace


Image * nv::ImageIO::load(const char * name)
{
	StdInputStream stream(name);
	
	if (stream.isError()) {
		return false;
	}
	
	return load(name, stream);
}

Image * nv::ImageIO::load(const char * name, Stream & s)
{
	const char * extension = Path::extension(name);
	
	if (strCaseCmp(extension, ".tga") == 0) {
		return loadTGA(s);
	}
#if defined(HAVE_JPEG)
	if (strCaseCmp(extension, ".jpg") == 0 || strCaseCmp(extension, ".jpeg") == 0) {
		return loadJPG(s);
	}
#endif
#if defined(HAVE_PNG)
	if (strCaseCmp(extension, ".png") == 0) {
		return loadPNG(s);
	}
#endif
	// @@ use image plugins?

	return NULL;
}


/// Load TGA image.
Image * nv::ImageIO::loadTGA(Stream & s)
{
	nvCheck(!s.isError());
	
	TgaHeader tga;
	s << tga;
	s.seek(TgaHeader::Size + tga.id_length);

	// Get header info.
	bool rle = false;
	bool pal = false;
	bool rgb = false;
	bool grey = false;

	switch( tga.image_type ) {
		case TGA_TYPE_RLE_INDEXED:
			rle = true;
			// no break is intended!
		case TGA_TYPE_INDEXED:
			if( tga.colormap_type!=1 || tga.colormap_size!=24 || tga.colormap_length>256 ) {
				nvDebug( "*** ImageIO::loadTGA: Error, only 24bit paletted images are supported.\n" );
				return false;
			}
			pal = true;
			break;

		case TGA_TYPE_RLE_RGB:
			rle = true;
			// no break is intended!
		case TGA_TYPE_RGB:
			rgb = true;
			break;

		case TGA_TYPE_RLE_GREY:
			rle = true;
			// no break is intended!
		case TGA_TYPE_GREY:
			grey = true;
			break;

		default:
			nvDebug( "*** ImageIO::loadTGA: Error, unsupported image type.\n" );
			return false;
	}

	const uint pixel_size = (tga.pixel_size/8);
	nvDebugCheck(pixel_size <= 4);
	
	const uint size = tga.width * tga.height * pixel_size;

	
	// Read palette
	uint8 palette[768];
	if( pal ) {
		nvDebugCheck(tga.colormap_length < 256);
		s.serialize(palette, 3 * tga.colormap_length);
	}

	// Decode image.
	uint8 * mem = new uint8[size];
	if( rle ) {
		// Decompress image in src.
		uint8 * dst = mem;
		int num = size;

		while (num > 0) {
			// Get packet header
			uint8 c; 
			s << c;

			uint count = (c & 0x7f) + 1;
			num -= count * pixel_size;

			if (c & 0x80) {
				// RLE pixels.
				uint8 pixel[4];	// uint8 pixel[pixel_size];
				s.serialize( pixel, pixel_size );
				do {
					memcpy(dst, pixel, pixel_size);
					dst += pixel_size;
				} while (--count);
			}
			else {
				// Raw pixels.
				count *= pixel_size;
				//file->Read8(dst, count);
				s.serialize(dst, count);
				dst += count;
			}
		}
	}
	else {
		s.serialize(mem, size);
	}

	// Allocate image.
	AutoPtr<Image> img(new Image());
	img->allocate(tga.width, tga.height);

	int lstep;
	Color32 * dst;
	if( tga.flags & TGA_ORIGIN_UPPER ) {
		lstep = tga.width;
		dst = img->pixels();
	}
	else {
		lstep = - tga.width;
		dst = img->pixels() + (tga.height-1) * tga.width;
	}

	// Write image.
	uint8 * src = mem;
	if( pal ) {
		for( int y = 0; y < tga.height; y++ ) {
			for( int x = 0; x < tga.width; x++ ) {
				uint8 idx = *src++;
				dst[x].setBGRA(palette[3*idx+0], palette[3*idx+1], palette[3*idx+2], 0xFF);
			}
			dst += lstep;
		}
	}
	else if( grey ) {
		img->setFormat(Image::Format_ARGB);
		
		for( int y = 0; y < tga.height; y++ ) {
			for( int x = 0; x < tga.width; x++ ) {
				dst[x].setBGRA(*src, *src, *src, *src);
				src++;
			}
			dst += lstep;
		}
	}
	else {
		
		if( tga.pixel_size == 16 ) {
			for( int y = 0; y < tga.height; y++ ) {
				for( int x = 0; x < tga.width; x++ ) {
					Color555 c = *reinterpret_cast<Color555 *>(src);
					uint8 b = (c.b << 3) | (c.b >> 2);					
					uint8 g = (c.g << 3) | (c.g >> 2);
					uint8 r = (c.r << 3) | (c.r >> 2);
					dst[x].setBGRA(b, g, r, 0xFF);
					src += 2;
				}
				dst += lstep;
			}
		}
		else if( tga.pixel_size == 24 ) {
			for( int y = 0; y < tga.height; y++ ) {
				for( int x = 0; x < tga.width; x++ ) {
					dst[x].setBGRA(src[0], src[1], src[2], 0xFF);
					src += 3;
				}
				dst += lstep;
			}
		}
		else if( tga.pixel_size == 32 ) {
			img->setFormat(Image::Format_ARGB);
			
			for( int y = 0; y < tga.height; y++ ) {
				for( int x = 0; x < tga.width; x++ ) {
					dst[x].setBGRA(src[0], src[1], src[2], src[3]);
					src += 4;
				}
				dst += lstep;
			}
		}
	}

	// free uncompressed data.
	delete [] mem;

	return img.release();
}

/// Save TGA image.
bool nv::ImageIO::saveTGA(Stream & s, const Image * img)
{
	nvCheck(!s.isError());
	nvCheck(img != NULL);
	nvCheck(img->pixels() != NULL);
	
	TgaFile tga;
	tga.head.id_length = 0;
	tga.head.colormap_type = 0;
	tga.head.image_type = TGA_TYPE_RGB;

	tga.head.colormap_index = 0;
	tga.head.colormap_length = 0;
	tga.head.colormap_size = 0;

	tga.head.x_origin = 0;
	tga.head.y_origin = 0;
	tga.head.width = img->width();
	tga.head.height = img->height();
	if(img->format() == Image::Format_ARGB) {
		tga.head.pixel_size = 32;
		tga.head.flags = TGA_ORIGIN_UPPER;
	}
	else {
		tga.head.pixel_size = 24;
		tga.head.flags = TGA_ORIGIN_UPPER;
	}

	// @@ Serialize directly.
	tga.allocate();

	const uint n = img->width() * img->height();
	if(img->format() == Image::Format_ARGB) {
		for(uint i = 0; i < n; i++) {
			Color32 color = img->pixel(i);
			tga.mem[4 * i + 0] = color.b;
			tga.mem[4 * i + 1] = color.g;
			tga.mem[4 * i + 2] = color.r;
			tga.mem[4 * i + 3] = color.a;
		}
	}
	else {
		for(uint i = 0; i < n; i++) {
			Color32 color = img->pixel(i);
			tga.mem[3 * i + 0] = color.b;
			tga.mem[3 * i + 1] = color.g;
			tga.mem[3 * i + 2] = color.r;
		}
	}

	s << tga;
	
	tga.free();
	
	return true;
}


#if defined(HAVE_PNG)

static void user_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
	nvDebugCheck(png_ptr != NULL);
	
	Stream * s = (Stream *)png_ptr->io_ptr;
	s->serialize(data, (int)length);
	
	if (s->isError()) {
		png_error(png_ptr, "Read Error");
	}
}


Image * nv::ImageIO::loadPNG(Stream & s)
{
	nvCheck(!s.isError());
	
	// Set up a read buffer and check the library version
	png_structp png_ptr;
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
	//	nvDebug( "*** LoadPNG: Error allocating read buffer in file '%s'.\n", name );
		return false;
	}

	// Allocate/initialize a memory block for the image information
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		png_destroy_read_struct(&png_ptr, NULL, NULL);
	//	nvDebug( "*** LoadPNG: Error allocating image information for '%s'.\n", name );
		return false;
	}

	// Set up the error handling
	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	//	nvDebug( "*** LoadPNG: Error reading png file '%s'.\n", name );
		return false;
	}

	// Set up the I/O functions.
	png_set_read_fn(png_ptr, (void*)&s, user_read_data);


	// Retrieve the image header information
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type;
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL);


	if (color_type == PNG_COLOR_TYPE_PALETTE && bit_depth <= 8) {
		// Convert indexed images to RGB.
		png_set_expand(png_ptr);
	}
	else if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
		// Convert grayscale to RGB.
		png_set_expand(png_ptr);
	}
	else if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
		// Expand images with transparency to full alpha channels
		// so the data will be available as RGBA quartets.
		png_set_expand(png_ptr);
	}
	else if (bit_depth < 8) {
		// If we have < 8 scale it up to 8.
		//png_set_expand(png_ptr);
		png_set_packing(png_ptr);
	}

	// Reduce bit depth.
	if (bit_depth == 16) {
		png_set_strip_16(png_ptr);
	}

	// Represent gray as RGB
	if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
		png_set_gray_to_rgb(png_ptr);
	}

	// Convert to RGBA filling alpha with 0xFF.
	if (!(color_type & PNG_COLOR_MASK_ALPHA)) {
		png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
	}

	// @todo Choose gamma according to the platform?
	double screen_gamma = 2.2;
	int intent;
	if (png_get_sRGB(png_ptr, info_ptr, &intent)) {
		png_set_gamma(png_ptr, screen_gamma, 0.45455);
	}
	else {
		double image_gamma;
		if (png_get_gAMA(png_ptr, info_ptr, &image_gamma)) {
			png_set_gamma(png_ptr, screen_gamma, image_gamma);
		}
		else {
			png_set_gamma(png_ptr, screen_gamma, 0.45455);
		}
	}

	// Perform the selected transforms.
	png_read_update_info(png_ptr, info_ptr);

	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL);

	AutoPtr<Image> img(new Image());
	img->allocate(width, height);

	// Set internal format flags.
	if(color_type & PNG_COLOR_MASK_COLOR) {
		//img->flags |= PI_IF_HAS_COLOR;
	}
	if(color_type & PNG_COLOR_MASK_ALPHA) {
		//img->flags |= PI_IF_HAS_ALPHA;
		img->setFormat(Image::Format_ARGB);
	}

	// Read the image
	uint8 * pixels = (uint8 *)img->pixels();
	png_bytep * row_data = new png_bytep[sizeof(png_byte) * height];
	for (uint i = 0; i < height; i++) {
		row_data[i] = &(pixels[width * 4 * i]);
	}

	png_read_image(png_ptr, row_data);
	delete [] row_data;

	// Finish things up
	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	// RGBA to BGRA.
	uint num = width * height;
	for(uint i = 0; i < num; i++)
	{
		Color32 c = img->pixel(i);
		img->pixel(i) = Color32(c.b, c.g, c.r, c.a);
	}
	
	// Compute alpha channel if needed.
	/*if( img->flags & PI_IU_BUMPMAP || img->flags & PI_IU_ALPHAMAP ) {
		if( img->flags & PI_IF_HAS_COLOR && !(img->flags & PI_IF_HAS_ALPHA)) {
			img->ComputeAlphaFromColor();
		}
	}*/

	return img.release();
}


FloatImage * nv::ImageIO::loadFloatPNG(Stream & s)
{
	return NULL;
}


#endif // defined(HAVE_PNG)

#if defined(HAVE_JPEG)

static void init_source (j_decompress_ptr /*cinfo*/){
}

static boolean fill_input_buffer (j_decompress_ptr cinfo){
	struct jpeg_source_mgr * src = cinfo->src;
	static JOCTET FakeEOI[] = { 0xFF, JPEG_EOI };

	// Generate warning
	nvDebug("jpeglib: Premature end of file\n");

	// Insert a fake EOI marker
	src->next_input_byte = FakeEOI;
	src->bytes_in_buffer = 2;

	return TRUE;
}

static void skip_input_data (j_decompress_ptr cinfo, long num_bytes) {
	struct jpeg_source_mgr * src = cinfo->src;

	if(num_bytes >= (long)src->bytes_in_buffer) {
		fill_input_buffer(cinfo);
		return;
	}

	src->bytes_in_buffer -= num_bytes;
	src->next_input_byte += num_bytes;
}

static void term_source (j_decompress_ptr /*cinfo*/){
	// no work necessary here
}


Image * nv::ImageIO::loadJPG(Stream & s)
{
	nvCheck(!s.isError());
	
	// Read the entire file.
	Array<uint8> byte_array;
	byte_array.resize(s.size());
	s.serialize(byte_array.unsecureBuffer(), s.size());
	
	jpeg_decompress_struct cinfo;
	jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	cinfo.src = (struct jpeg_source_mgr *) (*cinfo.mem->alloc_small)
			((j_common_ptr) &cinfo, JPOOL_PERMANENT, sizeof(struct jpeg_source_mgr));
	cinfo.src->init_source = init_source;
	cinfo.src->fill_input_buffer = fill_input_buffer;
	cinfo.src->skip_input_data = skip_input_data;
	cinfo.src->resync_to_restart = jpeg_resync_to_restart;	// use default method
	cinfo.src->term_source = term_source;
	cinfo.src->bytes_in_buffer = byte_array.size();
	cinfo.src->next_input_byte = byte_array.buffer();

	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);

	/*
	cinfo.do_fancy_upsampling = FALSE;	// fast decompression
	cinfo.dct_method = JDCT_FLOAT;			// Choose floating point DCT method.
	*/

	uint8 * tmp_buffer = new uint8 [cinfo.output_width * cinfo.output_height * cinfo.num_components];
	uint8 * scanline = tmp_buffer;

	while( cinfo.output_scanline < cinfo.output_height ){
		int num_scanlines = jpeg_read_scanlines (&cinfo, &scanline, 1);
		scanline += num_scanlines * cinfo.output_width * cinfo.num_components;
	}

	jpeg_finish_decompress(&cinfo);

	AutoPtr<Image> img(new Image());
	img->allocate(cinfo.output_width, cinfo.output_height);

	Color32 * dst = img->pixels();
	const int size = img->height() * img->width();
	const uint8 * src = tmp_buffer;

	if( cinfo.num_components == 3 ) {
		img->setFormat(Image::Format_RGB);
		for( int i = 0; i < size; i++ ) {
			*dst++ = Color32(src[0], src[1], src[2]);
			src += 3;
		}
	}
	else {
		img->setFormat(Image::Format_ARGB);
		for( int i = 0; i < size; i++ ) {
			*dst++ = Color32(*src, *src, *src, *src);
			src++;
		}
	}

	delete [] tmp_buffer;
	jpeg_destroy_decompress (&cinfo);

	return img.release();
}

#endif // defined(HAVE_JPEG)

#if defined(HAVE_TIFF)

FloatImage * nv::ImageIO::loadFloatTIFF(Stream & s)
{
	nvCheck(!s.isError());
	return NULL;
}

FloatImage * nv::ImageIO::loadFloatTIFF(const char * fileName)
{
	TIFF * tif = TIFFOpen(fileName, "r");
	if (!tif)
	{
		nvDebug("Can't open '%s' for reading\n", fileName);
		return NULL;
	}
	
	::uint16 spp, bpp;
	::uint32 width, height;
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bpp);
	TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &spp);
	
	if (spp != 1 || (bpp != 8 && bpp != 16 && bpp != 32)) {
		nvDebug("Can't load '%s', only 1 sample per pixel supported\n", fileName);
		TIFFClose(tif);
		return NULL;
	}
	
	FloatImage * fimage = new FloatImage();
	fimage->allocate(spp, width, height);
	
	int linesize = TIFFScanlineSize(tif);
	tdata_t buf = (::uint8 *)nv::mem::malloc(linesize);
	
	for (uint y = 0; y < height; y++) {
		TIFFReadScanline(tif, buf, y, 0);
		
		float * dst = fimage->scanline(y, 0);
		
		if (bpp == 8) {
			for(uint x = 0; x < width; x++) {
				dst[x] = float(((::uint8 *)buf)[x]) / float(0xFF);
			}
		}
		else if (bpp == 16) {
			for(uint x = 0; x < width; x++) {
				dst[x] = float(((::uint16 *)buf)[x]) / float(0xFFFF);
			}
		}
		else /*if (bpp == 32)*/ {
			// Mantissa has only 24 bits, so drop 8 bits.
			for(uint x = 0; x < width; x++) {
				dst[x] = float(((::uint32 *)buf)[x] >> 8) / float(0xFFFFFF);
			}
		}
	}

	nv::mem::free(buf);
	
	TIFFClose(tif);
	
	return fimage;
}

#endif

#if 0

/** Save PNG*/
static bool SavePNG(const PiImage * img, const char * name) {
	nvCheck( img != NULL );
	nvCheck( img->mem != NULL );

	if( piStrCmp(piExtension(name), ".png" ) != 0 ) {
		return false;
	}
	
	if( img->flags & PI_IT_CUBEMAP ) {
		nvDebug("*** Cannot save cubemaps as PNG.");
		return false;
	}
	if( img->flags & PI_IT_DDS ) {
		nvDebug("*** Cannot save DDS surface as PNG.");
		return false;
	}

	nvDebug( "--- Saving '%s'.\n", name );
	
	PiAutoPtr<PiStream> ar( PiFileSystem::CreateFileWriter( name ) );
	if( ar == NULL ) {
		nvDebug( "*** SavePNG: Error, cannot save file '%s'.\n", name );
		return false;
	}

/*
public class PNGEnc {

    public static function encode(img:BitmapData):ByteArray {
        // Create output byte array
        var png:ByteArray = new ByteArray();
        // Write PNG signature
        png.writeUnsignedInt(0x89504e47);
        png.writeUnsignedInt(0x0D0A1A0A);
        // Build IHDR chunk
        var IHDR:ByteArray = new ByteArray();
        IHDR.writeInt(img.width);
        IHDR.writeInt(img.height);
        IHDR.writeUnsignedInt(0x08060000); // 32bit RGBA
        IHDR.writeByte(0);
        writeChunk(png,0x49484452,IHDR);
        // Build IDAT chunk
        var IDAT:ByteArray= new ByteArray();
        for(var i:int=0;i < img.height;i++) {
            // no filter
            IDAT.writeByte(0);
            var p:uint;
            if ( !img.transparent ) {
                for(var j:int=0;j < img.width;j++) {
                    p = img.getPixel(j,i);
                    IDAT.writeUnsignedInt(
                        uint(((p&0xFFFFFF) << 8)|0xFF));
                }
            } else {
                for(var j:int=0;j < img.width;j++) {
                    p = img.getPixel32(j,i);
                    IDAT.writeUnsignedInt(
                        uint(((p&0xFFFFFF) << 8)|
                        (shr(p,24))));
                }
            }
        }
        IDAT.compress();
        writeChunk(png,0x49444154,IDAT);
        // Build IEND chunk
        writeChunk(png,0x49454E44,null);
        // return PNG
        return png;
    }

    private static var crcTable:Array;
    private static var crcTableComputed:Boolean = false;

    private static function writeChunk(png:ByteArray, 
            type:uint, data:ByteArray) {
        if (!crcTableComputed) {
            crcTableComputed = true;
            crcTable = [];
            for (var n:uint = 0; n < 256; n++) {
                var c:uint = n;
                for (var k:uint = 0; k < 8; k++) {
                    if (c & 1) {
                        c = uint(uint(0xedb88320) ^ 
                            uint(c >>> 1));
                    } else {
                        c = uint(c >>> 1);
                    }
                }
                crcTable[n] = c;
            }
        }
        var len:uint = 0;
        if (data != null) {
            len = data.length;
        }
        png.writeUnsignedInt(len);
        var p:uint = png.position;
        png.writeUnsignedInt(type);
        if ( data != null ) {
            png.writeBytes(data);
        }
        var e:uint = png.position;
        png.position = p;
        var c:uint = 0xffffffff;
        for (var i:int = 0; i < (e-p); i++) {
            c = uint(crcTable[
                (c ^ png.readUnsignedByte()) & 
                uint(0xff)] ^ uint(c >>> 8));
        }
        c = uint(c^uint(0xffffffff));
        png.position = e;
        png.writeUnsignedInt(c);
    }
}
*/
}

#endif // 0

#if 0


namespace ImageIO {

	/** Init ImageIO plugins. */
	void InitPlugins() {
	//	AddInputPlugin( "", LoadANY );
		AddInputPlugin( "tga", LoadTGA );
#if HAVE_PNG
		AddInputPlugin( "png", LoadPNG );
#endif
#if HAVE_JPEG
		AddInputPlugin( "jpg", LoadJPG );
#endif
		AddInputPlugin( "dds", LoadDDS );
		
		AddOutputPlugin( "tga", SaveTGA );
	}
	
	/** Reset ImageIO plugins. */
	void ResetPlugins() {
		s_plugin_load_map.Clear();
		s_plugin_save_map.Clear();
	}
	
	/** Add an input plugin. */
	void AddInputPlugin( const char * ext, ImageInput_Plugin plugin ) {
		s_plugin_load_map.Add(ext, plugin);
	}
	
	/** Add an output plugin. */
	void AddOutputPlugin( const char * ext, ImageOutput_Plugin plugin ) {
		s_plugin_save_map.Add(ext, plugin);
	}

	
	bool Load(PiImage * img, const char * name, PiStream & stream) {
			
		// Get name extension.
		const char * extension = piExtension(name);
		
		// Skip the dot.
		if( *extension == '.' ) {
			extension++;
		}
		
		// Lookup plugin in the map.
		ImageInput_Plugin plugin = NULL;
		if( s_plugin_load_map.Get(extension, &plugin) ) {
			return plugin(img, stream);
		}
		
		/*foreach(i, s_plugin_load_map) {
			nvDebug("%s %s %d\n", s_plugin_load_map[i].key.GetStr(), extension, 0 == strcmp(extension, s_plugin_load_map[i].key));
		}
		
		nvDebug("No plugin found for '%s' %d.\n", extension, s_plugin_load_map.Size());*/
		
		return false;
	}

	bool Save(const PiImage * img, const char * name, PiStream & stream) {
				
		// Get name extension.
		const char * extension = piExtension(name);
		
		// Skip the dot.
		if( *extension == '.' ) {
			extension++;
		}
		
		// Lookup plugin in the map.
		ImageOutput_Plugin plugin = NULL;
		if( s_plugin_save_map.Get(extension, &plugin) ) {
			return plugin(img, stream);
		}
		
		return false;
	}
	
} // ImageIO

#endif // 0

