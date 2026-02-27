/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2026 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

struct PSInput
{
    float3 color : COLOR0;
};

[shader("fragment")]
float4 main(PSInput input) : SV_Target
{
    return float4(input.color, 1.0f);
}
