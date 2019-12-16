cmake_minimum_required(VERSION 3.14)
project(sigbus-reproduction)

set(CMAKE_CXX_STANDARD 14)

set(BOOST_LIBS
        system
        filesystem
        )

find_package(Boost COMPONENTS ${BOOST_LIBS} REQUIRED)

add_executable(main src/main/cpp/main.cpp)

find_package(JNI)

if (JNI_FOUND)
    include_directories(SYSTEM ${JAVA_INCLUDE_PATH})
    include_directories(SYSTEM ${JAVA_INCLUDE_PATH2})
endif()

if (JNI_FOUND)
    target_link_libraries(main ${JAVA_JVM_LIBRARY})
endif()

target_link_libraries(
        main
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
)

set_target_properties(main PROPERTIES PREFIX "../out/")

ADD_CUSTOM_TARGET(build_jar ALL
        COMMAND gradle jar
        WORKING_DIRECTORY ../
        COMMENT "building sigbus-reproduction.jar...")

add_dependencies(main build_jar)
