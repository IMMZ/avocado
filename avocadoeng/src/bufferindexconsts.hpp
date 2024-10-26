#ifndef AVOCADO_CORE_BUFFERINDEXCONSTS_HPP
#define AVOCADO_CORE_BUFFERINDEXCONSTS_HPP

namespace avocado::core {

enum class BufferIndex: size_t {
    CopyImage = 2
    , FirstImageLayoutTransition = 3
    , SecondImageLayoutTransition = 4
};

}

#endif // AVOCADO_CORE_BUFFERINDEXCONSTS_HPP
