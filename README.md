# PROTOCOLO: NO CONFORME

Videojuego nativo de horror sistémico en primera persona desarrollado con **C++20** para Windows. Renderizado por software Win32/GDI, audio procedural WinMM y generación de laberintos por código. Incluye un rework de arte industrial, luces dinámicas y tipografía Annapurna SIL rasterizada. **No contiene HTML, CSS, JavaScript ni WebView.**

## La experiencia

Debes completar cinco entregas dentro de un laberinto de pasillos opresivos. El Revisor te persigue, los criterios se contradicen y parte del espacio se transforma mientras trabajas. Tu fortaleza no es disparar: es observar, orientarte, estabilizar el mundo y decidir cuándo rechazar una orden.

Hay seis actos narrativos, terminales empotradas en paredes, escáner direccional, anclas de estabilidad, persecución mediante búsqueda de caminos, acumulación de presión y un desenlace jugable. El HUD indica la orden vigente y los pasos necesarios; el plano (Tab) muestra una ruta transitable hacia el objetivo.

## Ejecutable publicado

El archivo **HorrorMe-Windows-x64.exe** se publica directamente en [GitHub Releases](https://github.com/glitchaka/horror-me/releases) cuando las pruebas de Windows pasan. No se distribuye el código como ZIP.

El workflow extrae la versión de `project(HorrorMe VERSION ...)` en `CMakeLists.txt`. Para publicar otra versión, se debe aumentar esa versión y actualizar `docs/RELEASE_NOTES.md`. Una versión ya publicada no se sobrescribe silenciosamente.

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

## Regla de navegación

Las terminales y el panel del núcleo están integrados en los muros; no bloquean los pasillos. Cada orden señala el panel correcto y su distancia por ruta. Pulsa `Tab` para consultar el trayecto marcado en azul o `F` para escanear la dirección del siguiente tramo. Acércate al panel iluminado y mantén `E`; los paneles incorrectos muestran una advertencia. Al finalizar las cinco entregas, localiza el núcleo, acércate y pulsa `R`.

## Dirección visual v0.4.0

La puesta en escena diferencia azulejos administrativos rotos, paredes metálicas oxidadas, hormigón húmedo y archivos industriales. El suelo y el techo se dibujan en perspectiva a partir de coordenadas del mundo; incluye focos intermitentes, lámparas de emergencia, agua, señalización, neblina de distancia y una degradación visual modulada por la presión.

El Revisor tiene una silueta alargada y asimétrica, rostro eclipsado por el resplandor de sus ojos, uniforme de inspección y extremidades imposibles. Los terminales se representan como consolas industriales integradas en paredes.

El HUD utiliza glifos antialias de las fuentes **Annapurna SIL Regular y Bold**, integrados como un atlas rasterizado en C++. No es necesario instalar los archivos TTF. El UI mantiene visible la orden, distancia y ruta en el mapa incluso durante episodios de interferencia.

**Calidad:** compilación y pruebas automatizadas sobre el renderizado antes de publicar; todavía es necesaria una evaluación manual de legibilidad, contraste, ritmo y atmósfera en Windows.
