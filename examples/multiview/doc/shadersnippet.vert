/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
//![1]
float rotationSign = gl_ViewIndex == 0 ? -1.0 : 1.0;
//![1]
//![2]
layout(push_constant) uniform PushConstants {
    int arrayLayer;
} pushConstants;

void main()
{
    vec3 color = texture(colorTexture, vec3(texCoord, pushConstants.arrayLayer)).rgb;
    fragColor = vec4(color, 1.0);
}
//![2]
//![3]
float rotationSign = gl_ViewIndex == 0 ? -1.0 : 1.0;
//![3]
