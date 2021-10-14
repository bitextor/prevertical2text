cmake_minimum_required(VERSION 2.8.12)

project(prevertical2text_src)

find_package(ZLIB 1.2.11 REQUIRED)
find_package( Boost 1.71 COMPONENTS locale iostreams filesystem log regex REQUIRED )

include_directories(
    ${ZLIB_INCLUDE_DIR}
    ${Boost_INCLUDE_DIR}
)

add_library(prevertical2text_lib
        html.cc
        bilangwriter.cc
    xh_scanner.cc
    entities.cc
)


target_link_libraries(prevertical2text_lib
    base64
    preprocess_util
    ${Boost_LIBRARIES}
    ${ZLIB_LIBRARIES}
)

