cmake_minimum_required(VERSION 4.2.0)
project(RayTracing VERSION 0.1.0 LANGUAGES C)

#Use C23

set(C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED True)


# I looked at https://github.com/RayTracing/raytracing.github.io/blob/release/CMakeLists.txt
# Just to see how to do the CMake stuff properly (and how to organize this repo).
# I did not look at the source code.

# Sources

set ( SOURCE_ONE_WEEKEND
  src/InOneWeekend/main.c
#   src/InOneWeekend/camera.h
#   src/InOneWeekend/color.h
#   src/InOneWeekend/hittable.h
#   src/InOneWeekend/hittable_list.h
#   src/InOneWeekend/interval.h
#   src/InOneWeekend/material.h
#   src/InOneWeekend/ray.h
#   src/InOneWeekend/rtweekend.h
#   src/InOneWeekend/sphere.h
#   src/InOneWeekend/vec3.h
)

include_directories(src)


# Executables

add_executable(inOneWeekend      ${SOURCE_ONE_WEEKEND})