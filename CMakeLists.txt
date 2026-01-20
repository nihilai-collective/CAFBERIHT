# Copyright 2026 Nihilai Collective Corp
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
# http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License

cmake_minimum_required(VERSION 3.20)

project(CAFBERIHT 
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION "Constexpr Aggregated and Filtered Bases for Efficient Runtime Iteration of Heterogeneous Types"
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (NOT DEFINED CAFBERIHT_WIDTH)
	set(CAFBERIHT_WIDTH 100 CACHE STRING "Width of the cafberiht struct" FORCE)
endif()

option(CAFBERIHT_GENERATE_ASSEMBLY "Generate assembly output for examples" ON)
option(CAFBERIHT_VERBOSE_BUILD "Enable verbose build output" OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(MSVC_COMPILE_OPTIONS 
    /Ob3 /GS- /O2 
    /std:c++latest
    /EHsc
)

set(CLANG_COMPILE_OPTIONS 
    $<$<CONFIG:Release>:-O3>
    -fbracket-depth=${CAFBERIHT_WIDTH}
    -ftemplate-depth=${CAFBERIHT_WIDTH}
)

set(GNU_COMPILE_OPTIONS     
    $<$<CONFIG:Release>:-O3>
)

set(ASSEMBLY_OUTPUT_DIR "${CMAKE_BINARY_DIR}/assembly_output")
file(MAKE_DIRECTORY ${ASSEMBLY_OUTPUT_DIR})
        
get_filename_component(EXAMPLE_NAME ./src/cafberiht_example.cpp NAME_WE)
            
add_executable(cafberiht ${CMAKE_SOURCE_DIR}/src/cafberiht_example.cpp)

target_compile_options(cafberiht PUBLIC
    $<$<CXX_COMPILER_ID:Clang>:${CLANG_COMPILE_OPTIONS}>
    $<$<CXX_COMPILER_ID:MSVC>:${MSVC_COMPILE_OPTIONS}>
    $<$<CXX_COMPILER_ID:GNU>:${GNU_COMPILE_OPTIONS}>            
)

target_compile_definitions(cafberiht PUBLIC
    CAFBERIHT_WIDTH=${CAFBERIHT_WIDTH}
)

install(FILES $<TARGET_FILE:cafberiht>
    DESTINATION lib/cmake/CAFBERIHT
)
            
set_target_properties(cafberiht PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
)
            
if(CAFBERIHT_VERBOSE_BUILD)
    message(STATUS "Configured example: cafberiht")
endif()

message(${ASSEMBLY_OUTPUT_DIR})

if(CAFBERIHT_GENERATE_ASSEMBLY)
    set(ASSEMBLY_FILE "${ASSEMBLY_OUTPUT_DIR}/cafberiht.s")
                
    if(MSVC)
        add_custom_command(
            TARGET cafberiht POST_BUILD
            COMMAND ${CMAKE_CXX_COMPILER}
                /c
                /FAcs
                /Fa${ASSEMBLY_FILE}
                ${CMAKE_SOURCE_DIR}/src/cafberiht_example.cpp
                /I${CMAKE_CURRENT_SOURCE_DIR}/include
		        -DCAFBERIHT_WIDTH=${CAFBERIHT_WIDTH}
                ${MSVC_COMPILE_OPTIONS}
            BYPRODUCTS ${ASSEMBLY_FILE}
            COMMENT "Generating assembly for cafberiht (MSVC)\nCafberiht Width: ${CAFBERIHT_WIDTH}"
            VERBATIM
        )
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_custom_command(
            TARGET cafberiht POST_BUILD
            COMMAND ${CMAKE_CXX_COMPILER}
                -S
                -masm=intel
                -fverbose-asm
                -O3
                -fbracket-depth=${CAFBERIHT_WIDTH}
                -ftemplate-depth=${CAFBERIHT_WIDTH}
                -std=c++23
		        -DCAFBERIHT_WIDTH=${CAFBERIHT_WIDTH}
                -I${CMAKE_CURRENT_SOURCE_DIR}/include
                ${CMAKE_SOURCE_DIR}/src/cafberiht_example.cpp
                -o ${ASSEMBLY_FILE}
            BYPRODUCTS ${ASSEMBLY_FILE}
            COMMENT "Generating assembly for cafberiht (Clang)\nCafberiht Width: ${CAFBERIHT_WIDTH}"
            VERBATIM
        )
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_custom_command(
            TARGET cafberiht POST_BUILD
            COMMAND ${CMAKE_CXX_COMPILER}
                -S
                -masm=intel
                -fverbose-asm
                -O3
                -std=c++23
		        -DCAFBERIHT_WIDTH=${CAFBERIHT_WIDTH}
                -I${CMAKE_CURRENT_SOURCE_DIR}/include
                ${CMAKE_SOURCE_DIR}/src/cafberiht_example.cpp
                -o ${ASSEMBLY_FILE}
            BYPRODUCTS ${ASSEMBLY_FILE}
            COMMENT "Generating assembly for cafberiht (GCC)\nCafberiht Width: ${CAFBERIHT_WIDTH}"
            VERBATIM
        )
    endif()
                
    if(CAFBERIHT_VERBOSE_BUILD)
        message(STATUS "  Assembly output: ${ASSEMBLY_FILE}")
    endif()
endif()
        
message(STATUS "CAFBERIHT Examples configured:")
message(STATUS "  Build directory: ${CMAKE_BINARY_DIR}/bin")
if(CAFBERIHT_GENERATE_ASSEMBLY)
    message(STATUS "  Assembly output directory: ${ASSEMBLY_OUTPUT_DIR}")
endif()

message(STATUS "")
message(STATUS "CAFBERIHT Configuration Summary")
message(STATUS "================================")
message(STATUS "Version:          ${PROJECT_VERSION}")
message(STATUS "Build Type:       ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler:         ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C++ Standard:     C++${CMAKE_CXX_STANDARD}")
message(STATUS "Generate ASM:     ${CAFBERIHT_GENERATE_ASSEMBLY}")
message(STATUS "Install Prefix:   ${CMAKE_INSTALL_PREFIX}")
message(STATUS "================================")
message(STATUS "")