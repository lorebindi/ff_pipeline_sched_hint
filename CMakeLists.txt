cmake_minimum_required(VERSION 3.18)
project(ff_pipeline_sched_hint CXX)

# Assicurati che il compilatore sia GCC e la versione sia adeguata
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "7.5.0")
        message(FATAL_ERROR "Insufficient gcc version (>= 7.5.0 required)")
    endif()
else()
    message(FATAL_ERROR "Compiler is not GCC")
endif()

# Imposta il tipo di build se non è specificato
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
    message(STATUS "Setting build type to '${CMAKE_BUILD_TYPE}' as none was specified")
else()
    message(STATUS "Build type is '${CMAKE_BUILD_TYPE}'")
endif()

# Configura il percorso della libreria FastFlow
set(FF_ROOT "/home/bindi/fastflow")
include_directories(${FF_ROOT})

# Imposta il C++ standard
set(CMAKE_CXX_STANDARD 20)

# Aggiungi il file sorgente del progetto
add_executable(test_ossched src/test_ossched_pipe.cpp)

# Linka eventuali librerie se necessario
# target_link_libraries(test_ossched ${FF_LIBRARIES})

# Se FastFlow utilizza librerie esterne aggiuntive, assicurati di linkarle
# esempio: target_link_libraries(test_ossched Threads::Threads)
