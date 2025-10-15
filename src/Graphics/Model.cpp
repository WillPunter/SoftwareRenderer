/*  Model.cpp */

#include "Model.hpp"
#include "./../Maths/Transform.hpp"

namespace Graphics {

Maths::Matrix<double, 4, 4> model_transform(const Model& model) {
    return Maths::make_translation(
        model.position(0),
        model.position(1),
        model.position(2)
    ) * Maths::make_rotation_model(
        model.rotation(0),
        model.rotation(1),
        model.rotation(2)
    ) * Maths::make_enlargement(
        model.scale(0),
        model.scale(1),
        model.scale(2)
    );
}

}