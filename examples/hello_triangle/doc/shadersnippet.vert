/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
//![1]
// actually located in /shaders/examples/hello_triangle/hello_triangle.vert.spv
layout(set = 0, binding = 0) uniform Entity
{
    mat4 modelMatrix;
}
entity;
//![1]
