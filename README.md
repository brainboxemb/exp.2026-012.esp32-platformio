# ESP32 PlatformIO — minimal Arduino baseline

Een zelfstandig, reproduceerbaar **PlatformIO-project** voor de klassieke ESP32-WROOM-32 (`esp32dev`). De standaardomgeving blijft een minimale Arduino-referentie. Een tweede, optionele omgeving test afzonderlijk één HUB75-paneel, zonder LVGL of de gedeelde displaycode.

## Projectstructuur

```text
.
├── .github/workflows/build.yml
├── .gitignore
├── docs/hub75.md
├── platformio.ini
└── src/
    ├── main.cpp             # standaard: seriële echo
    └── hub75_demo.cpp       # optioneel: één HUB75-paneel
```

De enige firmwarefunctionaliteit is `Serial` op 115200 baud: de ESP32 meldt `ESP32 baseline ready` en echoot alle ontvangen bytes.

## PlatformIO IDE in VS Code

Open **de rootmap van deze repository** in VS Code (de map waarin `platformio.ini` staat). Installeer zo nodig de extensie **PlatformIO IDE**. Kies **Build**, **Upload** of **Monitor** in de PlatformIO-projecttaken.

De configuratie gebruikt bewust:
- `board = esp32dev`;
- `framework = arduino`;
- `platform = espressif32@8.0.0` (vastgepind voor reproduceerbare builds).

De **standaardomgeving** heeft geen `lib_deps`, WiFi-gegevens, HAL-wrapper, eigen CMake-bestanden of LVGL. De losstaande omgeving `esp32dev-hub75` voegt uitsluitend de HUB75 DMA-driver toe.

## Terminalcommando's

Voer uit in de repositoryroot:

```powershell
pio run -e esp32dev
pio run -e esp32dev -t size
pio run -e esp32dev -t upload
pio device monitor -b 115200
```

Na het openen van de monitor kun je op de **RESET/EN-knop** drukken om de melding `ESP32 baseline ready` opnieuw te zien. Typ of plak tekst: deze wordt teruggestuurd (echo).

Een nieuwe schone firmwarebuild:

```powershell
pio run -e esp32dev -t clean
pio run -e esp32dev
```

De eerste build downloadt de toolchain en de Arduino-core. Die initiële installatie staat los van de hoeveelheid applicatiecode.

## Optionele HUB75-test

De standaardbuild (`pio run`) blijft ongewijzigd. Voor **één 64×32-paneel**, zonder Adafruit GFX:

```powershell
pio run -e esp32dev-hub75
pio run -e esp32dev-hub75 -t upload
```

Zie [de HUB75-kwalificatie](docs/hub75.md) voor de bestaande GPIO-bedrading, voeding en verwachte kleuren.

## CI en kwalificatie

GitHub Actions compileert op Ubuntu zonder aangesloten hardware, vergelijkt de standaardomgeving met de HUB75-test en rapporteert de buildtijd en firmwaregrootte. CI bevestigt alleen **compilatie**, niet dat upload en seriële echo op een fysieke ESP32 werken.

De Arduino-core bevat intern reeds gecompileerde ESP-IDF-onderdelen. Deze baseline bouwt geen aparte ESP-IDF-applicatie met onnodige frameworkcomponenten.

## Buildtijden meten

De CI-workflow meet drie situaties **in dezelfde Ubuntu-runner**:

| Meting | Betekenis |
| --- | --- |
| Initial build | Download/installatie van platform, Arduino-core en toolchain **plus** de eerste compilatie |
| Clean rebuild | `pio run -t clean`, daarna alle projectbestanden opnieuw compileren met de tools al geïnstalleerd |
| No-change build | Meteen nogmaals `pio run` uitvoeren zonder bronwijzigingen (incrementeel) |

De exacte tijden verschijnen onder **Actions → Build ESP32 baseline → betreffende run → Summary**. Deze meting voorkomt dat downloadtijd wordt verward met compileertijd. De tweede meting sluit de tijd van het `clean`-commando zelf uit.

Op je eigen computer kun je een schone herbouw meten met PowerShell:

```powershell
pio run -e esp32dev -t clean
Measure-Command { pio run -e esp32dev }
```

En daarna een incrementele build:

```powershell
Measure-Command { pio run -e esp32dev }
```

De werkelijke tijden verschillen per laptop en afhankelijk van toolchain-cache en antivirus.

## Uitbreiden, uitsluitend na een werkende basis

1. Baseline compileren, uploaden en seriële echo controleren.
2. HUB75 DMA-library in een **afzonderlijke omgeving** bouwen en de toename in afhankelijkheden/build meten (deze stap is beschikbaar voor CI- en hardwarekwalificatie).
3. Eén paneel aansturen, daarna vijf panelen.
4. Vervolgens pas de RGB565-renderer en het seriële displayprotocol uit Experiment 009 integreren.

Houd de baseline klein: voeg nog geen display- of netwerkcode toe voordat de voorgaande stap is geverifieerd.
