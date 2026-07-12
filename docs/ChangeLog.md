v0.89:
        - macOS: corrige el icono de Vimeo. En v0.88 el glyph había quedado mal (círculo+flecha chico, al ~50%, con un anillo fantasma del squircle viejo) porque la extracción por diferencia de fondo falló con el marco blanco del diseño original. Ahora se extrae por saturación (círculo CMY y flecha teal) y queda al **74%** limpio. Además el fondo pasa al **mismo gris que PipeSync** (gradiente vertical 46→28, nunca negro), consistente con el resto de las apps LGA. [ Mac - Fix icono Vimeo (glyph 74% + gris estandar) ]

v0.88:
        - macOS: se ajusta el tamaño del glyph dentro de `LGA_VimeoDownloader.icns`. Quedaba grande (~89% del squircle); ahora va al **74%** con padding (como Codex/PipeSync), centrado sobre el plate plano (squircle 80% de Apple). [ Mac - Glyph al 74% del squircle ]

v0.87:
        - macOS: el icono de la app pasa al estándar glass — `.icns` squircle plano a la grilla exacta del 80% de Apple (esquinas continuas), sin rim horneado, dejando que macOS Tahoe agregue el borde de Liquid Glass al renderizar (como Photoshop/Cursor). Corrige el "icon jail" (recuadro gris en vistas de lista/detalle). Aprendizaje completo en `LGA_PipeSync_2/Docs/_Doc_Aprendizaje_QT_C.md`. [ Mac - Icono glass 80% + Tahoe rim ]

v0.86:
        - Version actual inicial registrada para empezar a mantener el changelog del repo bajo reglas `.cursor` y `.codex`. [ Reglas - Se inicializa changelog y reglas de repo ]

