#    This file is part of Kuesa.
#
#    Copyright (C) 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
#    This file was auto-generated
#
#    Licensees holding valid proprietary KDAB Kuesa licenses may use this file in
#    accordance with the Kuesa Enterprise License Agreement provided with the Software in the
#    LICENSE.KUESA.ENTERPRISE file.
#
#    Contact info@kdab.com if any conditions of this licensing are not clear to you.
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU Affero General Public License as
#    published by the Free Software Foundation, either version 3 of the
#    License, or (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU Affero General Public License for more details.
#
#    You should have received a copy of the GNU Affero General Public License
#    along with this program.  If not, see <https://www.gnu.org/licenses/>.

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
  - Classes: 'Classes.md'
  - Examples: 'Examples.md'
  - License: 'license.md'
