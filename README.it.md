# STACH

[Русский](README.md) · [English](README.en.md) · [简体中文](README.zh-CN.md) · [繁體中文](README.zh-TW.md) · [Español](README.es.md) · [हिन्दी](README.hi.md) · [العربية](README.ar.md) · [Français](README.fr.md) · [বাংলা](README.bn.md) · [Português](README.pt.md) · [Bahasa Indonesia](README.id.md) · [اردو](README.ur.md) · [Deutsch](README.de.md) · [日本語](README.ja.md) · [한국어](README.ko.md) · [Türkçe](README.tr.md) · [Tiếng Việt](README.vi.md) · **Italiano** · [فارسی](README.fa.md) · [Polski](README.pl.md) · [Українська](README.uk.md)

STACH trasforma uno **Waveshare ESP32-S3-Touch-AMOLED-1.75** in un compagno tascabile. I suoi occhi seguono il tuo dito e rispondono all'inclinazione. Scorri verso destra per giocare o utilizza il telefono per modificarne l'espressione, la luminosità, il comportamento e il nome visualizzato. La scheda ospita la propria rete Wi-Fi e il pannello di controllo.

![STACH eye animation](preview/stach-smart-eyes.gif)

## Installa su Waveshare 1.75

Queste istruzioni sono state testate su Windows con l'ambiente `stach-eyes-175` PlatformIO. Costruisce l'applicazione completa da `src/main.cpp` per il display rotondo 466 × 466 CO5300, 16 MB di flash e 8 MB di PSRAM. Controlla il modello sulla scheda o sulla sua confezione prima di eseguire il flashing. Le revisioni 1.43, 1.43B e 1.75C hanno hardware diverso.

### 1. Prepara il tuo computer

Per la prima build sono necessari Python 3.13 con il launcher `py`, Git per Windows, un cavo dati USB e una connessione Internet. [Python per Windows](https://www.python.org/downloads/windows/) · [Git per Windows](https://git-scm.com/downloads/win)

Apri PowerShell e clona il progetto in una nuova cartella. Un percorso breve con caratteri ASCII funziona bene con gli strumenti ESP32.

```powershell
cd $env:USERPROFILE
git clone https://github.com/leonidgg2323-code/stach.git stach-firmware
cd stach-firmware
py -3.13 -m venv .venv
.\.venv\Scripts\python.exe -m pip install "platformio==6.2.0"
```

In alternativa, scegli Codice **→ Scarica ZIP** su GitHub ed estrai l'archivio. Apri PowerShell nella cartella contenente `platformio.ini` ed esegui gli ultimi due comandi sopra. I comandi seguenti chiamano direttamente il Python dell'ambiente virtuale.

### 2. Installa le dipendenze e crea

```powershell
.\.venv\Scripts\python.exe -m platformio pkg install -e stach-eyes-175
.\.venv\Scripts\python.exe -m platformio run -e stach-eyes-175
```

La prima build scarica la toolchain ESP32 e può richiedere diversi minuti. La build applica automaticamente la patch di compatibilità della libreria del mouse Bluetooth. Attendi `stach-eyes-175  SUCCESS` prima del caricamento.

### 3. Trova la porta seriale e flasha la scheda

Collega la scheda tramite USB ed elenca le porte disponibili.

```powershell
.\.venv\Scripts\python.exe -m platformio device list
```

Identifica la porta che appare quando colleghi la scheda e scompare quando la scolleghi. Sostituisci `COM3` negli esempi con la tua porta. Chiudi qualsiasi programma che lo utilizza, incluso il monitor seriale IDE Arduino.

**Il caricamento sostituisce il firmware sulla scheda.** Per mantenere la possibilità di ripristinare il menu di fabbrica, salvare un backup flash completo all'esterno del repository prima del caricamento. Un backup completo per questo modello è **16,777,216 bytes**. Utilizza un nuovo nome file se hai già un backup. L'immagine può contenere impostazioni e credenziali, quindi mantienila privata.

```powershell
.\.venv\Scripts\python.exe -m pip install "esptool>=5,<6"
.\.venv\Scripts\python.exe -m esptool --chip esp32s3 --port COM3 read-flash 0 ALL "$env:USERPROFILE\stach-before-install.flash-backup.bin"
```

Per installare STACH, esegui il comando upload.

```powershell
.\.venv\Scripts\python.exe -m platformio run -e stach-eyes-175 -t upload --upload-port COM3
```

La scheda si riavvia dopo il controllo del checksum e il messaggio `SUCCESS`. PlatformIO scrive il bootloader, la tabella delle partizioni e l'applicazione nei rispettivi indirizzi. Il file `firmware.bin` nella directory build contiene solo l'applicazione.

### 4. Registra l'account proprietario

1. Al primo avvio, scansiona **1/2 JOIN WI-FI** con il tuo telefono. Unisciti alla rete `Stach-XXXXXX`. Se il tuo telefono segnala che non ha accesso a Internet, scegli **Resta connesso**.
2. Il display passa a **2/2 REGISTER** quando il telefono si connette. Se il primo codice è ancora visualizzato, tocca il display una volta. Scansiona il secondo codice con lo stesso telefono.
3. Scegli un nome utente composto da 3-32 lettere (A-Z o a-z), cifre, trattini o trattini bassi e una password di almeno 12 caratteri. Salva i tuoi dati di accesso. L'account e le impostazioni sono memorizzati sulla scheda.
4. In **Аккаунт владельца** (Account proprietario), è possibile modificare il nome visualizzato nel pannello o uscire. Per le visite successive, accedi al Wi-Fi della scheda e apri [pannello di controllo locale STACH](http://192.168.4.1/).

Il forum accetta un account proprietario. La registrazione richiede il codice monouso mostrato sul display fisico. I comandi di controllo richiedono una sessione del proprietario e un token di richiesta. Il pannello utilizza HTTP sulla rete Wi-Fi locale della scheda. Accedi di nuovo dopo il riavvio della bacheca.

## Gesti e giochi

| Azione | Risultato |
|---|---|
| Tocca gli occhi o inclina la tavola | Gli occhi seguono il dito o l'inclinazione. |
| Scorri verso destra dagli occhi | Apri il menu dei giochi. |
| Scorri verso sinistra in un gioco | Ritorna al menù. Scorri nuovamente verso sinistra per tornare agli occhi. |
| Tieni lo schermo per gli occhi per circa un secondo | Mostra il codice QR Wi-Fi. Toccare per alternare tra i codici di rete e quelli della centrale. Tieni premuto per tornare agli occhi. |
| Scorri verso l'alto dagli occhi | Avvia la modalità mouse Bluetooth. Accoppialo con **Stach Mouse** sul tuo computer e mantieni ferma la scheda per la calibrazione. |
| Tocca lo schermo in modalità mouse | Tocca per fare clic con il pulsante sinistro, tocca due volte per fare clic con il pulsante destro, scorri per scorrere o tieni premuto per uscire. |

In **Targets**, colpisci quanti più bersagli puoi in 20 secondi. In **Sums**, rispondi a cinque domande aggiuntive. Entrambi i giochi mostrano il tuo punteggio e salvano il tuo record personale. **Dice** lancia un dado a sei facce e **Coin** lancia una moneta. Ogni risultato proviene dal generatore di numeri casuali hardware ESP32. Tocca di nuovo al termine dell'animazione per un nuovo tiro o capovolgimento.

## Display e potenza

Il pannello del telefono include la selezione dell'espressione, un joystick per lo sguardo, controlli per l'ammiccamento e l'inclinazione, la luminosità degli occhi e un'impostazione per quanto gli occhi si muovono da soli. Le impostazioni dell'aspetto persistono dopo i riavvii. È possibile abilitare la modalità mouse dopo l'avvio.

Sulla scheda 1.75, **AXP2101** riporta la percentuale della batteria e lo stato di carica. L'indicatore mostra `USB` quando la scheda funziona con alimentazione via cavo e non rileva batteria. Con il risparmio energetico attivato, la frequenza di aggiornamento degli occhi diminuisce dopo 45 secondi di inattività e lo schermo si spegne dopo 5 minuti. Con una carica dichiarata pari o inferiore al 15%, tali intervalli scendono a 10 secondi e 2 minuti. Tocca lo schermo per riattivarlo. Giochi, codici QR, modalità mouse e ricarica attiva mantengono il display sveglio.

## Risoluzione dei problemi e ripristino

| Sintomo | Cosa provare |
|---|---|
| Non viene visualizzata alcuna nuova porta COM | Prova un cavo dati USB e un'altra porta USB, quindi esegui nuovamente `platformio device list`. |
| La porta non può essere aperta o segnala `Access denied` | Chiudere il monitor seriale. Ricollegare la scheda e verificare il suo numero COM. |
| La costruzione fallisce | Verifica che la cartella corrente contenga `platformio.ini`, quindi ripeti entrambi i comandi nel passaggio 2. |
| Lo schermo rimane vuoto | Controlla il modello della scheda e il nome dell'ambiente nel registro di caricamento. Per il board da 1,75, la linea di successo dovrebbe includere `stach-eyes-175`. |
| Il telefono non riesce ad aprire il pannello | Controlla che sia ancora su `Stach-XXXXXX` e apri `http://192.168.4.1/`. La prima registrazione richiede il secondo codice QR. |

Per visualizzare la diagnostica, aprire il monitor seriale a 115200 baud.

```powershell
.\.venv\Scripts\python.exe -m platformio device monitor -p COM3 -b 115200
```

Invia `I` per un report di inizializzazione hardware, `S` per elencare i dispositivi I²C o `D` per un test del display di cinque secondi. Premere **Ctrl+C** per chiudere il monitor.

Per ripristinare il backup completo sulla stessa scheda, chiudi il monitor e verifica prima la dimensione dell'immagine. Il comando seguente ripristina anche le impostazioni e i dati dell'account contenuti nel backup.

```powershell
.\.venv\Scripts\python.exe -m esptool --chip esp32s3 --port COM3 write-flash 0x0 "$env:USERPROFILE\stach-before-install.flash-backup.bin"
```

**Hai bisogno di aiuto?** Contatta `twiinkprostolox123` su Discord o [@def0kk (Telegram)](https://t.me/def0kk). Includere il modello della scheda, l'ambiente PlatformIO e il messaggio di errore. Mantieni privati ​​la password, il codice QR di registrazione e il backup flash completo.

## Storia e sviluppo del progetto

- **Fase 9 — Potenza.** I vecchi profili 1.43 misurano e filtrano la tensione ADC su GPIO4. Per stimare la carica è necessario controllare lo schema per la revisione specifica e calibrarlo rispetto alla tensione della batteria misurata. La sola alimentazione USB non conferma la ricarica. Il profilo 1.75 legge direttamente l'AXP2101.
- **Fase 10 — Modalità e personalizzazione.** Passa dal viso, al mouse Bluetooth, al QR Wi-Fi e al pannello QR dal display o tramite `POST /api/mode`. La luminosità degli occhi e l'attività dello sguardo variano dal 20% al 100%. Il controllo del mouse BLE richiede un profilo hardware.
- **Fase 11 — Affidabilità e sicurezza.** Il firmware controlla le origini delle richieste HTTP, protegge i comandi con un token di richiesta, limita le richieste e ripristina il punto di accesso Wi-Fi. Il profilo 1.75 richiede anche l'accesso del proprietario. [Note di sicurezza della fase 11](STAGE11-SECURITY.md)
- **Fase 12: build e diagnostica.** GitHub Actions controlla il codice e i pacchetti vengono compilati con checksum. `GET /api/hardware` riporta i risultati dell'inizializzazione del software. L'output del display, la risposta al tocco e l'accoppiamento possono essere testati sulla scheda fisica. [Costruisci e rilascia](STAGE12-RELEASE.md) · [Test dell'hardware](STAGE12-HARDWARE-TESTS.md) · [Diagnostica dell'hardware](STAGE2-HARDWARE-DIAGNOSTICS.md)

Il punto di ingresso del firmware è `src/main.cpp`. Le intestazioni risiedono in `include/` e la sorgente del pannello di controllo è `web/stach-panel.html`. Dopo aver modificato il pannello, aggiorna `preview/stach-panel.html` in modo che corrisponda, quindi esegui `python tools/build_web_header.py`. Esegui `python tools/check_panel.py` per controllare la pagina incorporata e il contratto API (richiede Node.js). Su Linux, esegui `sh tools/test_core.sh` per i test principali portatili. Esegui `python tools/test_ble_patch.py` per verificare la compatibilità della patch BLE.

STACH è un progetto indipendente ispirato al genere Pip-KeyPet. PlatformIO installa librerie di terze parti e il progetto applica una patch di compatibilità a ESP32 BLE Mouse 0.3.1. [Risorse Waveshare 1.75](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.75) · [Riferimento Waveshare 1.43](https://www.waveshare.com/wiki/ESP32-S3-Touch-AMOLED-1.43)
