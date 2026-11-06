# PROTOCOLO: NO CONFORME — v0.4.1 (Windows x64)

### Correcciones visuales y de ventana
- Escalado **pixel-perfect**: aumento entero del framebuffer a los píxeles físicos de pantalla, sin suavizado, con bandas negras cuando la resolución no es múltiplo exacto.
- Per-monitor DPI awareness configurada antes de crear la ventana para evitar doble escalado de Windows.
- Nueva presentación sin decoración: pantalla completa sin bordes de inicio. **F11** o **Alt+Enter** alternan con ventana sin bordes.
- Minimap permanente con jugador blanco, objetivo dorado, ruta azul y Revisor rojo. `Tab` conserva el mapa ampliado.
- Eliminada la cruceta central estática: ya no apunta accidentalmente al enemigo.
- Marcador del objetivo ligado al panel correcto, cuando está a la vista; indicación de siguiente tramo de ruta cuando aún se encuentra tras un muro.
- Glifos de Annapurna SIL renderizados con píxeles nítidos.
- Pruebas automatizadas de composición sin filtrado a 3×/5×, minimapa y ubicación del marcador del objetivo.

### Plataforma
Windows x64 nativo en C++20, Win32/GDI y WinMM. Sin Android ni tecnologías web.

### Calidad
Compilación y pruebas automatizadas de Windows. Falta comprobar el comportamiento a escala visual en diferentes monitores y configuraciones de DPI.
