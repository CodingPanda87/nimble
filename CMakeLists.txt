cmake_minimum_required(VERSION 3.10)
project(nb)

find_package(Qt5 REQUIRED COMPONENTS Widgets)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/Zc:__cplusplus)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# ---------------- nb-lib --------------
file(GLOB_RECURSE NB_CPPS CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)
add_library(${PROJECT_NAME} SHARED
    ${NB_CPPS}
)
target_include_directories(${PROJECT_NAME}
          PRIVATE
            ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/3rd
            ${CMAKE_SOURCE_DIR}/_inc
          )
if(WIN32) 
    # 确保生成导入库 (.lib)
    set_target_properties(${PROJECT_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# Enable testing
enable_testing()

# Download and include GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://gitee.com/mirrors/googletest.git #https://github.com/google/googletest.git
  GIT_TAG release-1.11.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE) # 使用动态运行时库
FetchContent_MakeAvailable(googletest)

# Add your executable and test files
add_executable(${PROJECT_NAME}_test
          test/test_version.cpp
          test/test_itf.cpp
          test/test_log.cpp
          src/log.cpp
          test/test_threadpool.cpp
          test/test_event.cpp
          src/event.cpp
          test/test_memory.cpp
          test/test_iii_mempl.cpp
          test/test_mempool.cpp
          src/plugin_admin.cpp
          src/sys.cpp
          test/test_sys.cpp
          )
target_include_directories(${PROJECT_NAME}_test
          PRIVATE
            ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/3rd
          )

# add_executable(${PROJECT_NAME}_demo
#             demo.cpp
#           )

# Link with GoogleTest
target_link_libraries(${PROJECT_NAME}_test PRIVATE gtest_main)

# Add test
add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)

# Plugin test libraries
add_library(test_plugin1 MODULE
    integrate_test/plugin_test1.cpp
)
target_compile_definitions(test_plugin1
    PRIVATE 
        NB_EXPORT
)
target_include_directories(test_plugin1
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/3rd
)
set_target_properties(test_plugin1 PROPERTIES PREFIX "")
add_library(test_plugin2 MODULE
    integrate_test/plugin_test2.cpp
)
target_compile_definitions(test_plugin2
    PRIVATE 
        NB_EXPORT
)
target_include_directories(test_plugin2
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/3rd
)
set_target_properties(test_plugin2 PROPERTIES PREFIX "")

add_library(test_plugin_QtUI MODULE
    integrate_test/plugin_qtUI.cpp
)
target_compile_definitions(test_plugin_QtUI
    PRIVATE 
        NB_EXPORT
)
target_include_directories(test_plugin_QtUI
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/3rd
)
set_target_properties(test_plugin_QtUI PROPERTIES PREFIX "")
target_link_libraries(test_plugin_QtUI
    PRIVATE
    Qt5::Widgets
    ${PROJECT_NAME}
)

# ------------ nb starter ------------
add_executable(Starter
               ${CMAKE_SOURCE_DIR}/starter/console.cpp
               )
target_include_directories(Starter
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/3rd
)

target_link_libraries(Starter 
                        PRIVATE
                      ${PROJECT_NAME})

# Qt UI starter
add_executable(StarterQUI
               ${CMAKE_SOURCE_DIR}/starter/qtUI.cpp
               )

if(WIN32)
    set_target_properties(StarterQUI PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

target_include_directories(StarterQUI
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/3rd
)

target_link_libraries(StarterQUI
    PRIVATE
    Qt5::Widgets
    ${PROJECT_NAME}
)
