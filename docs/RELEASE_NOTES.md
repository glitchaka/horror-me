# PROTOCOLO: NO CONFORME — Versión 0.3.0 (Windows x64)

### Correcciones
- Terminales de misión y panel final representados como dispositivos empotrados en paredes, sin objetos similares a puertas en medio del pasillo.
- Mejor orientación: orden y número de entrega claros, distancia en pasos, camino marcado en el plano (Tab) y escáner (F) hacia el siguiente segmento transitable.
- Advertencia de terminal incorrecta y señal clara al estar junto a la terminal adecuada.
- Los cambios de mapa preservan paredes con paneles; comprobado mediante pruebas del núcleo.

### Plataforma
- Windows x64 nativo, C++20, GDI, WinMM.
- Se retiró del árbol de código el port experimental Android; fue solo una prueba puntual.

### Publicación
- `HorrorMe-Windows-x64.exe` y archivo de verificación SHA-256.
- La compilación Windows y las pruebas automáticas deben pasar antes de publicar.

### Pendiente
Verificación manual visual, legibilidad de paneles en distintas distancias, equilibrado de IA y controles.
