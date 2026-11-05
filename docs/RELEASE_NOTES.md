# PROTOCOLO: NO CONFORME — Versión 0.2.0 (Windows y Android)

Versión nativa del juego de horror sistémico en primera persona para Windows x64 y Android ARM64.

### Android (prueba)
- HorrorMe-Android-arm64-test.apk: APK de prueba para Android ARM64, firmado con la clave de depuración automática de GitHub Actions.
- Pantalla horizontal y controles táctiles: palanca izquierda para moverse, arrastre para girar; botones Sellar, Escán, Ancla, Plano, No y Pausa.
- Simulación, renderizado, IA y laberinto utilizan los mismos módulos C++20 de Windows.
- La salida gráfica utiliza SurfaceView/Canvas nativos y el sonido AudioTrack; sin tecnologías web ni ficheros externos.
- Android puede solicitar autorización para instalar aplicaciones desde el navegador o administrador de archivos.
- APK de prueba, no distribución final para Google Play.

### Windows
- HorrorMe-Windows-x64.exe: ejecutable nativo con runtime C++ estático.

### Integridad
Los archivos .sha256 adjuntos permiten verificar los binarios.

### Estado de calidad
Windows verifica la compilación y los tests del núcleo. Android compila con Gradle/NDK y comprueba que el APK incluya la biblioteca nativa ARM64. Pendientes: pruebas manuales en teléfono, respuesta táctil y audio.
