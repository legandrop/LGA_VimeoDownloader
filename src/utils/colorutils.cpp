#include "vimeodownloader/colorutils.h"

// Definición de colores basados en el tema actualizado
const QString ColorUtils::BG_PRINCIPAL = "#1d1d1d";
const QString ColorUtils::TXT_PRINCIPAL = "#b2b2b2";
const QString ColorUtils::BOTON_GRIS_OSCURO = "#443a91";
const QString ColorUtils::BOTON_GRIS_OSCU_HOVER = "#774dcb";
const QString ColorUtils::BORDER_PRINCIPAL = "#555555";
const QString ColorUtils::ACCENT_COLOR = "#443a91";

QString ColorUtils::getStyleSheet()
{
    return QString(R"(
        QWidget {
            background-color: %1;
            color: %2;
            font-family: "Inter", Arial, Helvetica, sans-serif;
            font-size: 14px;
            font-weight: 400;
        }
        
        QPushButton {
            background-color: %3;
            color: %2;
            border: none;
            padding: 8px 16px;
            border-radius: 6px;
            font-weight: 500;
            min-height: 20px;
        }
        
        QPushButton:hover {
            background-color: %4;
        }
        
        QPushButton[class="primary"] {
            background-color: %6;
        }
        
        QPushButton[class="primary"]:hover {
            background-color: %4;
        }
        
        QLineEdit {
            background-color: %1;
            color: %2;
            border: 1px solid %5;
            border-radius: 4px;
            padding: 8px 12px;
        }
        
        QLineEdit:focus {
            border-color: %6;
            background-color: #2a2a2a;
        }
        
        QGroupBox {
            color: %2;
            border: 1px solid %5;
            border-radius: 6px;
            margin-top: 10px;
            padding-top: 10px;
            font-weight: 500;
        }
        
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
            color: %2;
        }
        
        QProgressBar {
            border: 1px solid %5;
            border-radius: 4px;
            text-align: center;
            background-color: %1;
            color: %2;
        }
        
        QProgressBar::chunk {
            background-color: %6;
            border-radius: 3px;
        }
        
        QTextEdit {
            background-color: %1;
            color: %2;
            border: none;
            border-radius: 4px;
            padding: 4px;
        }
    )")
    .arg(BG_PRINCIPAL)      // %1
    .arg(TXT_PRINCIPAL)     // %2
    .arg(BOTON_GRIS_OSCURO) // %3
    .arg(BOTON_GRIS_OSCU_HOVER) // %4
    .arg(BORDER_PRINCIPAL)  // %5
    .arg(ACCENT_COLOR);     // %6
}

QColor ColorUtils::hexToQColor(const QString &hex)
{
    return QColor(hex);
}

QString ColorUtils::qColorToHex(const QColor &color)
{
    return color.name();
}
