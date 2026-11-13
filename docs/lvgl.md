# LVGL 9.6: minimale kwalificatie op vijf HUB75-panelen

De derde PlatformIO-omgeving voegt LVGL toe aan de **werkende vijfpanelenopstelling**. Dit is een gerichte kwalificatie van buildtijd, firmwaregrootte, heap en RGB565-flush; het is nog niet de applicatie uit Experiment 009.

## Omgevingen

| Environment | Functie |
| --- | --- |
| `esp32dev` | Minimale Arduino + UART baseline |
| `esp32dev-hub75` | Vijf panelen, zelfgetekende kleur-/oriëntatietest |
| **`esp32dev-hub75-lvgl`** | Vijf panelen, LVGL 9.6, één header en drie tekstregels |

```powershell
pio run -e esp32dev-hub75-lvgl
pio run -e esp32dev-hub75-lvgl -t upload
pio device monitor -b 115200
```

Bij het opstarten hoort een **limegroene balk met 'LVGL 9.6'** te verschijnen en daaronder drie regels: `5x HUB75 / 160x64`, `RGB565 10MHz` en `BRIGHTNESS: 10%`.

De bedrading, paneelmapping en DMA-configuratie zijn ongewijzigd. De HUB75-klok blijft **10 MHz**; `setBrightness8(24)` blijft circa **9,4%**. De native mapper in `include/hub75_geometry.h` voert de RGB565-pixels naar de bestaande elektrische 320×32 DMA-keten.

### RGB565 versus HUB75-kleurdiepte

**LVGL rendert en buffert RGB565 (16 bit per pixel)**. In de flush-callback
worden de ruwe `uint16_t`-pixels na de coördinatentransformatie rechtstreeks
met `MatrixPanel_I2S_DMA::drawPixel(x, y, packed565)` doorgestuurd.
De HUB75-driver biedt inderdaad deze RGB565-pixel-API, ook met `NO_GFX`.
Hij zet die intern om in RGB-kanaalwaarden en schrijft ze in zijn
DMA-bitplanes. Een RGB565-buffer wordt dus **niet letterlijk als 16-bit
signaal naar het paneel gestuurd**: de panelen zijn seriële HUB75-matrices,
met BCM/PWM voor de helderheid. De ingestelde `PIXEL_COLOR_DEPTH_BITS=6`
bepaalt hoeveel bitplanes de DMA-driver intern gebruikt.

De afzonderlijke `hub75_demo.cpp` blijft voor eenvoudige volle RGB-kleuren
`fillScreenRGB888` gebruiken: dat is een kleurtest, geen LVGL-renderpad.


## Bewust minimale LVGL-configuratie

De library staat vast op **Git-tag `v9.6.0`**. Het volledige configuratiebestand `include/lv_conf.h` wordt via `-DLV_CONF_INCLUDE_SIMPLE -Iinclude` aan ALLE LVGL-bronbestanden doorgegeven.

- **RGB565** software-renderer met een **160×8×2 = 2.560 bytes** gedeeltelijke tekenbuffer (geen GUI-framebuffer van 160×64).
- **C library `malloc`** om de standaard statische LVGL-heap van 64 KiB te voorkomen.
- **Één bitmapfont**: `UNSCII_8`; standaard-Montserrat uit.
- Alleen **labelwidget**; alle andere widgets uit.
- Themes, Flex, Grid, complex software drawing, vector graphics, logging en demos uit.
- Geen Adafruit GFX, WiFi, externe renderers of ingebouwde LVGL-simulator.

Een minimale `lv_conf.h` alléén is niet voldoende voor korte buildtijden:
PlatformIO compileerde aanvankelijk **551 LVGL-bronbestanden**, inclusief
Linux-drivers, 3D-libraries, thema's, andere widgets en backends.

Daarom gebruikt alleen de LVGL-omgeving de controleerbare PRE-buildmiddleware
`scripts/lvgl_source_filter.py`. Die heeft een expliciete lijst van vereiste
LVGL-coremodules, RGB565-softwaredrawing, de UNSCII-8-font en de labelwidget.
De upstream-library en de geïnstalleerde `library.json` worden **niet**
aangepast. CI controleert bovendien dat niet onverwacht meer dan 140
LVGL-bronbestanden worden gebouwd.

### Eerste meting van het optimalisatie-effect

| Meting | Zonder bronfilter | Met bronfilter |
| --- | ---: | ---: |
| LVGL-bronbestanden gecompileerd | **551** | **117** |
| Schone GUI-build (tools geïnstalleerd) | 52,68 s | **21,24 s** |
| Ongewijzigde GUI-build | 6,81 s | **2,78 s** |
| Firmwareflash | 728.212 bytes | 728.212 bytes |
| Statische RAM | 27.820 bytes | 27.820 bytes |

Dit zijn twee afzonderlijke, geslaagde GitHub Actions-runs op Ubuntu;
de kloktijden zijn dus indicatief. Het resultaat laat zien dat onnodige
compilatie kan worden weggefilterd zonder het firmwaregedrag te wijzigen.
De resterende LVGL-core kost nog steeds relatief veel flashgeheugen en
moet bij uitbreiding worden bewaakt.

## CI-meting

GitHub Actions bouwt de drie configuraties op dezelfde Ubuntu-runner. Voor de nieuwe LVGL-omgeving worden de initiële build (incl. library download), schone rebuild met geïnstalleerde dependencies, incrementele build en RAM/flash gerapporteerd. De bestaande baseline/HUB75-builds blijven ongewijzigd.

## Hardwaregrenzen

De CI-build bewijst dat compiler, LVGL-configuratie en API's samenwerken. **Of de ESP32-WROOM de LVGL-heap tijdens runtime en het verversen van alle vijf panelen probleemloos aankan, moet op de fysieke opstelling worden gecontroleerd.** De seriële monitor toont de vrije heap en DMA-geschikte heap, zodat dit direct zichtbaar is.
