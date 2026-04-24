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
    set(SHADERS_DIR ${CMAKE_CURRENT_LIST_DIR}/Shaders)
    set(ENTRY_POINTS -entry VertexMain -entry FragmentMain)

    add_custom_command(
        OUTPUT ${SHADERS_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADERS_DIR}
    )

     # Get Shader source name (by removing .slang from shader source)
    string(REPLACE ".slang" "" SHADER_NAME ${SHADER_SOURCE})
    string(APPEND SHADER_NAME "Slang")

    # Compile Shaders
    add_custom_command(
        OUTPUT ${SHADERS_DIR}/${SHADER_NAME}.spv
        COMMAND
            ${SLANGC_EXECUTABLE}
            ${SHADER_SOURCE}
            -target spirv
            -profile spirv_1_4
            -emit-spirv-directly
            -fvk-use-entrypoint-name ${ENTRY_POINTS}
            -o ${SHADER_NAME}.spv

        WORKING_DIRECTORY ${SHADERS_DIR}
        DEPENDS ${SHADERS_DIR} ${SHADER_SOURCES}
        COMMENT "Compiling Shader (Slang)"
        VERBATIM
    )

    add_custom_target(
        ${TARGET}
        DEPENDS ${SHADERS_DIR}/${SHADER_NAME}.spv
    )

endfunction()


function(add_glsl_shader_target TARGET)

    cmake_parse_arguments("SHADER" "" "" "SOURCE" ${ARGN})
    set(SHADERS_DIR ${CMAKE_CURRENT_LIST_DIR}/Shaders)

    add_custom_command(
        OUTPUT ${SHADERS_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADERS_DIR}
    )

    if( ${SHADER_SOURCE} MATCHES ".frag")

        string(REPLACE ".frag" "" SHADER_NAME ${SHADER_SOURCE})
        string(APPEND SHADER_NAME "Frag")

    elseif( ${SHADER_SOURCE} MATCHES ".vert")

        string(REPLACE ".vert" "" SHADER_NAME ${SHADER_SOURCE})
        string(APPEND SHADER_NAME "Vert")

    else()

        string(REPLACE "." "" SHADER_NAME ${SHADER_SOURCE})
        string(APPEND SHADER_NAME "Idk")

    endif()

    # Compile Shaders
    add_custom_command(
        OUTPUT ${SHADERS_DIR}/${SHADER_NAME}.spv
        COMMAND
            glslangvalidator
            -V
            ${SHADER_SOURCE}
            -o ${SHADER_NAME}.spv

        WORKING_DIRECTORY ${SHADERS_DIR}
        DEPENDS ${SHADERS_DIR} ${SHADER_SOURCES}
        COMMENT "Compiling Shader (GLSL)"
        VERBATIM
    )

    add_custom_target(
        ${TARGET}
        DEPENDS ${SHADERS_DIR}/${SHADER_NAME}.spv
    )

endfunction()



# Shaders
add_slang_shader_target(
    SlangShader
    SOURCE "Shader.slang"
)

add_glsl_shader_target(
    GLSLVertShader
    SOURCE "Shader.vert"
)
add_glsl_shader_target(
    GLSLFragShader
    SOURCE "Shader.frag"
)

# Red Triangle Shaders
add_slang_shader_target(
    RedTriangleSlangShader
    SOURCE "RedTriangle.slang"
)

add_glsl_shader_target(
    RedTriangleGLSLVertShader
    SOURCE "RedTriangle.vert"
)
add_glsl_shader_target(
    RedTriangleGLSLFragShader
    SOURCE "RedTriangle.frag"
)


# Red Triangle Shaders

add_glsl_shader_target(
    ColorTriangleGLSLVertShader
    SOURCE "ColorTriangle.vert"
)
add_glsl_shader_target(
    ColorTriangleGLSLFragShader
    SOURCE "ColorTriangle.frag"
)


add_dependencies(${PROJECT_NAME} SlangShader)
add_dependencies(${PROJECT_NAME} GLSLVertShader)
add_dependencies(${PROJECT_NAME} GLSLFragShader)

add_dependencies(${PROJECT_NAME} RedTriangleSlangShader)
add_dependencies(${PROJECT_NAME} RedTriangleGLSLVertShader)
add_dependencies(${PROJECT_NAME} RedTriangleGLSLFragShader)

add_dependencies(${PROJECT_NAME} ColorTriangleGLSLVertShader)
add_dependencies(${PROJECT_NAME} ColorTriangleGLSLFragShader)



add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Shaders
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/Shaders
)
