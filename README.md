# ESP32 PlatformIO — minimal Arduino baseline

Een zelfstandig, reproduceerbaar **PlatformIO-project** voor de klassieke ESP32-WROOM-32 (`esp32dev`). De standaardomgeving blijft een minimale Arduino-referentie. Twee optionele omgevingen testen eerst de **bestaande vijf HUB75-panelen** zonder GUI en daarna dezelfde panelen met een **minimaal geconfigureerde LVGL 9.6 GUI**. De seriële Arduino-baseline blijft ongewijzigd.

## Projectstructuur

```text
.
├── .github/workflows/build.yml
├── .gitignore
├── docs/hub75.md
├── docs/lvgl.md
├── include/hub75_geometry.h
├── include/lv_conf.h
├── tests/test_hub75_geometry.cpp
├── platformio.ini
├── scripts/lvgl_source_filter.py
└── src/
    ├── main.cpp             # standaard: seriële echo
    ├── hub75_demo.cpp       # optioneel: bestaande vijf panelen
    └── hub75_lvgl_demo.cpp  # optioneel: vijf panelen met LVGL
```

De enige firmwarefunctionaliteit is `Serial` op 115200 baud: de ESP32 meldt `ESP32 baseline ready` en echoot alle ontvangen bytes.

## PlatformIO IDE in VS Code

Open **de rootmap van deze repository** in VS Code (de map waarin `platformio.ini` staat). Installeer zo nodig de extensie **PlatformIO IDE**. Kies **Build**, **Upload** of **Monitor** in de PlatformIO-projecttaken.

De configuratie gebruikt bewust:
- `board = esp32dev`;
- `framework = arduino`;
- `platform = espressif32@8.0.0` (vastgepind voor reproduceerbare builds).

De **standaardomgeving** heeft geen `lib_deps`, WiFi-gegevens, HAL-wrapper, eigen CMake-bestanden of LVGL. De losstaande omgeving `esp32dev-hub75` voegt uitsluitend de HUB75 DMA-driver toe. De derde omgeving `esp32dev-hub75-lvgl` voegt alleen de noodzakelijke LVGL-onderdelen toe.

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

De standaardbuild (`pio run`) blijft ongewijzigd. Voor **vijf 64×32-panelen** in de bestaande 160×64-opstelling, zonder Adafruit GFX:

```powershell
pio run -e esp32dev-hub75
pio run -e esp32dev-hub75 -t upload
```

Zie [de HUB75-kwalificatie](docs/hub75.md) voor de bestaande GPIO-bedrading, voeding en verwachte kleuren.

## Optionele GUI-test (LVGL 9.6)

De volgende stap gebruikt je bestaande vijfpanelenopstelling. De module
`esp32dev-hub75-lvgl` toont een klein testscherm met limegroene header,
drie labels en RGB565-uitvoer. De helderheid blijft ongeveer 10%.

```powershell
pio run -e esp32dev-hub75-lvgl
pio run -e esp32dev-hub75-lvgl -t upload
pio device monitor -b 115200
```

De GUI gebruikt alleen labels en de UNSCII-8-font, **geen themes, andere widgets,
Adafruit GFX of extra LVGL-renderers**. Configuratie: `include/lv_conf.h`.
De extra bronfilter `scripts/lvgl_source_filter.py` zorgt dat PlatformIO
niet alle 551 LVGL-bestanden bouwt; CI controleert de selectie.
Zie [docs/lvgl.md](docs/lvgl.md) voor de kwalificatie en de verwachte testweergave.

## CI en kwalificatie

GitHub Actions compileert op Ubuntu zonder aangesloten hardware, vergelijkt de standaardomgeving met de HUB75-test en de LVGL-GUI-test en rapporteert de buildtijd en firmwaregrootte. CI bevestigt alleen **compilatie**, niet dat upload en seriële echo op een fysieke ESP32 werken.

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
3. De bestaande vijfpanelenopstelling (160×64) testen op kleur, richting, scan en DMA-geheugen.
4. LVGL als optionele, minimale GUI-library op de vijf panelen kwalificeren.
5. Pas daarna de bestaande renderer en het seriële displayprotocol uit Experiment 009 integreren.

Houd de baseline klein: voeg nog geen display- of netwerkcode toe voordat de voorgaande stap is geverifieerd.
