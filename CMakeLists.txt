# Generated CMake project file for the TELESCREEN RP5 teaching lab

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(telescreen C)

# Warnings and hardening for a security-teaching build. These flags are part of the
# lesson: a device that ships without them is a device that ships backdoors.
add_compile_options(
        -Wall
        -Wextra
        -Wshadow
        -Wconversion
        -Wformat=2
        -fstack-protector-strong
        -D_FORTIFY_SOURCE=2
        -O2
)

# The application links OpenSSL for AES-256-GCM and ChaCha20-Poly1305.
find_package(OpenSSL REQUIRED)

add_executable(telescreen
        src/main.c
        src/teled.c
        src/aead.c
        src/partition.c
        src/env.c
        src/container.c
        src/jffs2.c
        src/crc.c
        src/beacon.c
        src/collector.c
        src/kex.c
        src/identity.c
        src/camera.c
)

target_include_directories(telescreen PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(telescreen
        OpenSSL::Crypto
)

# The lab's image tooling is plain Python; make the targets available from CMake.
find_package(Python3 COMPONENTS Interpreter)

add_custom_target(verify
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/scripts/verify_telescreen.py
                --image ${CMAKE_CURRENT_LIST_DIR}/images/full.img
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        COMMENT "Verifying the four-partition image layout"
)
