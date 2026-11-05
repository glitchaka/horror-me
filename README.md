# PROTOCOLO: NO CONFORME

Videojuego nativo de horror sistémico en primera persona desarrollado con **C++20** para Windows. Renderizado por software Win32/GDI, audio procedural WinMM y generación de laberintos por código. **No contiene HTML, CSS, JavaScript ni WebView.**

## La experiencia

Debes completar cinco entregas dentro de un laberinto de pasillos opresivos. El Revisor te persigue, los criterios se contradicen y parte del espacio se transforma mientras trabajas. Tu fortaleza no es disparar: es observar, orientarte, estabilizar el mundo y decidir cuándo rechazar una orden.

Hay seis actos narrativos, escáner direccional, anclas de estabilidad, persecución mediante búsqueda de caminos, acumulación de presión, mensajes del sistema y un desenlace jugable.

## Ejecutable publicado

El archivo **HorrorMe-Windows-x64.exe** se publica directamente en [GitHub Releases](https://github.com/glitchaka/horror-me/releases) cuando las pruebas de Windows pasan. No se distribuye el código como ZIP.

El workflow extrae la versión de `project(HorrorMe VERSION ...)` en `CMakeLists.txt`. Para publicar otra versión, se debe aumentar esa versión y actualizar `docs/RELEASE_NOTES.md`. Una versión ya publicada no se sobrescribe silenciosamente.

## Android — APK nativo de prueba

El cliente para teléfonos Android ARM64 reutiliza el motor C++20 mediante JNI; no usa HTML, WebView ni JavaScript. La interfaz táctil se dibuja con Android SurfaceView y el audio usa AudioTrack.

La versión de prueba se publica como **HorrorMe-Android-arm64-test.apk** dentro de [GitHub Releases](https://github.com/glitchaka/horror-me/releases) una vez aprobada su compilación Android. La firma es de depuración; puede requerir desinstalar una versión anterior para actualizar. Requiere Android 7+ y procesador ARM64.

Controles: palanca izquierda para desplazamiento, arrastrar para mirar, botones SELLAR, ESCAN, ANCLA, PLANO, NO, CORRER y PAUSA. Detalles en [android/README.md](android/README.md).

## Compilar en Windows

Requisitos: Windows 10/11, CMake 3.21+ y compilador MSVC con soporte de C++20.

```powershell
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Con Visual Studio, el ejecutable queda en `build/Release/HorrorMe.exe`. Para otros generadores consulta el directorio de salida indicado por CMake.

## Controles

| Tecla | Función |
|---|---|
| W / S | Avanzar / retroceder |
| A / D | Desplazamiento lateral |
| Ratón o flechas izquierda / derecha | Girar |
| Shift | Correr |
| E (mantener) | Sellar la terminal correcta |
| F | Escanear la dirección del objetivo |
| Q | Activar un ancla |
| Tab (mantener) | Consultar el plano |
| P / Esc | Pausar / continuar |
| M | Silenciar |
| Enter | Comenzar / reiniciar |
| R (cerca del núcleo) | Rechazar la última exigencia |

No hay mecánicas de salto ni disparos.

## Arquitectura

- `include/horror/`: contratos, tipos y componentes.
- `src/core/`: topología, navegación, perseguidor y máquina de estados, sin Win32.
- `src/render/`: framebuffer, raycasting, sprites generados por código y HUD.
- `src/platform/`: ventana, eventos, presentación GDI y sintetizador de audio WinMM.
- `tests/CoreTests.cpp`: conectividad, reconfiguración de mapas, IA y estados.
- `docs/ARCHITECTURE.md`: dependencias, invariantes y principios SOLID.
- `docs/GAME_DESIGN.md`: fases y mecánicas de juego.
- `.github/workflows/windows.yml`: compilación y pruebas Windows.

## Calidad y límites

La construcción se comprueba en GitHub Actions con MSVC Release y CTest. Las pruebas del núcleo verifican conectividad del mapa tras múltiples mutaciones, acceso a objetivos y respuesta del perseguidor.

**Queda pendiente una prueba visual y de jugabilidad manual en Windows.** La compilación correcta no demuestra por sí sola que el equilibrio, la presentación y la experiencia de juego sean definitivos.
