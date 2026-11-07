# Integración experimental de disk-seal en HorrorMe

La variante protegida vincula un Installation ID dentro del HorrorMe.exe con el sello físico del pendrive y con una licencia Ed25519 firmada por la autoridad administrativa. La validación se ejecuta dentro del juego, antes de abrir la ventana, no mediante un lanzador externo.

## Compilación

La versión ordinaria de HorrorMe sigue intacta: HORROR_ENABLE_DISK_SEAL=OFF por defecto.

Para una variante protegida en Windows x64 con MSVC:

- Activar HORROR_ENABLE_DISK_SEAL=ON al configurar CMake.
- Proporcionar HORROR_SEAL_PUBLIC_KEY_HEX, la clave pública real de la autoridad firmante (64 caracteres hexadecimales).
- CMake compila el puente Rust que utiliza disk-seal, enlazándolo de forma estática con el juego. Su dependencia está fijada a un commit.

## Preparación de la instalación para otro técnico

1. Personalizar una copia del HorrorMe.exe protegido mediante disk-seal, incorporándole un ID único.
2. Copiar ese ejecutable al USB final y generar installation.seal allí mismo, con aplicación horror-me.
3. Fuera del USB, firmar los bytes exactos del sello mediante SignedLicense::issue y la clave privada de la autoridad.
4. Guardar los dos archivos en data/installation.seal y data/installation.license, al lado del HorrorMe.exe personalizado.
5. Ejecutar HorrorMe.exe --disk-seal-check para validar sin interfaz: retorno 0 válido, 21 sin ID, 22 sello, licencia o USB inválido.

La firma Authenticode de Windows, si se usa, debe aplicarse DESPUÉS de personalizar el ejecutable. La clave privada de licencias jamás se distribuye con el juego ni se incluye en este repositorio.

## Pruebas automatizadas

El workflow de la rama de integración compila la variante con una clave pública efímera, personaliza un ejecutable real de Windows y confirma que no se abre sin autorización. Es deliberadamente una construcción distinta de la publicada en Releases.

GitHub Actions no dispone de un USB físico para comprobar el caso positivo; las pruebas de disk-seal validan el caso positivo con identidades de almacenamiento simuladas. La prueba con un pendrive físico y una licencia autorizada sigue pendiente.

El software de un repositorio público puede modificarse y recompilarse. Esta protección disuade copias casuales de ejecutables distribuidos, no proporciona invulnerabilidad frente a ingeniería inversa.
