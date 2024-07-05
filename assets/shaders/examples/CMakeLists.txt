# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
add_subdirectory(bindgroup_indexing)
add_subdirectory(bindgroup_partially_bound)
add_subdirectory(buffer_reference)
add_subdirectory(compute_particles)
add_subdirectory(compute_oit_transparency)
add_subdirectory(depth_texture_lookup)
add_subdirectory(dynamic_ubo)
add_subdirectory(hello_triangle_msaa)
add_subdirectory(hello_triangle_native)
add_subdirectory(hello_triangle_overlap)
add_subdirectory(hello_triangle)
add_subdirectory(hello_xr)
add_subdirectory(hello_xr_multiview)
add_subdirectory(multiview)
add_subdirectory(offscreen_rendering)
add_subdirectory(render_to_texture)
add_subdirectory(render_to_texture_subpass)
add_subdirectory(textured_quad)
add_subdirectory(hello_sphere_rt)
add_subdirectory(hello_triangle_rt)
add_subdirectory(hello_sphere_mesh)
add_subdirectory(hybrid_raster_rt)

if(KDGPU_HLSL_SUPPORT)
    add_subdirectory(hello_triangle_hlsl)
endif()
