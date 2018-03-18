project(aionminer)
cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11") #  -Wall

## Enable solvers here
option(USE_CPU_TROMP "USE CPU_TROMP" OFF)
option(USE_CUDA_TROMP "USE CUDA_TROMP" ON)

## Add solvers here
if (USE_CPU_TROMP)
    add_definitions(-DUSE_CPU_TROMP)
    message("-- USE_CPU_TROMP DEFINED")
endif()
if (USE_CUDA_TROMP)
    add_definitions(-DUSE_CUDA_TROMP)
    message("-- USE_CUDA_TROMP DEFINED")
endif()

########
# LINUX
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64 -msse2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64 -msse2")
    
    #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64 -mavx")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64 -mavx")
    
    # optimizations
    add_definitions(-O3)
endif()

# Common
include_directories(${aionminer_SOURCE_DIR}/aionminer)

# BOOST
#find_package(Threads REQUIRED COMPONENTS)
# compile boost staticaly
set(Boost_USE_STATIC_LIBS ON)
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
#set(BUILD_SHARED_LIBRARIES OFF)
#set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
find_package(Boost REQUIRED COMPONENTS system log_setup log date_time filesystem thread program_options)

if (Boost_FOUND)
  # From the offical documentation:
  # Add include directories to the build. [...] If the SYSTEM option is given,
  # the compiler will be told the directories are meant as system include
  # directories on some platforms (signalling this setting might achieve effects
  # such as the compiler skipping warnings [...])."
  include_directories (SYSTEM ${Boost_INCLUDE_DIR})

  # From the offical documentation:
  # "Specify directories in which the linker will look for libraries. [...] Note
  # that this command is rarely necessary. Library locations returned by
  # find_package() and find_library() are absolute paths. Pass these absolute
  # library file paths directly to the target_link_libraries() command. CMake
  # will ensure the linker finds them."
  link_directories (${Boost_LIBRARY_DIRS})
else()
    message("Boost_FOUND NOT FOUND")
endif ()

include_directories(${CMAKE_CURRENT_BINARY_DIR}/../)

set(SOURCE_FILES
    # sources
    aionminer/amount.cpp
    aionminer/api.cpp
    aionminer/arith_uint256.cpp
    aionminer/crypto/sha256.cpp
    aionminer/json/json_spirit_reader.cpp
    aionminer/json/json_spirit_value.cpp
    aionminer/json/json_spirit_writer.cpp
    aionminer/libstratum/AionStratum.cpp
    aionminer/main.cpp
    aionminer/speed.cpp
    aionminer/uint256.cpp
    aionminer/utilstrencodings.cpp
    # headers
    aionminer/amount.h
    aionminer/api.hpp
    aionminer/arith_uint256.h
    aionminer/crypto/sha256.h
    aionminer/hash.h
    aionminer/json/json_spirit.h
    aionminer/json/json_spirit_error_position.h
    aionminer/json/json_spirit_reader.h
    aionminer/json/json_spirit_reader_template.h
    aionminer/json/json_spirit_stream_reader.h
    aionminer/json/json_spirit_utils.h
    aionminer/json/json_spirit_value.h
    aionminer/json/json_spirit_writer.h
    aionminer/json/json_spirit_writer_template.h
    aionminer/libstratum/StratumClient.cpp
    aionminer/libstratum/StratumClient.h
    aionminer/libstratum/AionStratum.cpp
    aionminer/libstratum/AionStratum.h
    aionminer/primitives/block.h
    aionminer/primitives/transaction.h
    aionminer/script/script.h
    aionminer/serialize.h
    aionminer/speed.hpp
    aionminer/streams.h
    aionminer/support/allocators/zeroafterfree.h
    aionminer/tinyformat.h
    aionminer/uint252.h
    aionminer/uint256.h
    aionminer/utilstrencodings.h
    aionminer/version.h
    aionminer/SolverStub.h # just a stub
    aionminer/AvailableSolvers.h
    aionminer/ISolver.h
    aionminer/Solver.h
    aionminer/MinerFactory.h
    aionminer/MinerFactory.cpp

    # make same path on windows
    #blake shared
    # src
    blake2/blake2bx.cpp
    # headers
    blake2/blake2.h
    blake2/blake2b-load-sse2.h
    blake2/blake2b-load-sse41.h
    blake2/blake2b-round.h
    blake2/blake2-config.h
    blake2/blake2-impl.h
    blake2/blake2-round.h
    )

#set(LIBS ${LIBS} ${Threads_LIBRARIES} ${Boost_LIBRARIES})
set(LIBS ${LIBS} ${Boost_LIBRARIES})

message("-- CXXFLAGS: ${CMAKE_CXX_FLAGS}")
message("-- LIBS: ${LIBS}")

if (USE_CPU_TROMP)
    add_subdirectory(cpu_tromp)
endif()
if (USE_CUDA_TROMP)
    add_subdirectory(cuda_tromp)
endif()

ADD_EXECUTABLE(${PROJECT_NAME} ${SOURCE_FILES})

#target_link_libraries(${PROJECT_NAME} ${LIBS} ${CUDA_LIBRARIES} )
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} ${LIBS} )

# link libs
if (USE_CPU_TROMP)
   target_link_libraries(${PROJECT_NAME} cpu_tromp)
endif()
if (USE_CUDA_TROMP)
   target_link_libraries(${PROJECT_NAME} cuda_tromp)
endif()

    
