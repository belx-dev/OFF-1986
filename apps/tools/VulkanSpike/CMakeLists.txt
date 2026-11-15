# Spike: render a dumped render Frame with direct Vulkan, offscreen.
# Built only with -DPOSEIDON_BUILD_VULKAN_SPIKE=ON and the vcpkg feature
# `vulkan-spike` (preset linux-x64-clang-rwdi-vulkan sets both).  Links
# nothing from the engine: the dump format header is self-contained.
cmake_minimum_required(VERSION 3.25)
project(PoseidonVulkanSpike LANGUAGES CXX)

find_package(VulkanHeaders CONFIG REQUIRED)
find_library(VULKAN_LOADER_LIB NAMES vulkan vulkan-1 REQUIRED)
find_program(GLSL_COMPILER NAMES glslc glslangValidator glslang REQUIRED
    HINTS ${VCPKG_INSTALLED_DIR}/${VCPKG_HOST_TRIPLET}/tools/glslang ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/glslang)

set(SPIKE_SHADERS flat.vert flat.frag)
set(SPIKE_SPV)
foreach(sh IN LISTS SPIKE_SHADERS)
    set(src ${CMAKE_CURRENT_SOURCE_DIR}/shaders/${sh})
    set(spv ${CMAKE_CURRENT_BINARY_DIR}/${sh}.spv)
    if(GLSL_COMPILER MATCHES "glslc")
        set(cmd ${GLSL_COMPILER} --target-env=vulkan1.3 -O ${src} -o ${spv})
    else()
        set(cmd ${GLSL_COMPILER} -V --target-env vulkan1.3 ${src} -o ${spv})
    endif()
    add_custom_command(OUTPUT ${spv} COMMAND ${cmd} DEPENDS ${src} COMMENT "SPIR-V ${sh}" VERBATIM)
    list(APPEND SPIKE_SPV ${spv})
endforeach()
set(SPIKE_HEADER ${CMAKE_CURRENT_BINARY_DIR}/spike_shaders.hpp)
add_custom_command(OUTPUT ${SPIKE_HEADER}
    COMMAND ${CMAKE_COMMAND} -DOUT=${SPIKE_HEADER} "-DNAMES=kFlatVert;kFlatFrag" "-DFILES=${SPIKE_SPV}"
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedSpirv.cmake
    DEPENDS ${SPIKE_SPV} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedSpirv.cmake
    COMMENT "Embedding SPIR-V" VERBATIM)

add_executable(PoseidonVulkanSpike main.cpp ${SPIKE_HEADER})
target_include_directories(PoseidonVulkanSpike PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/engine)   # Poseidon/Graphics/Rendering/Frame/FrameDumpFormat.hpp only
target_link_libraries(PoseidonVulkanSpike PRIVATE Vulkan::Headers ${VULKAN_LOADER_LIB})
target_compile_features(PoseidonVulkanSpike PRIVATE cxx_std_20)
set_target_properties(PoseidonVulkanSpike PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${DIST_DIR}")
