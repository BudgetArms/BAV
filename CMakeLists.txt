cmake_minimum_required(VERSION 3.16)

# BudgetArms Vulkan (Renderer)
project(BAV)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# Get Vulkan SDK
find_package(Vulkan REQUIRED)


include(FetchContent)

# Add GLFW
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
        GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(glfw)

# Add GLM
FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 1.0.3
        GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(glm)

# Add VMA (Vulkan Memory Allocator)
FetchContent_Declare(
        vma
        GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
        GIT_TAG v3.3.0
        GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(vma)

# Add stb (stb single-file public domain libraries, an image loader)
FetchContent_Declare(
        stb
        GIT_REPOSITORY https://github.com/nothings/stb.git
        GIT_TAG master
        GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(stb)

add_library(stb_lib INTERFACE)
target_include_directories(stb_lib INTERFACE ${stb_SOURCE_DIR})


add_executable(
        ${PROJECT_NAME}
        BAV/main.cpp
        BAV/VulkanApplication.cpp
        BAV/ConversionHelpers.cpp
        BAV/CreationHelper.cpp
        BAV/CreationHelper.hpp
)


# Link libraries
target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE
        glfw
        glm
        Vulkan::Vulkan
        GPUOpen::VulkanMemoryAllocator
        stb_lib
)


# Include directories
target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
        glfw
        glm
)



find_program(SLANGC_EXECUTABLE slangc HINTS $ENV{VULKAN_SDK}/bin REQUIRED)
find_program(GLSLANG_VALIDATOR "glslangValidator" HINTS $ENV{VULKAN_SDK}/bin REQUIRED)


function(add_slang_shader_target TARGET)

    cmake_parse_arguments("SHADER" "" "" "SOURCE" ${ARGN})

    set(SHADERS_DIR "${CMAKE_CURRENT_LIST_DIR}/Shaders")
    set(OUTPUT_FILE "${SHADERS_DIR}/${TARGET}.spv")

    set(ENTRY_POINTS -entry VertexMain -entry FragmentMain)

    # Compile Shaders
    add_custom_command(
        OUTPUT "${OUTPUT_FILE}"
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADERS_DIR}
        COMMAND
            ${SLANGC_EXECUTABLE}
            ${SHADER_SOURCE}
            -target spirv
            -profile spirv_1_4
            -emit-spirv-directly
            -fvk-use-entrypoint-name ${ENTRY_POINTS}
            -o ${OUTPUT_FILE}

        DEPENDS "${SHADER_SOURCE}"
        COMMENT "Compiling Shader (Slang)"
        VERBATIM
    )

    add_custom_target(
        ${TARGET}
        DEPENDS "${OUTPUT_FILE}"
    )

endfunction()


function(add_glsl_shader_target TARGET)

    cmake_parse_arguments("SHADER" "" "" "SOURCE" ${ARGN})

    set(SHADERS_DIR "${CMAKE_CURRENT_LIST_DIR}/Shaders")
    set(OUTPUT_FILE "${SHADERS_DIR}/${TARGET}.spv")

    # Compile Shaders
    add_custom_command(
        OUTPUT "${OUTPUT_FILE}"
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADERS_DIR}
        COMMAND
            glslangvalidator
            -V
            ${SHADER_SOURCE}
            -o ${OUTPUT_FILE}

        DEPENDS "${SHADER_SOURCE}"
        COMMENT "Compiling Shader (GLSL)"
        VERBATIM
    )

    add_custom_target(
        ${TARGET}
        DEPENDS "${OUTPUT_FILE}"
    )

endfunction()

# Macro for extracting Extension Name
macro(ExtensionToPascalCase ExtensionResult Extension)
    string(SUBSTRING "${Extension}" 1  1 FirstLetter)
    string(SUBSTRING "${Extension}" 2 -1 RestOfLetters)

    string(TOUPPER "${FirstLetter}" FirstLetter)

    set(${ExtensionResult} "${FirstLetter}${RestOfLetters}")
endmacro()


# Add Shaders
file(GLOB SlangShaders "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*.slang")

foreach (SlangShader IN LISTS SlangShaders)
    get_filename_component(SLANG_SHADER_NAME "${SlangShader}" NAME_WE)

    get_filename_component(ShaderExtension "${SlangShader}" EXT)
    ExtensionToPascalCase(ShaderExtension "${ShaderExtension}")
    string(APPEND SLANG_SHADER_NAME "${ShaderExtension}")

    add_slang_shader_target(
            "${SLANG_SHADER_NAME}"
            SOURCE "${SlangShader}"
    )

    add_dependencies(${PROJECT_NAME} "${SLANG_SHADER_NAME}")
endforeach()


file(GLOB GlslShaders
    "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*.vert"
    "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*.frag"
)

foreach (GlslShader IN LISTS GlslShaders)
    get_filename_component(GLSL_SHADER_NAME "${GlslShader}" NAME_WE)

    get_filename_component(ShaderExtension "${GlslShader}" EXT)
    ExtensionToPascalCase(ShaderExtension "${ShaderExtension}")
    string(APPEND GLSL_SHADER_NAME "${ShaderExtension}")

    add_glsl_shader_target(
            "${GLSL_SHADER_NAME}"
            SOURCE "${GlslShader}"
    )

    add_dependencies(${PROJECT_NAME} "${GLSL_SHADER_NAME}")
endforeach()



add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Shaders
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/Shaders
)
