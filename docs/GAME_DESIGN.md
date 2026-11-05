# PROTOCOLO: NO CONFORME — Diseño jugable

## Fantasía del jugador
No eres un héroe armado. Eres una persona competente atrapada en un sistema de verificación donde las metas cambian, las instrucciones se contradicen y todo se convierte en otra revisión. Esta es una **metáfora de trabajo**, no un diagnóstico psicológico del usuario.

El terror surge de la incertidumbre, el espacio estrecho, la proximidad de El Revisor y la amenaza de perder el control de la situación. La dificultad no debe proceder de controles defectuosos.

## Bucle principal
1. Leer el criterio actual en el HUD.
2. Explorar pasillos con el escáner o interpretar el plano.
3. Evitar al Revisor, protegerse con un ancla y encontrar la terminal vigente.
4. Mantener E para sellarla; una terminal equivocada aumenta la presión.
5. Aceptar una nueva instrucción, adaptarse a una reconfiguración local y repetir.
6. Tras cinco entregas, llegar al núcleo y pulsar R: **rechazar** la orden final.

## Fases narrativas
| Entregas completas | Acto | Propósito |
|---|---|---|
| 0 | I. Recepción | Aprender el espacio, movimientos y primera orden |
| 1 | II. Criterio variable | Constatar que una entrega aceptada no fija el criterio |
| 2 | III. Retrabajo | La repetición aumenta la presión y la persecución |
| 3 | IV. Auditoría | El Revisor gana protagonismo |
| 4 | V. Reincidencia | Conservar el control ante la última entrega |
| 5 | VI. La negativa | Llegar al núcleo, dejar de obedecer y salir |

## Mecánicas y consecuencias
- **Presión**: acumulador de 0 a 100. Sube lentamente y al acercarse o entrar en contacto con el Revisor; baja cuando se sella una entrega o se consume un ancla. A 100 termina la partida.
- **Revisión**: cada cuenta atrás modifica algunos pasillos; las posiciones del jugador, las terminales y el núcleo permanecen protegidas. Antes de aceptar el cambio se comprueba que el laberinto siga conectado.
- **Criterio vigente**: la terminal correcta puede cambiar tras una revisión, incluso si el jugador ya se dirigía a la anterior.
- **Escáner (F)**: muestra durante unos segundos la dirección del objetivo; tiene tiempo de reutilización y un pequeño coste de presión.
- **Ancla (Q)**: impide revisiones durante diez segundos, desplaza al Revisor y reduce la presión; las cargas son limitadas, pero pueden reponerse tras completar entregas.
- **Revisor**: perseguidor que navega el laberinto mediante búsqueda de caminos, acelera en las fases tardías y se hace más visible en la fase final.
- **Plano (Tab)**: herramienta explícita de orientación; muestra pasillos, terminales, núcleo, jugador y perseguidor.

## Compromisos de diseño
- Las acciones de movimiento no hacen otras acciones accidentalmente.
- Los objetivos deben permanecer accesibles tras cada revisión.
- La reconfiguración no debe teletransportar al jugador a un punto arbitrario.
- La opresión se comunica por el mundo, los mensajes, el audio y la IA, no por controles rotos.
- El fracaso debe tener una causa identificable, y la victoria una respuesta deliberada.

## Validación pendiente
La CI comprueba compilación de Windows y pruebas unitarias del núcleo. Todavía se requiere una sesión visual e interactiva en Windows para calibrar dificultad, ratón, fidelidad de audio, escalado GDI y legibilidad del HUD.
