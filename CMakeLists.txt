cmake_minimum_required(VERSION 3.16)

project(marketplace LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PISTACHE REQUIRED IMPORTED_TARGET libpistache)
pkg_check_modules(PQXX REQUIRED IMPORTED_TARGET libpqxx)

find_package(nlohmann_json CONFIG REQUIRED)
find_package(Threads REQUIRED)

file(GLOB GENERATED_API_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/generated/api/*.cpp"
)

file(GLOB GENERATED_MODEL_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/generated/model/*.cpp"
)

add_executable(marketplace
    src/main.cpp
    src/ProductApiImpl.cpp
    src/ProductRepository.cpp
    ${GENERATED_API_SOURCES}
    ${GENERATED_MODEL_SOURCES}
)

target_include_directories(marketplace PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/generated/api"
    "${CMAKE_CURRENT_SOURCE_DIR}/generated/model"
)

target_link_libraries(marketplace PRIVATE
    PkgConfig::PISTACHE
    PkgConfig::PQXX
    nlohmann_json::nlohmann_json
    Threads::Threads
)
