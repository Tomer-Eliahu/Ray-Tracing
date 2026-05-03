cmake_minimum_required(VERSION 4.2.0)
project(RayTracing VERSION 0.1.0 LANGUAGES C)

#Use C23

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)


# I looked at https://github.com/RayTracing/raytracing.github.io/blob/release/CMakeLists.txt
# Just to see how to do the CMake stuff properly (and how to organize this repo).
# I did not look at the source code.

# Sources

set ( EXTERNAL
  src/external/stb_image.h
)

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

set ( SOURCE_NEXT_WEEK
  src/TheNextWeek/main.c
  # src/TheNextWeek/aabb.h
  # src/TheNextWeek/bvh.h
  # src/TheNextWeek/camera.h
  # src/TheNextWeek/color.h
  # src/TheNextWeek/constant_medium.h
  # src/TheNextWeek/hittable.h
  # src/TheNextWeek/hittable_list.h
  # src/TheNextWeek/interval.h
  # src/TheNextWeek/material.h
  # src/TheNextWeek/perlin.h
  # src/TheNextWeek/quad.h
  # src/TheNextWeek/ray.h
  # src/TheNextWeek/rtw_stb_image.h
  # src/TheNextWeek/rtweekend.h
  # src/TheNextWeek/sphere.h
  # src/TheNextWeek/texture.h
  # src/TheNextWeek/vec3.h
)

set ( SOURCE_REST_OF_YOUR_LIFE
  src/TheRestOfYourLife/main.c
  # src/TheRestOfYourLife/aabb.h
  # src/TheRestOfYourLife/camera.h
  # src/TheRestOfYourLife/color.h
  # src/TheRestOfYourLife/constant_medium.h
  # src/TheRestOfYourLife/hittable.h
  # src/TheRestOfYourLife/hittable_list.h
  # src/TheRestOfYourLife/interval.h
  # src/TheRestOfYourLife/material.h
  # src/TheRestOfYourLife/onb.h
  # src/TheRestOfYourLife/pdf.h
  # src/TheRestOfYourLife/perlin.h
  # src/TheRestOfYourLife/quad.h
  # src/TheRestOfYourLife/ray.h
  # src/TheRestOfYourLife/rtw_stb_image.h
  # src/TheRestOfYourLife/rtweekend.h
  # src/TheRestOfYourLife/sphere.h
  # src/TheRestOfYourLife/texture.h
  # src/TheRestOfYourLife/vec3.h
)

include_directories(src)


# Executables

add_executable(inOneWeekend      ${SOURCE_ONE_WEEKEND})
add_executable(theNextWeek       ${EXTERNAL} ${SOURCE_NEXT_WEEK})
add_executable(theRestOfYourLife ${EXTERNAL} ${SOURCE_REST_OF_YOUR_LIFE})
add_executable(cos_cubed         src/TheRestOfYourLife/cos_cubed.c         )
add_executable(cos_density       src/TheRestOfYourLife/cos_density.c       )
add_executable(integrate_x_sq    src/TheRestOfYourLife/integrate_x_sq.c    )
add_executable(pi                src/TheRestOfYourLife/pi.c                )
add_executable(estimate_halfway  src/TheRestOfYourLife/estimate_halfway.c  )
add_executable(sphere_importance src/TheRestOfYourLife/sphere_importance.c )
add_executable(sphere_plot       src/TheRestOfYourLife/sphere_plot.c       )