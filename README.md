# VimeoDownloader

Una aplicación Qt/C++ multiplataforma para descargar videos de Vimeo usando yt-dlp.

## Características

- **Interfaz gráfica moderna** con tema oscuro (basado en el estilo de PipeSync)
- **Soporte para URLs de Vimeo** con validación en tiempo real
- **Descarga usando yt-dlp** con credenciales de usuario
- **Detección automática de yt-dlp** - instala o actualiza automáticamente
- **Configuración persistente** - guarda credenciales de Vimeo de forma segura
- **Log en tiempo real** - muestra todo el proceso de descarga
- **Compatible con macOS y Windows**
- **Aplicación completamente portable**

## Requisitos

### Para usar la aplicación:
- yt-dlp instalado en el sistema
  ```bash
  pip install yt-dlp
  ```

### Para compilar:
- Qt 6.8.2
- CMake 3.16+
- C++17 compatible compiler
- macOS 10.15+ (para macOS)

## Compilación

### macOS
```bash
./compilar.sh
```

### Crear versión portable
```bash
./deploy.sh
```

### Limpiar archivos de compilación
```bash
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
│   ├── core/               # Lógica de descarga
│   ├── ui/                 # Interfaz de usuario
│   └── utils/              # Utilidades
├── resources/              # Recursos (estilos, iconos)
│   ├── styles/
│   └── icons/
├── cmake/                  # Archivos de configuración CMake
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
4. **Instala yt-dlp** (si no está instalado):
   - Haz clic en "Install yt-dlp" para instalarlo automáticamente
   - Si ya está instalado, usa "Update yt-dlp" para actualizarlo
5. **Descarga videos**:
   - Ingresa una URL válida de Vimeo
   - Haz clic en "Download" (se habilita cuando todo está configurado)
   - Monitorea el progreso en la sección de Log

### Configuración

La aplicación guarda las credenciales de Vimeo en:
- **macOS**: `~/Library/Application Support/LGA/VimeoDownloader/config.ini`
- **Windows**: `%APPDATA%\LGA\VimeoDownloader\config.ini`

### Interfaz de Usuario

La aplicación tiene 4 secciones principales:

1. **Video URL**: Campo de entrada para URL de Vimeo + botón Download
2. **Progress**: Barra de progreso y estado actual
3. **Download Log**: Terminal en tiempo real con todo el proceso
4. **Settings**: Configuración en 2 filas:
   - Fila 1: `Username | Password | Save`
   - Fila 2: `Download Folder | Browse | Save | yt-dlp Button`

### Comando equivalente

La aplicación ejecuta internamente:
```bash
yt-dlp -u "usuario@email.com" -p "contraseña" --output "/ruta/descarga/%(title)s.%(ext)s" --format "best[height<=720]" "URL_DE_VIMEO"
```

### Actualización de yt-dlp

- Si yt-dlp está instalado: usa `yt-dlp -U` para actualizar
- Si no está instalado: usa `pip install yt-dlp` para instalar

## Notas de Desarrollo

- Basado en la estructura y estilo de PipeSync
- Usa el mismo tema oscuro y paleta de colores
- Preparado para funcionar tanto en macOS como Windows
- Arquitectura modular para fácil extensión

## Licencia

© 2024 LGA. Todos los derechos reservados.
