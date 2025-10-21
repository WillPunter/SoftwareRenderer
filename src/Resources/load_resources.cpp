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

/*  Each point on a face in an obj file can consist of up to three indices:
        The position index (required).
        The texture coordinate index (optional).
        The normal coordinate index (optional).
    These are written in the form:
        p/t/n
    But for conciseness the following are permuted:
        p/t/n
        p/t
        p//n
    These cases need to be handled.
    
    Note that all of these are integer indices to vertices that should already
    have been read in. */
enum class ObjTripletFormat {
    P,
    PT,
    PN,
    PTN,
    ERROR
};

static ObjTripletFormat parse_face_point_triplet(
    std::stringstream& line,
    int out[3]
) {
    /*  Try to read postion index. */
    line >> out[0];

    if (!line) {
        return ObjTripletFormat::ERROR;
    }

    /*  Consume whitespace. */
    line >> std::ws;

    /*  If there are no more characters, the triple is of form P. */
    char c;
    if (!line.get(c)) {
        return ObjTripletFormat::P;
    }

    /*  Check if a / was read. If not, then put the character read back into
        the stream. */
    if (c != '/') {
        line.unget();
        return ObjTripletFormat::P;
    }

    /*  Consume whitespace before next significant character. */
    line >> std::ws;

    char next = line.peek();

    /*  We expect something to follow a / so return an error if there are no
        more characters. */
    if (next == EOF) {
        return ObjTripletFormat::ERROR;
    }

    /*  A PN form is encoded as P//N, so we check for a second slash and parse
        as PN if so. */
    if (next == '/') {
        line.get();
        line >> std::ws;
        line >> out[2];

        if (!line) {
            return ObjTripletFormat::ERROR;
        }

        return ObjTripletFormat::PN;
    }

    /*  If we did not read a second slash, then it is of PT or PTN form, so
        read T. */
    line >> out[1];

    if (!line) {
        return ObjTripletFormat::ERROR;
    }

    line >> std::ws;

    /*  Check if there is another slash, denoting PTN. */
    if (!line.get(c)) {
        return ObjTripletFormat::PT;
    }

    /*  If the next character is not a slash, then we have finished parsing a
        PT triple successfully. */
    if (c != '/') {
        line.unget();
        return ObjTripletFormat::PT;
    }

    /*  If we read another slash, then we expect another integer for N. */
    line >> std::ws;
    line >> out[2];

    if (!line) {
        return ObjTripletFormat::ERROR;
    }

    return ObjTripletFormat::PTN;
}

/*  Load mesh - returns a unique pointer to a mesh. This will be nullptr if the
    file could not be opened. If the file could be parsed. Note that lines that
    cannot be parsed in the .obj file will simply be skipped (so as to allow
    the geometry of files with currently unsupported features to be loaded). */
struct FaceTriple {
    int position_index;
    int texture_coord_index;
    int normal_index;
};

Graphics::Mesh* load_mesh_from_obj(std::string obj_path) {
    Graphics::Mesh* mesh = nullptr;

    /*  Open file. */
    std::ifstream obj_file(obj_path);

    if (obj_file.is_open()) {
        bool failed = false;

        std::vector<Maths::Vector<double, 4>> vertices;
        std::vector<Maths::Vector<double, 4>> texture_coords;
        std::vector<Maths::Vector<double, 4>> normals;
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
                    } else {
                        failed = true;
                        break;
                    };
                } else if (mnemonic == "vt") {
                    /*  Vertex texture coords - attempt to read two doubles -
                        if not, dicard line. */
                    double x;
                    double y;
                    double z;

                    if (line_stream >> x >> y) {
                        /*  Add texture coord to texture coords array. */
                        texture_coords.push_back(
                            Maths::Vector<double, 4> { x, y, 1.0, 1.0 }
                        );
                    } else {
                        failed = true;
                        break;
                    };
                } else if (mnemonic == "vn") {
                    /*  Vertex normal - attempt to read three doubles - if not,
                        dicard line. */
                    double x;
                    double y;
                    double z;

                    if (line_stream >> x >> y >> z) {
                        /*  Add normal to normals array. */
                        normals.push_back(
                            Maths::Vector<double, 4> { x, y, z, 1.0 }
                        );
                    } else {
                        failed = true;
                        break;
                    };
                } else if (mnemonic == "f") {
                    /*  Read first P/T/N triple. */
                    int v1[3];
                    ObjTripletFormat v1_form =
                        parse_face_point_triplet(line_stream, v1);

                    if (v1_form == ObjTripletFormat::ERROR) {
                        failed = true;
                        break;
                    }

                    /*  Read second P/T/N triple. */
                    int v2[3];
                    ObjTripletFormat v2_form =
                        parse_face_point_triplet(line_stream, v2);

                    if (v2_form == ObjTripletFormat::ERROR) {
                        failed = true;
                        break;
                    }

                    /*  Read third P/T/N triple. */
                    int v3[3];
                    ObjTripletFormat v3_form =
                        parse_face_point_triplet(line_stream, v3);

                    if (v3_form == ObjTripletFormat::ERROR) {
                        failed = true;
                        break;
                    }

                    /*  Verify that all vertices have the same form. */
                    if (v1_form != v2_form || v2_form != v3_form) {
                        failed = true;
                        break;
                    }

                    /*  Fill out point structures - we do not use normals for
                        now as we compute them in the rendering pipeline. */
                    Graphics::Point point_1 {};

                    point_1.pos = vertices[v1[0] - 1];
                    point_1.r = 255.0;
                    point_1.g = 255.0;
                    point_1.b = 255.0;

                    if (
                        v1_form == ObjTripletFormat::PT ||
                        v1_form == ObjTripletFormat::PTN
                    ) {
                        Maths::Vector<double, 4> tex_vertex =
                            texture_coords[v1[1] - 1];
                        point_1.tex_x = tex_vertex(0);
                        point_1.tex_y = tex_vertex(1);
                    }

                    Graphics::Point point_2 {};

                    point_2.pos = vertices[v2[0] - 1];
                    point_2.r = 255.0;
                    point_2.g = 255.0;
                    point_2.b = 255.0;

                    if (
                        v2_form == ObjTripletFormat::PT ||
                        v2_form == ObjTripletFormat::PTN
                    ) {
                        Maths::Vector<double, 4> tex_vertex =
                            texture_coords[v2[1] - 1];
                        point_2.tex_x = tex_vertex(0);
                        point_2.tex_y = tex_vertex(1);
                    }

                    Graphics::Point point_3 {};

                    point_3.pos = vertices[v3[0] - 1];
                    point_3.r = 255.0;
                    point_3.g = 255.0;
                    point_3.b = 255.0;

                    if (
                        v3_form == ObjTripletFormat::PT ||
                        v3_form == ObjTripletFormat::PTN
                    ) {
                        Maths::Vector<double, 4> tex_vertex =
                            texture_coords[v3[1] - 1];
                        point_3.tex_x = tex_vertex(0);
                        point_3.tex_y = tex_vertex(1);
                    }

                    std::cout << "Tex coords: (" << std::to_string(point_1.tex_x) << ", " << std::to_string(point_1.tex_y) << "), (" << std::to_string(point_2.tex_x) << ", " << std::to_string(point_2.tex_y) << "), (" << std::to_string(point_3.tex_x) << ", " << std::to_string(point_3.tex_y) << ")" << std::endl;

                    /*  Add triangle to triangles vector. */
                    triangles.push_back(Graphics::Triangle {
                        { point_1, point_2, point_3 },
                        nullptr
                    });
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

/*  Attach a texture to a mesh that already has texture coordinates. */
void attach_texture(Graphics::Mesh& mesh, TrueColourBitmap& bitmap) {
    /*  Set the bitmap pointer of all triangles to this bitmap. */
    for (Graphics::Triangle& tri : mesh.triangles) {
        tri.bitmap_ptr = &bitmap;
    }
};

}