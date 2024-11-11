set(library-floder "Library")

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

if(NOT TARGET imgui)
    include(imgui.cmake)
    set_target_properties(imgui PROPERTIES FOLDER ${library-floder}/imgui)
endif()

if(NOT TARGET glfw)
    option(GLFW_BUILD_EXAMPLES "" OFF)
    option(GLFW_BUILD_TESTS "" OFF)
    option(GLFW_BUILD_DOCS "" OFF)
    option(GLFW_INSTALL "" OFF)
    add_subdirectory(glfw)
    set_target_properties(glfw PROPERTIES FOLDER ${library-floder}/glfw)
    set_target_properties(update_mappings PROPERTIES FOLDER ${library-floder}/glfw)
endif()

if(NOT TARGET spdlog)
    option(SPDLOG_BUILD_EXAMPLE "" OFF)
    option(SPDLOG_INSTALL "" OFF)
    add_subdirectory(spdlog)
    set_target_properties(spdlog PROPERTIES FOLDER ${library-floder}/spdlog)
endif()

if(NOT TARGET stb)
    include(stb.cmake)
    set_target_properties(stb PROPERTIES FOLDER ${library-floder}/stb)
endif()

if(NOT TARGET sol2)
    add_subdirectory(sol2-3.3.0)
endif()

if(NOT TARGET lua)
    include(lua.cmake)
endif()

if(NOT TARGET fastgltf)
    add_subdirectory(fastgltf)
endif()

if(NOT TARGET glm)
    add_library(glm INTERFACE)
    target_include_directories(glm INTERFACE glm)
endif()
