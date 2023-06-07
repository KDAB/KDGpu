/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
layout(location = 0) in vec4 vertexPos;
layout(location = 1) in vec4 vertexCol;

layout(location = 0) out vec4 color;

//![2]
out gl_PerVertex
{
    vec4 gl_Position;
    float gl_PointSize;
};

layout(set = 1, binding = 0) uniform Transform
{
    mat4 proj;
}
transform;

void main()
{
    color = vertexCol;
    gl_PointSize = 16.0;
    gl_Position = transform.proj * vertexPos;
}
//![2]
