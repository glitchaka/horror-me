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
- **GameSession**: orquesta reglas, criterios, entregas, presión, eventos sonoros, recompensas, penalizaciones y estados; no dibuja.
- **Input**: distingue teclas mantenidas de pulsaciones nuevas. Permite una única responsabilidad por control.
- **Canvas**: framebuffer RGB y rasterización de tipografía propia.
- **SceneRenderer**: raycasting de muros, sprites y efectos visuales; solo consulta el estado.
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

1. El jugador, El Revisor y cada terminal están en una casilla transitable.
2. Todos los espacios transitables permanecen conectados tras una distorsión.
3. Las revisiones locales no mueven arbitrariamente ni al jugador ni al objetivo.
4. El plazo de revisión se ajusta a la distancia por la red de pasillos.
5. Una vez completadas cinco entregas, el núcleo permanece accesible y no se generan nuevas revisiones.
6. Las acciones discretas leen pulsaciones nuevas; mantener teclas de movimiento no dispara otra acción.
7. Las actualizaciones de la simulación tienen paso de tiempo limitado y están separadas de la presentación.

## Verificación

`tests/CoreTests.cpp` prueba laberintos de diferentes semillas, mutaciones sin pérdida de acceso, avance del perseguidor y estados de partida. `.github/workflows/windows.yml` compila juego y pruebas con MSVC Release y ejecuta CTest.

Pendiente: pruebas manuales de interacción, experiencia audiovisual, accesibilidad, dificultad y estrés sostenido.
