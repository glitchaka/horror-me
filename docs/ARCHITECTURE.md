# Arquitectura y límites

**Diseño:** C++20, sin dependencias web, ejecutable Windows nativo.

## Capas y dependencias

```
WinApp (Win32)  ---> GameSession <--- Maze
       |                  |
       v                  v
     Input               Types
       |
       +---> SceneRenderer ---> Canvas
       +---> HudRenderer   ---> Canvas
       +---> IAudio <--- WinAudio (WinMM)
```

- **Maze**: genera y consulta la topología. No sabe nada de ventanas, enemigos ni sonido.
- **GameSession**: administra máquina de estados, misiones, presión, persecución y eventos de audio. Solo recibe `Input` y tiempo; no llama a la API de Windows ni dibuja.
- **Canvas**: operaciones de framebuffer y fuente rasterizada; desconoce las reglas del juego.
- **SceneRenderer**: raycasting y sprites. Lee una vista de solo lectura de `GameSession`.
- **HudRenderer**: menús, indicadores y mensajes. No altera la partida.
- **IAudio**: contrato de audio intercambiable; `WinAudio` es el adaptador WinMM.
- **WinApp**: ciclo de vida, ventana, mensajes, entrada y presentación GDI; integra los módulos.

## Criterios SOLID

- **S (responsabilidad única):** simulación, generación, dibujo, GUI y audio viven en módulos separados.
- **O (abierto/cerrado):** agregar otra salida de audio mediante `IAudio`, sin cambiar `GameSession`.
- **L (sustitución):** toda implementación de `IAudio` satisface los mismos métodos.
- **I (segregación):** `IAudio` expone únicamente operaciones de sonido, no controles gráficos ni de juego.
- **D (inversión):** la capa de aplicación depende del contrato `IAudio`; el núcleo produce eventos y no depende de WinMM.

Estos principios son decisiones arquitectónicas concretas, no una garantía de que cada detalle de implementación sea perfecto.

## Invariantes de jugabilidad

- Los cambios de mapa siempre reubican al jugador en una casilla transitable.
- Los terminales y el núcleo ocupan casillas transitables.
- El núcleo permanece estable en la fase final; no hay reconfiguraciones que impidan terminar.
- El input se consume como estado actual + flanco de pulsación para evitar acciones duplicadas.
- Las teclas de movimiento nunca activan ataques (no hay mecánica de disparos).
- El puntero se libera cuando la aplicación pierde el foco o entra en pausa.

## Estado de pruebas

La implementación debe subirse al repositorio **antes** de compilar o ejecutar por primera vez, de acuerdo con el flujo de trabajo solicitado. Pendiente: compilación Windows, análisis estático, validación de navegación y equilibrio de dificultad.
