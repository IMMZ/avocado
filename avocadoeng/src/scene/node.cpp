#include "node.hpp"

namespace avocado::core {

Node::~Node() {
    delete matrix;
    delete rotation;
    delete translation;
    delete scale;
}

Node::Node(Node &&other) {
    _children = std::move(other._children);
    _name = std::move(other._name);
    matrix = other.matrix;
    other.matrix = nullptr;

    rotation = other.rotation;
    other.rotation = nullptr;

    translation = other.translation;
    other.translation = nullptr;

    scale = other.scale; other.scale = nullptr;
    _cameraIndex = other._cameraIndex;
    _hasMesh = other._hasMesh;
}

Node& Node::operator=(Node &&other) {
    if (this == &other)
        return *this;

    _children = std::move(other._children);
    _name = std::move(other._name);
    matrix = other.matrix;
    other.matrix = nullptr;

    rotation = other.rotation;
    other.rotation = nullptr;

    translation = other.translation;
    other.translation = nullptr;

    scale = other.scale; other.scale = nullptr;
    _cameraIndex = other._cameraIndex;
    _hasMesh = other._hasMesh;

    return *this;
}

void Node::addChild(Node * const childNode) {
    if (nullptr != childNode)
        _children.push_back(childNode);
}

bool Node::hasMatrix() const noexcept {
    return (nullptr != matrix);
}

bool Node::hasRotation() const noexcept {
    return (nullptr != rotation);
}

}
