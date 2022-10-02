#ifndef AVOCADO_CORE_CONFIG
#define AVOCADO_CORE_CONFIG

#include <cstdint>

namespace avocado::core {

struct Config {
   static constexpr const char * const ENGINE_NAME = "Avocado Engine"; 
   static constexpr uint32_t ENGINE_MAJOR_VERSION = 1; 
   static constexpr uint32_t ENGINE_MINOR_VERSION = 0; 
   static constexpr uint32_t ENGINE_PATCH_VERSION = 1; 
};

} // namespace avocado::core.

#endif

