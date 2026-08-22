#ifndef SCENE_CAMERA_HPP
#define SCENE_CAMERA_HPP

#include "../math/matrix.hpp"
#include "../math/vecn.hpp"

namespace avocado::core {

struct Camera {
    math::Mat4x4 perspective;
    math::Mat4x4 localMatrix;
    math::vec3f position;
    math::vec3f up;
    math::vec3f targetPosition;
};

std::ostream& operator<<(std::ostream &stream, const Camera &camera);

}

#endif // ifndef SCENE_CAMERA_HPP
