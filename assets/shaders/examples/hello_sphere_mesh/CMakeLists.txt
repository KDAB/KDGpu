# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
kdgpu_compilemeshtaskshader(KDGpu_HelloMeshShaderMesh hello_sphere_mesh.mesh hello_sphere_mesh.mesh.spv)
kdgpu_compileshader(KDGpu_HelloMeshShaderFrag hello_sphere_mesh.frag hello_sphere_mesh.frag.spv)
