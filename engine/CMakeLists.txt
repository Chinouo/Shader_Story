
set(VULKAN_DIR "/Users/dtup/VulkanSDK/1.3.216.0")
set(VULKAN_LIB "${VULKAN_DIR}/macOS/lib/libvulkan.1.dylib")
set(VULKAN_INCLUDE "${VULKAN_DIR}/macOS/include")
add_compile_definitions("VK_ICD_FILENAMES=${VULKAN_DIR}/macOS/share/vulkan/icd.d/MoltenVK_icd.json")
add_compile_definitions("VK_LAYER_PATH=${VULKAN_DIR}/macOS/share/vulkan/explicit_layer.d")


file(GLOB_RECURSE HEADER_FILES  "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
                                "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

      
file(GLOB_RECURSE SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")



add_library(Engine SHARED ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(Engine PUBLIC glfw)
target_link_libraries(Engine PUBLIC tinyobjloader)
target_link_libraries(Engine PUBLIC glm)
target_link_libraries(Engine PUBLIC ${VULKAN_LIB})
target_link_libraries(Engine PRIVATE imgui-vk)
# TODO: shaderc lib
target_link_libraries(Engine PUBLIC "/Users/dtup/VulkanSDK/1.3.216.0/macOS/lib/libshaderc_shared.1.dylib")



target_include_directories(Engine PUBLIC ${CMAKE_SOURCE_DIR})
target_include_directories(Engine PUBLIC ${CMAKE_SOURCE_DIR}/third_party)
target_include_directories(Engine PUBLIC ${VULKAN_INCLUDE})
#target_include_directories(Engine PUBLIC ${CMAKE_SOURCE_DIR}/third_party/imgui)


## copy shader files
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/runtime/shaders
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})




