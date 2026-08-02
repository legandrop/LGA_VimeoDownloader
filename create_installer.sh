#!/bin/bash

echo "📦 Creando instalador de VideoDownloader..."

# Crear directorio del instalador
INSTALLER_DIR="VideoDownloader_Installer"
rm -rf "$INSTALLER_DIR"
mkdir -p "$INSTALLER_DIR"

# Copiar la aplicación (usando rsync para evitar problemas de metadatos)
if [ -d "deploy/VideoDownloader.app" ]; then
    echo "📋 Copiando aplicación (esto puede tardar un momento)..."
    rsync -a --exclude='*.dSYM' deploy/VideoDownloader.app "$INSTALLER_DIR/"
    echo "✅ Aplicación copiada al instalador"
    
    # Verificar tamaños
    ORIGINAL_SIZE=$(du -sh deploy/VideoDownloader.app | cut -f1)
    COPIED_SIZE=$(du -sh "$INSTALLER_DIR/VideoDownloader.app" | cut -f1)
    echo "📊 Tamaño original: $ORIGINAL_SIZE, Tamaño copiado: $COPIED_SIZE"
else
    echo "❌ No se encontró deploy/VideoDownloader.app"
    echo "   Ejecuta primero ./deploy.sh"
    exit 1
fi

# Crear script de instalación con nombre descriptivo
cat > "$INSTALLER_DIR/EJECUTAR_EN_TERMINAL.sh" << 'EOL'
#!/bin/bash

echo "🚀 Instalando VideoDownloader..."
echo ""

# Obtener el directorio del script
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

# Remover atributos de cuarentena
echo "🧹 Configurando permisos de seguridad..."
xattr -cr "$DIR/VideoDownloader.app"

if [ $? -eq 0 ]; then
    echo "✅ Configuración completada"
    echo ""
    echo "📱 Abriendo VideoDownloader..."
    open "$DIR/VideoDownloader.app"
    echo ""
    echo "✨ ¡Instalación completada!"
    echo "   Puedes mover VideoDownloader.app a tu carpeta Aplicaciones"
    echo ""
    echo "Presiona cualquier tecla para cerrar..."
    read -n 1
else
    echo "❌ Error en la configuración"
    echo "   Contacta al desarrollador"
    echo ""
    echo "Presiona cualquier tecla para cerrar..."
    read -n 1
fi
EOL

# Hacer ejecutable el instalador
chmod +x "$INSTALLER_DIR/EJECUTAR_EN_TERMINAL.sh"

# Crear instrucciones súper claras
cat > "$INSTALLER_DIR/INSTRUCCIONES_IMPORTANTES.txt" << 'EOL'
🚀 VideoDownloader - INSTRUCCIONES DE INSTALACIÓN

⚠️  IMPORTANTE: NO hagas doble clic en el archivo .sh
    Se abrirá en editor de texto y no funcionará.

📦 SI TIENES PROBLEMAS DE PERMISOS:
   Descomprime usando Terminal: tar -xzpf VideoDownloader_Installer.tar.gz

✅ INSTALACIÓN CORRECTA (2 pasos):

   1️⃣ Abre Terminal (Aplicaciones → Utilidades → Terminal)
   
   2️⃣ Arrastra el archivo "EJECUTAR_EN_TERMINAL.sh" a la ventana de Terminal
       y presiona Enter

   El script configurará automáticamente la aplicación y después se puede copiar  a la carpeta Aplicaciones.

💡 ¿Por qué estos pasos?
   macOS bloquea aplicaciones independientes que no están firmadas por Apple.
   Estos pasos le dicen a macOS que confíe en la aplicación.

EOL

# Crear archivo comprimido
echo ""
echo "📦 Creando archivo comprimido..."
tar -czpf "VideoDownloader_Installer.tar.gz" "$INSTALLER_DIR"

echo ""
echo "✅ Instalador creado exitosamente:"
echo "   📁 Carpeta: $INSTALLER_DIR/"
echo "   📦 Archivo: VideoDownloader_Installer.tar.gz"
echo ""
echo "🎯 Para distribuir:"
echo "   Envía el archivo .tar.gz a tus usuarios"
echo "   Ellos solo necesitan descomprimirlo y ejecutar el instalador"
