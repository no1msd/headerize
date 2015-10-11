find_package(Boost 1.54 COMPONENTS program_options regex REQUIRED)

include_directories(
    ${Boost_INCLUDE_DIR})

add_executable(headerize main.cpp)

target_link_libraries(headerize
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${Boost_REGEX_LIBRARY})
