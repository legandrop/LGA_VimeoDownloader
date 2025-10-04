#ifndef COLORUTILS_H
#define COLORUTILS_H

#include <QString>
#include <QColor>

class ColorUtils
{
public:
    // Colores del tema oscuro de PipeSync
    static const QString BG_PRINCIPAL;
    static const QString TXT_PRINCIPAL;
    static const QString BOTON_GRIS_OSCURO;
    static const QString BOTON_GRIS_OSCU_HOVER;
    static const QString BORDER_PRINCIPAL;
    static const QString ACCENT_COLOR;
    
    // Métodos utilitarios
    static QString getStyleSheet();
    static QColor hexToQColor(const QString &hex);
    static QString qColorToHex(const QColor &color);
};

#endif // COLORUTILS_H
