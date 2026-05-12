#ifndef SCENE_HPP
#define SCENE_HPP

#include "node.hpp"
#include "sampler.hpp"

#include "../vertex.hpp"

#include "../math/matrix.hpp"
#include "../math/quaternion.hpp"

#include "../vulkan/buffer.hpp"
#include "../vulkan/image.hpp"
#include "../vulkan/vulkan_core.h"

namespace avocado::core {

struct Scene {
    std::vector<Node*> _rootNodes;
    std::string _name;
};

}

#endif // ifndef SCENE_HPP
