# ESP32 PlatformIO — minimal Arduino baseline

Een zelfstandig, reproduceerbaar **PlatformIO-project** voor de klassieke ESP32-WROOM-32 (`esp32dev`). Deze repository is een schone referentie voordat we HUB75, LVGL of onze gedeelde displaycode toevoegen.

## Projectstructuur

```text
.
├── .github/workflows/build.yml
├── .gitignore
├── platformio.ini
└── src/main.cpp
```

De enige firmwarefunctionaliteit is `Serial` op 115200 baud: de ESP32 meldt `ESP32 baseline ready` en echoot alle ontvangen bytes.

## PlatformIO IDE in VS Code

Open **de rootmap van deze repository** in VS Code (de map waarin `platformio.ini` staat). Installeer zo nodig de extensie **PlatformIO IDE**. Kies **Build**, **Upload** of **Monitor** in de PlatformIO-projecttaken.

De configuratie gebruikt bewust:
- `board = esp32dev`;
- `framework = arduino`;
- `platform = espressif32@8.0.0` (vastgepind voor reproduceerbare builds).

Geen `lib_deps`, WiFi-gegevens, HAL-wrapper, eigen CMake-bestanden, LVGL of HUB75-driver.

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

## CI en kwalificatie

GitHub Actions compileert op Ubuntu zonder aangesloten hardware en rapporteert de firmwaregrootte. CI bevestigt alleen **compilatie**, niet dat upload en seriële echo op een fysieke ESP32 werken.

De Arduino-core bevat intern reeds gecompileerde ESP-IDF-onderdelen. Deze baseline bouwt geen aparte ESP-IDF-applicatie met onnodige frameworkcomponenten.

## Uitbreiden, uitsluitend na een werkende basis

1. Baseline compileren, uploaden en seriële echo controleren.
2. HUB75 DMA-library toevoegen en de toename in afhankelijkheden/build meten.
3. Eén paneel aansturen, daarna vijf panelen.
4. Vervolgens pas de RGB565-renderer en het seriële displayprotocol uit Experiment 009 integreren.

Houd de baseline klein: voeg nog geen display- of netwerkcode toe voordat de voorgaande stap is geverifieerd.
