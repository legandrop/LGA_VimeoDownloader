# VimeoDownloader

Una aplicación Qt/C++ multiplataforma para descargar videos de Vimeo y YouTube usando yt-dlp con sistema de cola de descargas.

## Características

- **Interfaz gráfica moderna** con tema oscuro (basado en el estilo de PipeSync)
- **Soporte para URLs de Vimeo y YouTube** con validación en tiempo real
- **Sistema de cola de descargas** - procesa múltiples descargas secuencialmente
- **Contador persistente** - rastrea descargas completadas durante la sesión
- **Descarga usando yt-dlp + ffmpeg** con credenciales de usuario
- **Detección automática de herramientas** - instala o actualiza yt-dlp y ffmpeg automáticamente
- **Configuración persistente** - guarda credenciales de Vimeo de forma segura
- **Log en tiempo real** - muestra todo el proceso de descarga
- **Control total de cola** - botón Cancel para resetear todo
- **Compatible con macOS y Windows**
- **Aplicación completamente portable**

## Requisitos

### Para usar la aplicación:
- **No se requiere instalación manual** - La aplicación incluye sus propios binarios
- yt-dlp y ffmpeg se descargan automáticamente al directorio de la aplicación
  ```bash
  # macOS y Windows (automático via app)
  # La app descarga yt-dlp y ffmpeg automáticamente a carpetas locales:
  # - macOS: toolsmac/ dentro del bundle de la aplicación
  # - Windows: tools/ en el directorio de la aplicación
  
  # Linux (manual)
  sudo apt install yt-dlp ffmpeg  # Ubuntu/Debian
  ```

### Para compilar:
- Qt 6.8.2
- CMake 3.16+
- C++17 compatible compiler
- macOS 10.15+ (para macOS)

## Compilación

### Windows
```batch
compilar.bat
```

### macOS
```bash
./compilar.sh
```

### Crear versión portable
```batch
# Windows
deploy.bat

# macOS
./deploy.sh
```

### Limpiar archivos de compilación
```batch
# Windows
limpiar.bat

# macOS
./limpiar.sh
```

## Estructura del Proyecto

```
VimeoDownloader/
├── CMakeLists.txt          # Configuración de CMake
├── compilar.sh             # Script de compilación para desarrollo
├── deploy.sh               # Script de despliegue para producción
├── limpiar.sh              # Script para limpiar la compilación
├── include/                # Archivos de cabecera (.h)
│   └── vimeodownloader/
├── src/                    # Código fuente (.cpp)
│   ├── core/               # Lógica de descarga y cola
│   ├── ui/                 # Interfaz de usuario
│   └── utils/              # Utilidades y gestión de herramientas
├── resources/              # Recursos (estilos, iconos)
│   ├── styles/
│   └── icons/
├── cmake/                  # Archivos de configuración CMake
├── tools/                  # Herramientas externas Windows (yt-dlp.exe, ffmpeg.exe)
├── toolsmac/               # Herramientas externas macOS (yt-dlp, ffmpeg)
├── build/                  # Carpeta de compilación (generada)
└── deploy/                 # Versión portable (generada)
```

## Uso

1. **Ejecuta la aplicación**
2. **Configura las credenciales de Vimeo**:
   - Ingresa tu usuario y contraseña de Vimeo
   - Haz clic en "Save" para guardar las credenciales
3. **Configura la carpeta de descarga**:
   - Ingresa la ruta de descarga o usa "Browse" para seleccionar
   - Haz clic en "Save" para guardar la configuración
4. **Instala herramientas** (si no están instaladas):
   - Haz clic en "Install Tools" para instalar yt-dlp y ffmpeg automáticamente
   - Si ya están instaladas, usa "Update Tools" para actualizarlas
5. **Descarga videos**:
   - Ingresa una URL válida de Vimeo o YouTube
   - Haz clic en "Download" para agregar a la cola (se habilita cuando todo está configurado)
   - Puedes agregar múltiples URLs - se procesarán secuencialmente
   - Monitorea el progreso en la sección Progress y Log
   - Usa "Cancel" para cancelar toda la cola y resetear contadores

### Configuración

La aplicación guarda las credenciales de Vimeo en:
- **macOS**: `~/Library/Application Support/LGA/VimeoDownloader/config.ini`
- **Windows**: `%APPDATA%\LGA\VimeoDownloader\config.ini`

### Interfaz de Usuario

La aplicación tiene 4 secciones principales:

1. **Video URL**: Campo de entrada para URL de Vimeo/YouTube + botón Download
2. **Progress**: Contador de cola (actual/total) + barra de progreso + botón Cancel
3. **Settings**: Configuración en 3 filas:
   - Fila 1: `Username | Password | Save`
   - Fila 2: `Download Folder | Browse`
   - Fila 3: `Tools Button` (Install/Update Tools)
4. **Log**: Terminal en tiempo real con todo el proceso de cola

### Comando equivalente

La aplicación ejecuta internamente:

**Para Vimeo (con credenciales):**
```bash
yt-dlp -u "usuario@email.com" -p "contraseña" --output "/ruta/descarga/%(title)s.%(ext)s" --format "bv*+ba/b" --ffmpeg-location "/ruta/a/ffmpeg" "URL_DE_VIMEO"
```

**Para YouTube (sin credenciales, con cookies automáticas):**
```bash
yt-dlp --output "/ruta/descarga/%(title)s.%(ext)s" --format "bv*+ba/b" --merge-output-format mp4 --ffmpeg-location "/ruta/a/ffmpeg" --cookies-from-browser chrome "URL_DE_YOUTUBE"
```

### Sistema de Cola de Descargas

La aplicación incluye un sistema de cola avanzado:

- **Cola secuencial**: Las descargas se procesan una por una
- **Contador persistente**: Formato (actual/total) que persiste durante la sesión
- **Control total**: Botón Cancel resetea toda la cola a (0/0)
- **Auto-inicio**: La cola comienza automáticamente al agregar elementos
- **Feedback visual**: Barra de progreso siempre visible, activa durante descargas

Para más detalles, consulta: [DOWNLOAD_QUEUE_SYSTEM.md](DOWNLOAD_QUEUE_SYSTEM.md)

## Notas de Desarrollo

- Basado en la estructura y estilo de PipeSync
- Usa el mismo tema oscuro y paleta de colores
- Preparado para funcionar tanto en macOS como Windows
- Arquitectura modular para fácil extensión

## Licencia

© 2024 LGA. Todos los derechos reservados.
