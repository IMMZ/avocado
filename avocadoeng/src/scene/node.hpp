#ifndef SCENE_NODE_HPP
#define SCENE_NODE_HPP

#include "../math/matrix.hpp"
#include "../math/quaternion.hpp"
#include "../math/vecn.hpp"

#include <string>
#include <vector>


namespace avocado::core {

struct Mesh;

struct Node {
    std::vector<Node*> _children;
    std::string _name;
    avocado::math::Mat4x4 *matrix = nullptr;
    avocado::math::Quaternion *rotation = nullptr;
    avocado::math::vec3f * translation = nullptr;
    avocado::math::vec3f * scale = nullptr;
    int _cameraIndex = -1;
    bool _hasMesh = false;

    [[nodiscard]] inline bool hasTranslation() noexcept {
        return (nullptr != translation);
    }

    [[nodiscard]] inline bool hasScale() noexcept {
        return (nullptr != scale);
    }

    void addChild(Node * const childNode);
    [[nodiscard]] bool hasMatrix() const noexcept;
    [[nodiscard]] bool hasRotation() const noexcept;
    [[nodiscard]] inline bool hasCamera() const noexcept {
        return (_cameraIndex > -1);
    }

    [[nodiscard]] inline bool hasMesh() const noexcept {
        return _hasMesh;
    }
};

}

#endif
