/*  load_mesh.cpp

    Implementations of functions for loading meshes and relevant helper
    functions. */

#include "load_mesh.hpp"
#include <fstream>
#include <sstream>

namespace Resources {

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
                                    vertices[v1_adj],
                                    vertices[v2_adj],
                                    vertices[v3_adj]
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