# STACH

[Русский](README.md) · [English](README.en.md) · [简体中文](README.zh-CN.md) · [繁體中文](README.zh-TW.md) · **Español** · [हिन्दी](README.hi.md) · [العربية](README.ar.md) · [Français](README.fr.md) · [বাংলা](README.bn.md) · [Português](README.pt.md) · [Bahasa Indonesia](README.id.md) · [اردو](README.ur.md) · [Deutsch](README.de.md) · [日本語](README.ja.md) · [한국어](README.ko.md) · [Türkçe](README.tr.md) · [Tiếng Việt](README.vi.md) · [Italiano](README.it.md) · [فارسی](README.fa.md) · [Polski](README.pl.md) · [Українська](README.uk.md)

STACH convierte un **Waveshare ESP32-S3-Touch-AMOLED-1.75** en un compañero de bolsillo. Sus ojos siguen tu dedo y responden a la inclinación. Desliza el dedo hacia la derecha para jugar o usa tu teléfono para cambiar su expresión, brillo, comportamiento y nombre para mostrar. La placa alberga su propia red Wi-Fi y panel de control.

![STACH eye animation](preview/stach-smart-eyes.gif)

## Instalar en un Waveshare 1.75

Estas instrucciones fueron probadas en Windows con el entorno `stach-eyes-175` PlatformIO. Crea la aplicación completa desde `src/main.cpp` para la pantalla redonda CO5300 de 466 × 466, 16 MB de flash y 8 MB de PSRAM. Verifique el modelo en la placa o en su empaque antes del flasheo. Las revisiones 1.43, 1.43B y 1.75C tienen hardware diferente.

### 1. Prepara tu computadora

Necesita Python 3.13 con el iniciador `py`, Git para Windows, un cable de datos USB y una conexión a Internet para la primera compilación. [Pitón para Windows](https://www.python.org/downloads/windows/) · [Git para Windows](https://git-scm.com/downloads/win)

Abra PowerShell y clone el proyecto en una nueva carpeta. Una ruta corta con caracteres ASCII funciona bien con las herramientas ESP32.

```powershell
cd $env:USERPROFILE
git clone https://github.com/leonidgg2323-code/stach.git stach-firmware
cd stach-firmware
py -3.13 -m venv .venv
.\.venv\Scripts\python.exe -m pip install "platformio==6.2.0"
```

Alternativamente, elija Código **→ Descargar ZIP** en GitHub y extraiga el archivo. Abra PowerShell en la carpeta que contiene `platformio.ini` y ejecute los dos últimos comandos anteriores. Los siguientes comandos llaman directamente al Python del entorno virtual.

### 2. Instalar las dependencias y construir

```powershell
.\.venv\Scripts\python.exe -m platformio pkg install -e stach-eyes-175
.\.venv\Scripts\python.exe -m platformio run -e stach-eyes-175
```

La primera compilación descarga la cadena de herramientas ESP32 y puede tardar varios minutos. La compilación aplica automáticamente el parche de compatibilidad de la biblioteca de mouse Bluetooth. Espere a `stach-eyes-175  SUCCESS` antes de cargar.

### 3. Encuentra el puerto serie y actualiza la placa.

Conecte la placa por USB y enumere los puertos disponibles.

```powershell
.\.venv\Scripts\python.exe -m platformio device list
```

Identifique el puerto que aparece cuando conecta la placa y desaparece cuando la desconecta. Reemplace `COM3` en los ejemplos con su puerto. Cierre cualquier programa que lo utilice, incluido el monitor serie Arduino IDE.

La carga de **reemplaza el firmware en la placa.** Para mantener la opción de restaurar el menú de fábrica, guarde una copia de seguridad flash completa fuera del repositorio antes de cargar. Una copia de seguridad completa para este modelo es **16,777,216 bytes**. Utilice un nombre de archivo nuevo si ya tiene una copia de seguridad. La imagen puede contener configuraciones y credenciales, así que manténgala privada.

```powershell
.\.venv\Scripts\python.exe -m pip install "esptool>=5,<6"
.\.venv\Scripts\python.exe -m esptool --chip esp32s3 --port COM3 read-flash 0 ALL "$env:USERPROFILE\stach-before-install.flash-backup.bin"
```

Para instalar STACH, ejecute el comando de carga.

```powershell
.\.venv\Scripts\python.exe -m platformio run -e stach-eyes-175 -t upload --upload-port COM3
```

La placa se reinicia después de la verificación de la suma de verificación y el mensaje `SUCCESS`. PlatformIO escribe el gestor de arranque, la tabla de particiones y la aplicación en sus respectivas direcciones. El archivo `firmware.bin` en el directorio de compilación contiene solo la aplicación.

### 4. Registre la cuenta del propietario

1. En el primer arranque, escanee **1/2 JOIN WI-FI** con su teléfono. Únete a la red `Stach-XXXXXX`. Si su teléfono informa que no tiene acceso a Internet, elija **Manténgase conectado**.
2. La pantalla cambia a **2/2 REGISTER** cuando se conecta el teléfono. Si el primer código aún se muestra, toque la pantalla una vez. Escanea el segundo código con el mismo teléfono.
3. Elija un nombre de usuario de 3 a 32 letras (A–Z o a–z), dígitos, guiones o guiones bajos y una contraseña de al menos 12 caracteres. Guarde sus datos de inicio de sesión. La cuenta y la configuración se almacenan en el tablero.
4. En **Аккаунт владельца** (Cuenta de propietario), puede cambiar el nombre que se muestra en el panel o cerrar sesión. Para visitas posteriores, conéctese al Wi-Fi de la placa y abra el [panel de control local STAC](http://192.168.4.1/).

La junta acepta una cuenta de propietario. El registro requiere el código de un solo uso que se muestra en la pantalla física. Los comandos de control requieren una sesión de propietario y un token de solicitud. El panel utiliza HTTP en la red Wi-Fi local de la placa. Inicie sesión nuevamente después de que se reinicie el tablero.

## Gestos y juegos

| acción | Resultado |
|---|---|
| Toca los ojos o inclina el tablero. | Los ojos siguen tu dedo o la inclinación. |
| Desliza hacia la derecha desde los ojos | Abre el menú de juegos. |
| Desliza hacia la izquierda en un juego | Volver al menú. Desliza el dedo hacia la izquierda nuevamente para volver a los ojos. |
| Mantenga la pantalla ocular durante aproximadamente un segundo. | Muestra el código QR de Wi-Fi. Toque para cambiar entre los códigos de red y de panel. Mantenga presionado para volver a los ojos. |
| Desliza hacia arriba desde los ojos | Inicie el modo de mouse Bluetooth. Emparéjelo con **Stach Mouse** en su computadora y mantenga la placa quieta para calibrarla. |
| Toca la pantalla en modo ratón | Toque para hacer clic izquierdo, toque dos veces para hacer clic derecho, deslice para desplazarse o mantenga presionado para salir. |

En **Targets**, alcanza tantos objetivos como puedas en 20 segundos. En **Sums**, responda cinco preguntas de suma. Ambos juegos muestran tu puntuación y guardan tu mejor marca personal. **Dice** lanza un dado de seis caras y **Coin** lanza una moneda. Cada resultado proviene del generador de números aleatorios del hardware ESP32. Toque nuevamente después de que finalice la animación para realizar un nuevo giro o giro.

## Pantalla y potencia

El panel del teléfono incluye selección de expresión, un joystick de mirada, controles de parpadeo e inclinación, brillo de los ojos y una configuración de cuánto se mueven los ojos por sí solos. La configuración de apariencia persiste tras los reinicios. Puede habilitar el modo mouse después del inicio.

En la placa 1.75, el **AXP2101** informa el porcentaje de batería y el estado de carga. El indicador muestra `USB` cuando la placa funciona con alimentación por cable y no detecta batería. Con el ahorro de energía habilitado, la frecuencia de actualización de los ojos disminuye después de 45 segundos de inactividad y la pantalla se apaga después de 5 minutos. Con una carga reportada del 15% o menos, esos intervalos se reducen a 10 segundos y 2 minutos. Toca la pantalla para activarlo. Los juegos, los códigos QR, el modo mouse y la carga activa mantienen la pantalla activa.

## Solución de problemas y recuperación

| Síntoma | que probar |
|---|---|
| No aparece ningún nuevo puerto COM | Pruebe con un cable de datos USB y otro puerto USB, luego ejecute `platformio device list` nuevamente. |
| El puerto no se puede abrir o informa `Access denied` | Cierre el monitor serie. Vuelva a conectar la placa y verifique su número COM. |
| La construcción falla | Verifique que su carpeta actual contenga `platformio.ini`, luego repita ambos comandos en el paso 2. |
| La pantalla se queda en blanco | Verifique el modelo de placa y el nombre del entorno en el registro de carga. Para la placa 1.75, la línea de éxito debe incluir `stach-eyes-175`. |
| El teléfono no puede abrir el panel. | Verifique que todavía esté en `Stach-XXXXXX` y abra `http://192.168.4.1/`. El registro por primera vez requiere el segundo código QR. |

Para ver los diagnósticos, abra el monitor serie a 115200 baudios.

```powershell
.\.venv\Scripts\python.exe -m platformio device monitor -p COM3 -b 115200
```

Envíe `I` para obtener un informe de inicialización de hardware, `S` para enumerar los dispositivos I²C o `D` para una prueba de pantalla de cinco segundos. Presione **Ctrl+C** para cerrar el monitor.

Para restaurar su copia de seguridad completa en la misma placa, cierre el monitor y verifique primero el tamaño de la imagen. El siguiente comando también restaura la configuración y los datos de la cuenta contenidos en la copia de seguridad.

```powershell
.\.venv\Scripts\python.exe -m esptool --chip esp32s3 --port COM3 write-flash 0x0 "$env:USERPROFILE\stach-before-install.flash-backup.bin"
```

**¿Necesitas ayuda?** Comuníquese con `twiinkprostolox123` en Discord o [@def0kk (Telegram)](https://t.me/def0kk). Incluya el modelo de placa, el entorno PlatformIO y el mensaje de error. Mantenga en privado su contraseña, código QR de registro y copia de seguridad flash completa.

## Historia y desarrollo del proyecto.

- **Etapa 9 - Potencia.** Los perfiles 1.43 más antiguos miden y filtran el voltaje ADC en GPIO4. Para estimar la carga es necesario verificar el esquema de la revisión específica y calibrarlo con un voltaje de batería medido. La alimentación USB por sí sola no confirma la carga. El perfil 1,75 lee directamente el AXP2101.
- **Etapa 10: Modos y personalización.** Cambia entre la cara, el mouse Bluetooth, Wi-Fi QR y el panel QR desde la pantalla o mediante `POST /api/mode`. El brillo de los ojos y la actividad de la mirada oscilan entre el 20% y el 100%. El control del mouse BLE requiere un perfil de hardware.
- **Etapa 11: Fiabilidad y seguridad.** El firmware verifica los orígenes de las solicitudes HTTP, protege los comandos con un token de solicitud, limita las solicitudes y recupera el punto de acceso Wi-Fi. El perfil 1.75 también requiere un inicio de sesión de propietario. [Notas de seguridad de la etapa 11](STAGE11-SECURITY.md)
- **Etapa 12: compilaciones y diagnósticos.** GitHub Actions verifica el código y empaqueta las compilaciones con sumas de verificación. `GET /api/hardware` informa los resultados de la inicialización del software. La salida de la pantalla, la respuesta táctil y el emparejamiento se pueden probar en la placa física. [Construir y lanzar](STAGE12-RELEASE.md) · [Pruebas de hardware](STAGE12-HARDWARE-TESTS.md) · [Diagnóstico de hardware](STAGE2-HARDWARE-DIAGNOSTICS.md)

El punto de entrada del firmware es `src/main.cpp`. Los encabezados se encuentran en `include/` y la fuente del panel de control es `web/stach-panel.html`. Después de editar el panel, actualice `preview/stach-panel.html` para que coincida y luego ejecute `python tools/build_web_header.py`. Ejecute `python tools/check_panel.py` para verificar la página integrada y el contrato de API (requiere Node.js). En Linux, ejecute `sh tools/test_core.sh` para las pruebas principales portátiles. Ejecute `python tools/test_ble_patch.py` para comprobar la compatibilidad del parche BLE.

STACH es un proyecto independiente inspirado en el género Pip-KeyPet. PlatformIO instala bibliotecas de terceros y el proyecto aplica un parche de compatibilidad a ESP32 BLE Mouse 0.3.1. [Recursos de Waveshare 1.75](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.75) · [Referencia de Waveshare 1.43](https://www.waveshare.com/wiki/ESP32-S3-Touch-AMOLED-1.43)
