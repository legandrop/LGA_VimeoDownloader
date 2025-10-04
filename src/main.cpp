#include "vimeodownloader/mainwindow.h"
#include "vimeodownloader/colorutils.h"

#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QIcon>
#include <QFontDatabase>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Configurar información de la aplicación
    app.setApplicationName("VimeoDownloader");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("LGA");
    app.setOrganizationDomain("lga.com");
    
    // Cargar fuentes Inter si están disponibles
    QDir fontsDir(":/fonts");
    if (fontsDir.exists()) {
        QStringList fontFiles = fontsDir.entryList(QStringList() << "*.ttf", QDir::Files);
        for (const QString &fontFile : fontFiles) {
            QFontDatabase::addApplicationFont(":/fonts/" + fontFile);
        }
    }
    
    // Cargar y aplicar el tema oscuro
    QFile styleFile(":/styles/dark_theme.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&styleFile);
        QString styleSheet = stream.readAll();
        
        // Reemplazar variables de color con valores reales
        styleSheet.replace("bg_principal", ColorUtils::BG_PRINCIPAL);
        styleSheet.replace("txt_principal", ColorUtils::TXT_PRINCIPAL);
        styleSheet.replace("boton_gris_oscuro", ColorUtils::BOTON_GRIS_OSCURO);
        styleSheet.replace("boton_gris_oscu_hover", ColorUtils::BOTON_GRIS_OSCU_HOVER);
        styleSheet.replace("border_principal", ColorUtils::BORDER_PRINCIPAL);
        
        app.setStyleSheet(styleSheet);
        styleFile.close();
    } else {
        qDebug() << "No se pudo cargar el archivo de estilos";
        // Aplicar estilo básico como fallback
        app.setStyleSheet(ColorUtils::getStyleSheet());
    }
    
    // Crear y mostrar la ventana principal
    MainWindow window;
    window.show();
    
    return app.exec();
}
