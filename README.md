# PROTOCOLO: NO CONFORME

Videojuego nativo de horror sistémico en primera persona. Código fuente C++20 para Windows, con renderizado por software mediante Win32/GDI y audio procedural WinMM. **No usa HTML, JavaScript, WebView ni recursos gráficos externos.**

## Concepto
Un laberinto laboral imposible que se reescribe mientras intentas cumplir cinco entregas. El Revisor persigue, las prioridades cambian y la presión se acumula. Dispones de un escáner y anclas de estabilidad. La última orden no se resuelve obedeciendo: se resuelve diciendo **NO**.

## Compilación (Windows)

Requiere CMake 3.21+ y un compilador C++20 (MSVC o MinGW con Win32 y WinMM).

```powershell
cmake -S . -B build
cmake --build build --config Release
```

Con generadores multiconfiguración, el ejecutable queda en `build/Release/HorrorMe.exe` (el destino exacto depende del generador).

## Controles
- `W/S`: avanzar/retroceder.
- `A/D`: desplazamiento lateral.
- Ratón o flechas izquierda/derecha: mirar.
- `Shift`: correr.
- `E`: mantener para sellar la terminal correcta.
- `F`: escáner del objetivo.
- `Q`: ancla temporal.
- `Tab`: consultar el plano.
- `P` o `Esc`: pausar o reanudar.
- `M`: silenciar.
- `Enter`: comenzar o reiniciar.
- `R`: rechazar la última exigencia en el núcleo.

## Organización
- `include/horror/`: contratos y modelos públicos.
- `src/core/`: generación del laberinto y reglas del juego, independientes de Windows.
- `src/render/`: framebuffer, raycasting y HUD por software.
- `src/platform/`: ventana, entrada, presentación GDI y audio WinMM.
- `docs/ARCHITECTURE.md`: responsabilidades, dependencias y decisiones.
- `CMakeLists.txt`: compilación reproducible.

## Estado de esta entrega
Refactorización modular de la primera implementación. **Subida al repositorio antes de ejecutar compilaciones o pruebas**, según el flujo de trabajo acordado. No declarar esta revisión como compilada o validada funcionalmente hasta comprobarla.
