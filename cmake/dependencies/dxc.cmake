# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2022-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

cmake_policy(SET CMP0135 NEW)

message("-- Loading DXC compiler...")

if(NOT TARGET dxc::dxc)
    if(WIN32)
        set(DXC_FILE
            https://ci.appveyor.com/api/projects/dnovillo/directxshadercompiler/artifacts/build%2FRelease%2Fdxc-artifacts.zip?branch=main&pr=false&job=image%3A%20Visual%20Studio%202022
        )
    elseif(NOT APPLE)
        set(DXC_FILE
            https://ci.appveyor.com/api/projects/dnovillo/directxshadercompiler/artifacts/build%2Fdxc-artifacts.tar.gz?branch=main&pr=false&job=image%3A%20Ubuntu
        )
    endif()

    if(DXC_FILE)
        FetchContent_Declare(dxc URL ${DXC_FILE})
        FetchContent_MakeAvailable(dxc)

        if(WIN32)
            set(DXC_EXECUTABLE
                ${dxc_SOURCE_DIR}/bin/dxc.exe
                CACHE INTERNAL ""
            )
        elseif(NOT APPLE)
            set(DXC_EXECUTABLE
                ${dxc_SOURCE_DIR}/bin/dxc
                CACHE INTERNAL ""
            )
        endif()
    endif()
endif()
