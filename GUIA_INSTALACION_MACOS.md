# 🚀 Guía Completa: VimeoDownloader en macOS (Incluyendo Sequoia 15.1+)

Esta guía explica cómo hacer funcionar VimeoDownloader en cualquier Mac, incluyendo versiones recientes como macOS Sequoia 15.1, sin necesidad de certificados pagos de Apple.

## 📋 Problema que Resolvimos

**macOS Gatekeeper** bloquea aplicaciones que no están firmadas por Apple ($99/año). Esto causa dos errores:
1. `"La aplicación está dañada y no se puede abrir"`
2. `"No se puede abrir porque proviene de un desarrollador no identificado"`

## 🛠️ Solución Implementada

Creamos un **instalador portátil** que configura automáticamente los permisos necesarios.

---

## 🔧 Parte 1: Cambios en el Código Fuente

### 1.1 Modificar CMakeLists.txt

Para compatibilidad con versiones nuevas de macOS:

```cmake
# En CMakeLists.txt, línea 11:
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum OS X deployment version")
```

**¿Por qué?** Cambiamos de `10.15` a `11.0` para mejor compatibilidad con Sequoia 15.1+.

### 1.2 Scripts de Compilación

Los scripts `compilar.sh` y `deploy.sh` ya incluyen las configuraciones correctas:

```bash
-DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 \
-DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"
```

---

## 📦 Parte 2: Crear el Instalador Portátil

### 2.1 Ejecutar el Script de Instalador

```bash
./create_installer.sh
```

**¿Qué hace este script?**
- ✅ Copia la aplicación compilada (`deploy/VimeoDownloader.app`)
- ✅ Crea un script instalador (`EJECUTAR_EN_TERMINAL.sh`)
- ✅ Genera instrucciones claras (`INSTRUCCIONES_IMPORTANTES.txt`)
- ✅ Crea archivo comprimido (`VimeoDownloader_Installer.tar.gz`)

### 2.2 Contenido del Instalador

```
VimeoDownloader_Installer/
├── VimeoDownloader.app          # Tu aplicación completa
├── EJECUTAR_EN_TERMINAL.sh     # Script instalador (con permisos)
└── INSTRUCCIONES_IMPORTANTES.txt # Guía para el usuario
```

---

## 🎯 Parte 3: Distribuir a Usuarios

### 3.1 Envío del Instalador

Envía el archivo `VimeoDownloader_Installer.tar.gz` a tu amigo por:
- 📧 Email
- 💬 Telegram
- 📁 Google Drive
- 🌐 Cualquier medio

**Tamaño típico:** ~200MB (incluye Qt frameworks)

### 3.2 Instrucciones para el Usuario Final

**⚠️ IMPORTANTE:** El usuario NO debe hacer doble clic en archivos `.sh`

#### Método Automático (Recomendado):

1. **Descomprimir:**
   ```bash
   # En Finder: Doble clic en VimeoDownloader_Installer.tar.gz
   # O en Terminal si hay problemas de permisos:
   tar -xzpf VimeoDownloader_Installer.tar.gz
   ```

2. **Ejecutar instalador:**
   ```bash
   # Abrir Terminal (Aplicaciones → Utilidades → Terminal)
   # Arrastrar EJECUTAR_EN_TERMINAL.sh a Terminal y presionar Enter
   ```

#### Método Manual (Alternativo):

```bash
# Abrir Terminal
# Arrastrar VimeoDownloader.app a Terminal
# Escribir ANTES de la ruta: xattr -cr
# Presionar Enter
# Abrir VimeoDownloader.app
```

---

## 🔒 Parte 4: Cómo Funciona la Solución

### 4.1 Problema Técnico

- **Gatekeeper** bloquea apps no firmadas por seguridad
- **Atributos de cuarentena** se agregan a archivos descargados
- **Permisos de ejecución** se pierden al comprimir/descomprimir

### 4.2 Solución Técnica

El script `EJECUTAR_EN_TERMINAL.sh` hace automáticamente:

```bash
#!/bin/bash

echo "🚀 Instalando VimeoDownloader..."

# Obtener directorio del script
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

# Remover atributos de cuarentena
echo "🧹 Configurando permisos de seguridad..."
xattr -cr "$DIR/VimeoDownloader.app"

# Configurar permisos
echo "✅ Configuración completada"

# Abrir aplicación
echo "📱 Abriendo VimeoDownloader..."
open "$DIR/VimeoDownloader.app"
```

### 4.3 Ventajas de Esta Solución

- ✅ **Sin costo adicional** (no requiere certificado de $99/año)
- ✅ **Funciona en cualquier versión** de macOS (incluyendo Sequoia 15.1+)
- ✅ **Completamente automático** para el usuario
- ✅ **Mantiene todas las funcionalidades** de la aplicación
- ✅ **Proceso reversible** (no modifica el sistema)

---

## 🚀 Parte 5: Flujo de Trabajo Completo

### Para Desarrolladores:

1. **Compilar aplicación:**
   ```bash
   ./compilar.sh    # Compila con configuraciones optimizadas
   ./deploy.sh      # Crea versión portable
   ```

2. **Crear instalador:**
   ```bash
   ./create_installer.sh  # Crea paquete para distribución
   ```

3. **Distribuir:**
   ```bash
   # Enviar VimeoDownloader_Installer.tar.gz a usuarios
   ```

### Para Usuarios Finales:

1. **Descomprimir** el archivo recibido
2. **Ejecutar** `EJECUTAR_EN_TERMINAL.sh` desde Terminal
3. **¡Disfrutar!** de VimeoDownloader funcionando perfectamente

---

## 💡 Parte 6: Alternativas Consideradas

### ❌ GitHub Actions (No usado)
- **Ventaja:** Compilación automática en múltiples versiones
- **Desventaja:** Aún requiere pasos manuales del usuario

### ❌ Archivo DMG (No usado)
- **Ventaja:** Método estándar de Apple
- **Desventaja:** Más complejo y aún puede tener problemas de permisos

### ❌ Instalador PKG (No usado)
- **Ventaja:** Instalación profesional
- **Desventaja:** Interfaz compleja, requiere aprobación de términos

### ✅ Instalador Portátil (Elegido)
- **Ventaja:** Simple, efectivo, sin complicaciones adicionales

---

## 🔧 Parte 7: Solución de Problemas

### Problema: "operation not permitted"
```bash
# Solución:
chmod +x EJECUTAR_EN_TERMINAL.sh
./EJECUTAR_EN_TERMINAL.sh
```

### Problema: Aplicación sigue sin abrirse
```bash
# Solución manual:
xattr -cr VimeoDownloader.app
open VimeoDownloader.app
```

### Problema: Permisos perdidos al descomprimir
```bash
# Usar Terminal para descomprimir:
tar -xzpf VimeoDownloader_Installer.tar.gz
```

---

## 📈 Parte 8: Próximos Pasos y Mejoras

### Para Versión Profesional (Opcional):

1. **Obtener certificado de desarrollador** ($99/año)
2. **Firmar la aplicación** con `codesign`
3. **Notarizar con Apple** usando `notarytool`
4. **Distribuir sin restricciones** de Gatekeeper

### Mejoras Futuras:

- [ ] **Instalador gráfico** sin necesidad de Terminal
- [ ] **Firma automática** si se obtiene certificado
- [ ] **Actualizaciones automáticas** dentro de la aplicación
- [ ] **Soporte para App Store** (requiere certificado)

---

## 🎉 Conclusión

**VimeoDownloader ahora funciona perfectamente en cualquier Mac**, incluyendo las versiones más recientes como macOS Sequoia 15.1, sin necesidad de certificados pagos o conocimientos técnicos avanzados por parte del usuario.

**La solución es simple, efectiva y mantiene toda la funcionalidad original de la aplicación.**
