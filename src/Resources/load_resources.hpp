/*  load_resources.hpp

    Declarations of functions for loading meshes. */

#ifndef LOAD_RESOURCES_HPP
#define LOAD_RESOURCES_HPP

#include "./../Graphics/Model.hpp"
#include <memory>
#include <vector>

namespace Resources {

/*  Bitmap file header: the file header structure stores the following: The
    file type - a magic number used to verify the file is in fact a bmp. A
    valid bmp must have this equal 0x4d42 (the ASCII characters "BM").
    
    The size of the whole file.
    
    Two reserved parameters (no longer used anymore). These should be
    0 on a valid bitmap.
    
    The offset from the beginning of this structure to the beginning of
    the bitmap rgb data.
    
    Note that we disable padding as we need the structures to have exactly this
    size and layout in our code. */
#pragma pack(push, 1)
struct BitmapFileHeader {
    uint16_t file_type;
    uint32_t file_size;
    uint16_t reserved_1;
    uint16_t reserved_2;
    uint32_t rgb_offset;
};

/*  The info header follows the file header in a valid bmp structure. It
    stores information about the image encoding itself rather than just the
    file. The following data is stored:
        The number of bytes required for the structure as a whole, not
        including the file header or colour table (for bitmaps that may use
        them).
        
        The width and height of the bitmap in pixels.
        
        The number of planes for the bitmap - this is a now obsolete field
        but is still included in bmp files anyway - must be 1.
        
        The number of bits per pixel (we are interested in 24). 
        
        The compression method used - we are interested in uncompressed,
        using the value 0 indicates uncompressed RGB data.
        
        The size of the image data - this is set to 0 for uncompressed RGB
        bitmaps as this can be calculated from other fields.
        
        The number of x and y pixels that are expected per metre by the
        encoder of the bitmap - not relevant here but can be used for
        projects where a fixed size must be established.
        
        The number of colours used and considered important in the colour
        table. Since we exclusively support uncompresse true colour bitmaps
        for now, these fields are not of interest to us. */
struct BitmapInfoHeader {
    uint32_t bitmap_size;
    int32_t width;
    int32_t height;
    uint16_t plane_count;
    uint16_t bits_per_pixel;
    uint32_t compression_type;
    uint32_t image_size;
    int32_t x_pixels_per_metre;
    int32_t y_pixels_per_metre;
    uint32_t colours_used;
    uint32_t colours_important;
};

#pragma pack(pop)

/*  Representation of the .bmp bitmap file structure to simplify loading.
    While some platforms, such as Windows, provide an implementation of this
    in their relevant header files, it is not provided by others, so we
    recreate it here. */
struct BitmapFile {
    BitmapFileHeader file_header;
    BitmapInfoHeader info_header;
};

/*  RGBA / True colour pixel - a platform independent representation of an
    RGBA true colour pixel. */
struct RGBAPixel {
    uint8_t a;
    uint8_t b;
    uint8_t g;
    uint8_t r;
};

/*  For the time being, we are only really interested in true colour bitmaps,
    as true colour is verified by the rendering engine. */
struct TrueColourBitmap {
    int32_t width;
    int32_t height;
    std::vector<RGBAPixel> pixels;
};

/*  Load bitmap from bmp file. */
TrueColourBitmap* load_bitmap_from_file(std::string bitmap_path);

/*  Note that we do not return a smart pointer simply because a load can fail,
    and a resource load is potentially recoverable depending on the context, so
    we may want to accept nullptr as a return value. */
Graphics::Mesh* load_mesh_from_obj(std::string obj_path);

/*  Attach a texture to a mesh. */
void attach_texture(Graphics::Mesh& mesh, TrueColourBitmap& bitmap);

}

#endif