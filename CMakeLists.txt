cmake_minimum_required(VERSION 3.16)
project(VimeoDownloader VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configuración específica para macOS - Compatibilidad de versiones
if(APPLE)
    # Establecer la versión mínima de macOS soportada (macOS 10.15 Catalina)
    # Esto permite que la app funcione desde macOS 10.15 hasta las versiones más nuevas, compatible con Qt 6.8.2
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version")
    
    # Configurar arquitecturas - soporte para Intel y Apple Silicon
    if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "arm64")
        # Si estamos compilando en Apple Silicon, crear binario universal
        set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build architectures for OS X")
    else()
        # Si estamos compilando en Intel, solo Intel por ahora
        set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "Build architectures for OS X")
    endif()
    
    # Configuraciones adicionales para macOS
    set(CMAKE_MACOSX_RPATH ON)
    set(CMAKE_INSTALL_RPATH "@executable_path/../Frameworks")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
endif()

# Configuración de Debug
set(CMAKE_BUILD_TYPE Debug)
if(WIN32)
    if(MSVC)
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi")
    else()
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
    endif()
elseif(APPLE)
    # Configuración de debug para macOS
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Configuración para hacer la aplicación portable
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)

if(WIN32)
    set(CMAKE_PREFIX_PATH "C:/Qt/6.8.2/mingw_64")
else()
    set(CMAKE_PREFIX_PATH "$ENV{HOME}/Qt/6.8.2/macos")
endif()

# Encontrar paquetes de Qt
find_package(Qt6 COMPONENTS Core Gui Widgets Network REQUIRED)

# Encontrar paquetes específicos de Windows
if(WIN32)
    find_package(Qt6 COMPONENTS WinExtras QUIET)
    if(NOT Qt6WinExtras_FOUND)
        # En Qt6, WinExtras se movió a Core
        message(STATUS "Qt6WinExtras no encontrado, usando funcionalidades integradas en Qt6::Core")
    endif()
endif()

# Recursos
set(RESOURCES
    resources/resources.qrc
)

# Configuración del icono para Windows
if(WIN32)
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/LGA_VimeoDownloader.ico")
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/windows_icon.rc.in" "${CMAKE_CURRENT_BINARY_DIR}/windows_icon.rc" @ONLY)
    set(RESOURCES ${RESOURCES} "${CMAKE_CURRENT_BINARY_DIR}/windows_icon.rc")
endif()

# Archivos fuente
set(SOURCES
    src/main.cpp
    src/ui/mainwindow.cpp
    src/core/downloader.cpp
    src/core/downloadqueue.cpp
    src/utils/colorutils.cpp
    src/utils/toolsmanager.cpp
)

# Archivos de cabecera
set(HEADERS
    include/vimeodownloader/mainwindow.h
    include/vimeodownloader/downloader.h
    include/vimeodownloader/colorutils.h
    include/vimeodownloader/toolsmanager.h
    include/vimeodownloader/downloadqueue.h
    include/vimeodownloader/downloaditem.h
)

# Archivos de UI (comentado por ahora, no usamos archivos .ui)
# set(UI_FILES
#     src/ui/mainwindow.ui
# )

# Crear el ejecutable
add_executable(${PROJECT_NAME} 
    ${SOURCES} 
    ${HEADERS} 
    ${RESOURCES}
)

# Vincular con las bibliotecas de Qt
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)

# Vincular con bibliotecas específicas de Windows
if(WIN32)
    if(Qt6WinExtras_FOUND)
        target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::WinExtras)
    endif()
endif()

# Incluir directorios
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Configuración para Windows
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# Configuración para macOS
if(APPLE)
    # Configurar como aplicación de macOS
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Info.plist.in"
        MACOSX_BUNDLE_BUNDLE_NAME "VimeoDownloader"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
        MACOSX_BUNDLE_IDENTIFIER "com.lga.vimeodownloader"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.lga.vimeodownloader"
        MACOSX_BUNDLE_EXECUTABLE_NAME "VimeoDownloader"
        MACOSX_BUNDLE_COPYRIGHT "© 2024 LGA. Todos los derechos reservados."
        # Configurar versiones de macOS soportadas
        MACOSX_BUNDLE_INFO_STRING "VimeoDownloader ${PROJECT_VERSION}, Compatible con macOS 10.15+"
    )
    
    # Si existe un icono para macOS, configurarlo
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/LGA_VimeoDownloader.icns")
        set_target_properties(${PROJECT_NAME} PROPERTIES
            MACOSX_BUNDLE_ICON_FILE "LGA_VimeoDownloader.icns"
        )
        # Copiar el icono al bundle
        set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/LGA_VimeoDownloader.icns"
            PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
        target_sources(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/LGA_VimeoDownloader.icns")
    endif()
endif()

# Instalación
if(WIN32)
    # Instalación para Windows
    install(TARGETS ${PROJECT_NAME} DESTINATION bin)
    install(FILES $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> DESTINATION bin)
    
elseif(APPLE)
    # Instalación para macOS
    install(TARGETS ${PROJECT_NAME} DESTINATION .)
    
else()
    # Instalación para Linux (si es necesario en el futuro)
    install(TARGETS ${PROJECT_NAME} DESTINATION bin)
endif()
