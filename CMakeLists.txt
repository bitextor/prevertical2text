cmake_minimum_required(VERSION 2.8.12)

project(prevertical2text)

set(CMAKE_CXX_STANDARD 11)


set(CMAKE_CXX_FLAGS "-Wall -Wextra")
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I /usr/local/opt/icu4c/include")
endif ()
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Ofast")
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()
add_definitions(-DBOOST_LOG_DYN_LINK)

find_package(Boost 1.71 COMPONENTS program_options log log_setup REQUIRED)

# compile executable into bin/
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

# compile libraries into lib/
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# define where include files are
include_directories(
        ${PROJECT_SOURCE_DIR}
        ${Boost_INCLUDE_DIR}
)

# process subdirectories
add_subdirectory(src)

# kpu/preprocess dependency:
if (NOT PREPROCESS_PATH)
    # if preprocess_path is not defined, use the one in prevertical2text folder
    set(PREPROCESS_PATH "${PROJECT_SOURCE_DIR}/preprocess")
endif()

if (NOT SKIP_PREPROCESS_BUILD)
    add_subdirectory(${PREPROCESS_PATH} ${PREPROCESS_PATH}/bin)
endif()

target_include_directories(prevertical2text_lib PUBLIC ${PREPROCESS_PATH})
#

# define executables
add_executable(prevertical2text prevertical2text_main.cc)
target_link_libraries(prevertical2text
    prevertical2text_lib
    ${Boost_LIBRARIES}
)

include(GNUInstallDirs)

install(TARGETS prevertical2text
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
