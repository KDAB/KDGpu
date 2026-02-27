/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2026 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

[[vk::binding(0)]]
cbuffer Transform
{
    float4x4 model;
};

struct VSInput
{
    float3 position : POSITION;
    float3 color : COLOR0;
};

struct VSOutput
{
    float4 position : SV_Position;
    float3 color : COLOR0;
};

[shader("vertex")]
VSOutput main(VSInput input)
{
    VSOutput output;
    output.position = mul(model, float4(input.position, 1.0f));
    output.color = input.color;
    return output;
}
