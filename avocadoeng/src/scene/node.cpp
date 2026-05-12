#include "node.hpp"

namespace avocado::core {

void Node::addChild(Node * const childNode) {
    if (nullptr != childNode)
        _children.push_back(childNode);
}

bool Node::hasMatrix() const noexcept {
    return (nullptr != matrix);
}

bool Node::hasMesh() const noexcept {
    return (nullptr != mesh);
}

bool Node::hasRotation() const noexcept {
    return (nullptr != rotation);
}

}
