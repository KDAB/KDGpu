# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
site_name: KDGpu ${CMAKE_PROJECT_VERSION}
site_url: https://kdab.github.io/kdgpu/
site_description: KDGpu Documentation
site_author: Klarälvdalens Datakonsult AB (KDAB)
use_directory_urls: false

theme:
  name: 'material'
  features:
    - navigation.tabs
      # Navigation.tabs.sticky is an insiders only feature
      # Enable it so it might become available later
    - navigation.tabs.sticky
    - navigation.top
    - toc.integrate
  palette:
    scheme: kdab
  font:
    text: 'Open Sans'
  favicon: assets/assets_logo_tree.svg
  logo: assets/transparentWhiteKDAB.svg
copyright: "Copyright &copy; 2007-2023 Klar&auml;lvdalens Datakonsult AB (KDAB)<br>The Qt, C++ and OpenGL Experts<br><a href='https://www.kdab.com'>https://www.kdab.com/</a>"
extra:
  # Disabling the generator notice is currently a
  # Insiders only feature.
  # Will disappear if this feature becomes publicly available
  generator: false
  social:
    - icon: fontawesome/brands/twitter
      link: https://twitter.com/kdabqt
    - icon: fontawesome/brands/facebook
      link: https://facebook.com/kdabqt
    - icon: fontawesome/solid/envelope
      link: mailto:info@kdab.com
extra_css:
  - stylesheets/kdab.css
markdown_extensions:
  - pymdownx.highlight:
      linenums: true
  - pymdownx.superfences
  - toc:
      permalink: true
nav:
  - Home: 'index.md'
  - Getting Started:
    - Installation: 'INSTALL.md'
    - Using KDGpu: 'using-kdgpu/index.md'
  - Classes:
    - Public API: 'group__public.md'
    - Graphics Interface: 'group__api.md'
    - Vulkan Implementation: 'group__vulkan.md'
    - KDGpuExample Helper API: 'group__kdgpuexample.md'
  - Examples: 'kdgpu-examples.md'
  - License: 'license.md'
