# Compilación nativa para Android

Esta versión utiliza el mismo motor C++20 que la edición Windows.

## Arquitectura
- ../src/core y ../src/render: simulación y renderizado compartidos.
- app/src/main/cpp/NativeBridge.cpp: puente JNI, sin duplicar reglas.
- MainActivity.java: SurfaceView, orientación horizontal, mandos táctiles, ciclo de vida.
- AudioSynth.java: audio chiptune generado mediante AudioTrack, sin recursos externos.

## Controles
- Palanca izquierda: movimiento en cuatro direcciones.
- Deslizar en la zona central: girar la vista.
- CORRER: movimiento rápido.
- SELLAR (mantener): operar la terminal.
- ESCAN: dirección del objetivo.
- ANCLA: estabilizar el laberinto.
- PLANO (mantener): consultar mapa.
- NO: rechazar la orden final cerca del núcleo.
- PAUSA y AUDIO: suspender y controlar el sonido.
- ENTRAR o REINICIAR: nueva partida.

## Compilación
Se requiere Android SDK 35, NDK 27.2.12479018, CMake 3.22.1, Java 17 y Gradle 8.10.2.
Ejecutar: gradle -p android :app:assembleDebug
Salida: android/app/build/outputs/apk/debug/app-debug.apk

GitHub Actions adjunta el APK ARM64 a la Release junto con Windows.
**Es una versión de prueba firmada con la clave de depuración del entorno CI.** No es una firma de producción persistente; para actualizar puede requerir desinstalar la versión anterior.
Android 7.0 o superior (API 24), procesador ARM64 y orientación horizontal.
Sin WebView, HTML ni JavaScript.
