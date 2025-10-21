/*  load_resources.cpp

    Implementations of functions for loading meshes and relevant helper
    functions. */

#include "load_resources.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

namespace Resources {
/*  Load bitmap - returns a unique pointer to a true colour bitmap
    structure. */
TrueColourBitmap* load_bitmap_from_file(std::string bitmap_path) {
    std::ifstream in_file(bitmap_path, std::ifstream::binary);

    TrueColourBitmap* result = nullptr;

    BitmapFileHeader file_header {0};
    BitmapInfoHeader info_header {0};

    if (in_file.is_open()) {
        /*  Unfortunately, we cannot use static_cast in this case as we
            have not provided a means to convert a file_header to a char
            (as this makes no semantic sense). We cast to a char * as we
            want to read the given number of bytes into the file. */
        in_file.read((char*) &file_header, sizeof(file_header));

        if (!in_file) {
            std::cerr << "Load bitmap error - could not read file header of"
                " file " << bitmap_path << "." << std::endl;
            return nullptr;
        }

        /*  Verify that magic number is present, confirming that this is a
            bitmap file. */
        if (file_header.file_type != 0x4d42) {
            std::cerr << "Load bitmap error - file " << bitmap_path <<
                " is not a bitmap file." << std::endl;
            return nullptr;
        }

        /*  Read info header. */
        in_file.read((char *) &info_header, sizeof(info_header));

        if (!in_file) {
            std::cerr << "Load bitmap error - could not read info header of"
                " file " << bitmap_path << "." << std::endl;
            return nullptr;
        }

        /*  Verify that file is uncompressed and uses true colour. */
        if (info_header.compression_type != 0) {
            std::cerr << "Load bitmap error - compression not supported and"
                " file " << bitmap_path << " is compressed." << std::endl;
            return nullptr;
        }

        if (info_header.bits_per_pixel < 24) {
            std::cerr << "Load bitmap error - bitmap " << bitmap_path << " is"
                " not true colour - non-true colour bitmaps are not supported."
                << std::endl;
            return nullptr;
        }

        /*  Move file pointer to the beginning of the bitmap. */
        in_file.seekg(file_header.rgb_offset, in_file.beg);

        /*  Height can be positive or negative depending on how the lines of
            pixels are ordered in the bitmap data. */
        int32_t pos_height = abs(info_header.height);

        std::vector<RGBAPixel> pixels(info_header.width * pos_height);
        int num_pixels = 0;

        size_t bytes_per_pixel = info_header.bits_per_pixel / 8;
        size_t line_bytes = info_header.width * bytes_per_pixel;
        size_t padding = (line_bytes % 4 == 0) ? 0 : (4 - (line_bytes % 4));
        size_t row_bytes = line_bytes + padding;
        size_t buffer_size = row_bytes * pos_height;

        /*  Read bitmap lines. */
        std::unique_ptr<uint8_t> raw_data =
            std::unique_ptr<uint8_t>(new uint8_t[buffer_size]);
        in_file.read((char *) raw_data.get(), buffer_size);

        if (!in_file) {
            std::cerr << "Load bitmap error - could not load rgb data for "
                << bitmap_path << "." << std::endl;
            return nullptr;
        }

        int pixel_index = 0;
        int line_index = 0;

        /*  Copy pixel data from loaded buffer to output buffer. */
        for (int i = 0; i < pos_height; i++) {
            /*  Compute start of new line - this depends on the orientation
                of the bitmap. */
            if (info_header.height < 0) {
                line_index = i * row_bytes;
            } else {
                line_index = (pos_height - 1 - i) * row_bytes;
            }

            pixel_index = line_index;

            for (int j = 0; j < info_header.width; j++) {
                if (info_header.bits_per_pixel == 32) {
                    pixels[num_pixels] = {
                        raw_data.get()[pixel_index],
                        raw_data.get()[pixel_index + 1],
                        raw_data.get()[pixel_index + 2],
                        raw_data.get()[pixel_index + 3]
                    };
                } else if (info_header.bits_per_pixel == 24) {
                    pixels[num_pixels] = {
                        255,
                        raw_data.get()[pixel_index],
                        raw_data.get()[pixel_index + 1],
                        raw_data.get()[pixel_index + 2]
                    };
                }

                pixel_index += bytes_per_pixel;
                num_pixels ++;
            }
        }

        /*  Reading is complete and successful. */
        result = new TrueColourBitmap {
            info_header.width,
            pos_height,
            pixels
        };
    } else {
        std::cerr << "Load bitmap error - failed to open file " << bitmap_path << "." << std::endl;
    }

    in_file.close();

    return result;
};

/*  Load mesh - returns a unique pointer to a mesh. This will be nullptr if the
    file could not be opened. If the file could be parsed. Note that lines that
    cannot be parsed in the .obj file will simply be skipped (so as to allow
    the geometry of files with currently unsupported features to be loaded). */
Graphics::Mesh* load_mesh_from_obj(std::string obj_path) {
    Graphics::Mesh* mesh = nullptr;

    /*  Open file. */
    std::ifstream obj_file(obj_path);

    if (obj_file.is_open()) {
        bool failed = false;

        std::vector<Maths::Vector<double, 4>> vertices;
        std::vector<Graphics::Triangle> triangles;

        std::string line;
        while (getline(obj_file, line)) {
            /*  Parse line as a mnemonic follow by a sequence of relevant
                numbers. The logic to do this is already provided by string
                stream, so there is no point in rewriting it. */
            std::stringstream line_stream(line);

            std::string mnemonic = "";

            if (line_stream >> mnemonic) {
                if (mnemonic == "v") {
                    /*  Attempt to read three doubles - if not, dicard line. */
                    double x;
                    double y;
                    double z;

                    if (line_stream >> x >> y >> z) {
                        /*  Add vertex to vertices vector. */
                        vertices.push_back(
                            Maths::Vector<double, 4> { x, y, z, 1.0 }
                        );
                    };
                } else if (mnemonic == "f") {
                    int v1;
                    int v2;
                    int v3;

                    if (line_stream >> v1 >> v2 >> v3) {
                        /*  OBJ files encode vertex numbers starting at 1 - we
                            must subtract 1 to get them as indices to a
                            zero-indexed vector. */
                        int v1_adj = v1 - 1;
                        int v2_adj = v2 - 1;
                        int v3_adj = v3 - 1;

                        /*  Add triangle to triangles vector. */
                        if (
                            v1_adj >= 0 && v1_adj < vertices.size() &&
                            v2_adj >= 0 && v2_adj < vertices.size() &&
                            v3_adj >= 0 && v3_adj < vertices.size()
                        ) {
                            triangles.push_back(Graphics::Triangle {
                                {
                                    /*  Position, intensity, red, green,
                                        blue, texture x, texture_y.

                                        The subsequent attributes are
                                        set during perspective projection so
                                        are all set to 0 here. */
                                    Graphics::Point {
                                        vertices[v1_adj],
                                        0.0,
                                        255.0, 255.0, 255.0,
                                        0.0, 0.0,

                                        0.0, 0.0, 0.0, 0.0,
                                        0.0, 0.0, 0.0
                                    },

                                    Graphics::Point {
                                        vertices[v2_adj],
                                        0.0,
                                        255.0, 255.0, 255.0,
                                        0.0, 0.0,

                                        0.0, 0.0, 0.0, 0.0,
                                        0.0, 0.0, 0.0
                                    },

                                    Graphics::Point {
                                        vertices[v3_adj],
                                        0.0,
                                        255.0, 255.0, 255.0,
                                        0.0, 0.0,

                                        0.0, 0.0, 0.0, 0.0,
                                        0.0, 0.0, 0.0
                                    }
                                }
                            });
                        }
                    }
                }
            }
        };

        if (!failed) {
            mesh = new Graphics::Mesh();
            mesh->triangles = triangles;
        }
    }

    obj_file.close();

    return mesh;
}

}