# VimeoDownloader

Una aplicación Qt/C++ multiplataforma para descargar videos de Vimeo y YouTube usando yt-dlp con sistema de cola de descargas. **Completamente portable** - incluye todas las herramientas necesarias.

## Características

- **Interfaz gráfica moderna** con tema oscuro (basado en el estilo de PipeSync)
- **Soporte para URLs de Vimeo y YouTube** con validación en tiempo real
- **Sistema de cola de descargas** - procesa múltiples descargas secuencialmente
- **Contador persistente** - rastrea descargas completadas durante la sesión
- **Descarga usando yt-dlp + ffmpeg** con credenciales de usuario
- **Herramientas locales** - yt-dlp y ffmpeg incluidos en la aplicación (no requiere instalación global)
- **Formatos compatibles con QuickTime** - descarga directamente en MP4 con H.264 + AAC
- **Configuración persistente** - guarda credenciales de Vimeo de forma segura
- **Log en tiempo real** - muestra todo el proceso de descarga
- **Control total de cola** - botón Cancel para resetear todo
- **Compatible con macOS y Windows**
- **Aplicación completamente portable** - funciona sin instalación previa

## Requisitos

### Para usar la aplicación:
- **Completamente autónoma** - Incluye yt-dlp y ffmpeg en el paquete de la aplicación
- **No requiere instalación previa** - Las herramientas se descargan automáticamente si es necesario
  ```bash
  # Las herramientas se incluyen automáticamente:
  # - macOS: toolsmac/ dentro del bundle (.app/Contents/MacOS/toolsmac/)
  # - Windows: tools/ en el directorio de la aplicación

  # El botón "Update Tools" descarga la versión más reciente desde GitHub
  # El botón "Install Tools" instala herramientas iniciales si no existen
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

La aplicación ejecuta internamente comandos optimizados:

**Para Vimeo (con credenciales):**
```bash
yt-dlp -u "usuario@email.com" -p "contraseña" --output "/ruta/descarga/%(title).200s.%(ext)s" --restrict-filenames --format "bestvideo[ext=mp4]+bestaudio[ext=m4a]/best[ext=mp4]" --ffmpeg-location "/ruta/a/ffmpeg" "URL_DE_VIMEO"
```

**Para YouTube (sin credenciales, con cookies automáticas):**
```bash
yt-dlp --output "/ruta/descarga/%(title).200s.%(ext)s" --restrict-filenames --format "bestvideo[ext=mp4]+bestaudio[ext=m4a]/best[ext=mp4]" --ffmpeg-location "/ruta/a/ffmpeg" --cookies-from-browser chrome "URL_DE_YOUTUBE"
```

### Características técnicas

- **Formatos QuickTime**: Descarga directamente en MP4 con H.264 + AAC (compatible con QuickTime, Preview, VLC)
- **Nombres seguros**: `--restrict-filenames` convierte caracteres especiales a ASCII seguro
- **Herramientas locales**: yt-dlp y ffmpeg incluidos en la aplicación (toolsmac/ en macOS, tools/ en Windows)
- **Sin recodificación**: Los videos se descargan directamente en formatos compatibles

### Sistema de Cola de Descargas

La aplicación incluye un sistema de cola:

- **Cola secuencial**: Las descargas se procesan una por una
- **Contador persistente**: Formato (actual/total) que persiste durante la sesión
- **Control total**: Botón Cancel resetea toda la cola a (0/0)
- **Auto-inicio**: La cola comienza automáticamente al agregar elementos
- **Feedback visual**: Barra de progreso siempre visible, activa durante descargas

Para más detalles, consulta: [DOWNLOAD_QUEUE_SYSTEM.md](DOWNLOAD_QUEUE_SYSTEM.md)



