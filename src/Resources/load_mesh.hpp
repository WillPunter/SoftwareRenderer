/*  load_mesh.hpp

    Declarations of functions for loading meshes. */

#ifndef LOAD_MESH_HPP
#define LOAD_MESH_HPP

#include "./../Graphics/Model.hpp"
#include <memory>

namespace Resources {

/*  Note that we do not return a smart pointer simply because a load can fail,
    and a resource load is potentially recoverable depending on the context, so
    we may want to accept nullptr as a return value. */
Graphics::Mesh* load_mesh_from_obj(std::string obj_path);

}

#endif