# Integración experimental de disk-seal en HorrorMe

La variante protegida vincula un Installation ID dentro del HorrorMe.exe con el sello físico del pendrive y con una licencia Ed25519 firmada por la autoridad administrativa. La validación se ejecuta dentro del juego, antes de abrir la ventana, no mediante un lanzador externo.

## Compilación

La versión ordinaria de HorrorMe sigue intacta: HORROR_ENABLE_DISK_SEAL=OFF por defecto.

Para una variante protegida en Windows x64 con MSVC:

- Activar HORROR_ENABLE_DISK_SEAL=ON al configurar CMake.
- Proporcionar HORROR_SEAL_PUBLIC_KEY_HEX, la clave pública real de la autoridad firmante (64 caracteres hexadecimales).
- CMake compila el puente Rust que utiliza disk-seal, enlazándolo de forma estática con el juego. Su dependencia está fijada a un commit.

## Preparación de la instalación para otro técnico

La aplicación protegida **no autoriza cualquier pendrive que tenga el ejecutable**.
El USB permitido es aquel en el que se crea el sello durante la emisión:
`disk-seal` consulta el volumen/dispositivo actual del archivo de destino y
firma su vínculo con el Installation ID interno del HorrorMe.exe personalizado.

Ya existe una CLI administrativa en `glitchaka/disk-seal` que automatiza esa
emisión. En un equipo Windows de confianza, con la carpeta `C:\\SST-Admin`
y la herramienta `disk-seal.exe` compilada:

```powershell
# Solo una vez: emitir autoridad de licencias protegida mediante Windows DPAPI
disk-seal.exe keygen C:\SST-Admin\authority.dskey C:\SST-Admin\authority.pub

# Configurar/compilar la variante protegida de HorrorMe usando la clave pública
$public = (Get-Content C:\SST-Admin\authority.pub -Raw).Trim()
cmake -S . -B build-protected -A x64 -DBUILD_TESTING=OFF -DHORROR_ENABLE_DISK_SEAL=ON "-DHORROR_SEAL_PUBLIC_KEY_HEX=$public"
cmake --build build-protected --config Release

# Pendrive E: de Juan. Plantilla oficial SIN personalizar en el PC administrador.
New-Item -ItemType Directory -Force E:\HorrorMe | Out-Null
disk-seal.exe provision horror-me 0.4.1 C:\horror-me\build-protected\Release\HorrorMe.exe E:\HorrorMe\HorrorMe.exe Juan C:\SST-Admin\authority.dskey

# Juego protegido: comprobar desde Windows sin abrir la ventana
$p = Start-Process -FilePath E:\HorrorMe\HorrorMe.exe -ArgumentList '--disk-seal-check' -Wait -PassThru
$p.ExitCode  # 0 significa autorizado
```

La clave privada `authority.dskey` queda cifrada mediante DPAPI para la
cuenta administradora. **No se copia al pendrive de Juan**. La carpeta
`E:\HorrorMe\data` contendrá únicamente sello y licencia firmada.

Las instrucciones completas están en
[PROVISIONING_WINDOWS.md](https://github.com/glitchaka/disk-seal/blob/main/docs/PROVISIONING_WINDOWS.md).

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
