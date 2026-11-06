# Arquitectura de horror-me

**Objetivo:** un juego nativo C++20 para Windows, fácil de ampliar sin convertirlo en un `main.cpp` monolítico.

## Dependencias

```
       +-----------------------------+
       | WinApp / Win32 / Game loop  |
       +-----------------------------+
        |         |         |       |
      Input    GameSession   |     IAudio <--- WinAudio / WinMM
                  |          |
            +-----+-----+    +--> SceneRenderer --> Canvas
            |           |    +--> HudRenderer   --> Canvas
           Maze      RevisorAI
            ^           |
            +-----------+
```

## Responsabilidades

- **Maze**: estructura del mapa, búsqueda de rutas, celdas transitables y mutaciones locales que conservan conectividad.
- **RevisorAI**: posición del perseguidor, búsqueda de caminos, velocidad y desplazamiento; no gestiona ventanas, audio, objetivos ni menús.
- **GameSession**: orquesta reglas, criterios, entregas, presión, eventos sonoros, recompensas, penalizaciones y estados; asigna cada terminal a una casilla transitable con una pared adyacente, sin dibujar.
- **Input**: distingue teclas mantenidas de pulsaciones nuevas. Permite una única responsabilidad por control.
- **Canvas**: framebuffer RGB y rasterización de tipografía propia.
- **SceneRenderer**: raycasting de muros y paneles como decals en su cara frontal; solo el Revisor se representa como sprite dentro del corredor.
- **HudRenderer**: presenta información, menús, plano y mensajes; solo consulta el estado.
- **IAudio**: interfaz de audio abstracta; **WinAudio** utiliza WinMM y una cola de buffers PCM.
- **WinApp**: adaptación Windows, ciclo de vida, temporizador, teclado, ratón, presentación GDI e integración.

## SOLID — aplicación verificable

- **S:** módulos separados para topología, perseguidor, reglas, render, HUD y adaptadores nativos.
- **O:** otra implementación de `IAudio` puede sustituir a WinAudio; el núcleo no tiene por qué cambiar.
- **L:** los adaptadores de audio deben respetar `IAudio`.
- **I:** `IAudio` solo expone control de reproducción, eventos, tensión y silencio.
- **D:** WinApp invoca el contrato de sonido; GameSession emite `SoundEvent`, sin referenciar WinMM.

Estas decisiones aplican los principios al alcance actual; no implican que toda la arquitectura esté exenta de posibilidades de mejora.

## Invariantes

1. El jugador y El Revisor están en casillas transitables; cada terminal se opera desde una casilla transitable y se representa en una pared vecina, nunca como puerta flotante.
2. Los muros que alojan terminales o el núcleo se preservan en cada distorsión y todos los espacios transitables siguen conectados.
3. Las revisiones locales no mueven arbitrariamente ni al jugador ni al objetivo.
4. El plazo de revisión se ajusta a la distancia por la red de pasillos.
5. Una vez completadas cinco entregas, el núcleo permanece accesible y no se generan nuevas revisiones.
6. Las acciones discretas leen pulsaciones nuevas; mantener teclas de movimiento no dispara otra acción.
7. Las actualizaciones de la simulación tienen paso de tiempo limitado y están separadas de la presentación.

## Verificación

`tests/CoreTests.cpp` prueba laberintos de diferentes semillas, mutaciones sin pérdida de acceso, avance del perseguidor y estados de partida. `.github/workflows/windows.yml` compila juego y pruebas con MSVC Release y ejecuta CTest.

Pendiente: pruebas manuales de interacción, experiencia audiovisual, accesibilidad, dificultad y estrés sostenido.

## Subsistema visual (v0.4.0)
- `MaterialArtist`: calcula materiales del mundo en coordenadas estables, luz local, sector y degradación. Sin dependencias Win32 ni estado mutable.
- `Atmosphere`: procesa el framebuffer 3D antes de la interfaz con viñeta, ruido y anomalías localizadas por presión.
- `FontAtlas`: glifos rasterizados de Annapurna SIL Regular y Bold, empaquetados a 2 bits por píxel. No distribuye archivos TTF originales.
- `SceneRenderer`: coordina la proyección de techo/suelo, paredes, paneles murales, Revisor y posproceso atmosférico.
- `HudRenderer`: dossier institucional opresivo con objetivos y controles legibles; no recibe el posproceso visual, garantizando contraste.
- `tests/RenderTests.cpp`: comprueba atlas, materiales y dibujo 3D junto al HUD sin requerir una ventana.

## Presentación y navegación (v0.4.1)

- `PixelViewport`: calcula el factor entero de escala y los márgenes de letterboxing en píxeles físicos.
- `PixelPresenter`: convierte el framebuffer de 384×216 en una imagen del tamaño del cliente aplicando **solo replicación de píxeles**, sin filtros. Es independiente de Win32 y puede probarse en CI.
- `WinApp`: administra la ventana `WS_POPUP`, resolución del monitor, conmutación F11/Alt+Enter, DPI aware por monitor y `SetDIBitsToDevice` 1:1. No implementa algoritmos de escalado.
- `HudRenderer`: minimapa permanente y mapa ampliado separados; guía al objetivo mediante la ruta y usa colores distintos para objetivo y perseguidor.
- `SceneRenderer`: sin mira fija; las marcas de objetivo se proyectan sobre el panel mural y quedan detrás del sprite del enemigo si se cruzan.
