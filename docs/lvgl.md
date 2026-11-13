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

## Bewust minimale LVGL-configuratie

De library staat vast op **Git-tag `v9.6.0`**. Het volledige configuratiebestand `include/lv_conf.h` wordt via `-DLV_CONF_INCLUDE_SIMPLE -Iinclude` aan ALLE LVGL-bronbestanden doorgegeven.

- **RGB565** software-renderer met een **160×8×2 = 2.560 bytes** gedeeltelijke tekenbuffer (geen GUI-framebuffer van 160×64).
- **C library `malloc`** om de standaard statische LVGL-heap van 64 KiB te voorkomen.
- **Één bitmapfont**: `UNSCII_8`; standaard-Montserrat uit.
- Alleen **labelwidget**; alle andere widgets uit.
- Themes, Flex, Grid, complex software drawing, vector graphics, logging en demos uit.
- Geen Adafruit GFX, WiFi, externe renderers of ingebouwde LVGL-simulator.

Let op: het uitschakelen van LVGL-modules in `lv_conf.h` minimaliseert vooral de **effectieve code en firmwaregrootte**, niet noodzakelijk het **aantal bronbestanden dat PlatformIO compileert**. Het CI-log laat daarom ook de totale bouwtijd zien. We voeren geen oncontroleerbare patches uit op de upstream-library.

## CI-meting

GitHub Actions bouwt de drie configuraties op dezelfde Ubuntu-runner. Voor de nieuwe LVGL-omgeving worden de initiële build (incl. library download), schone rebuild met geïnstalleerde dependencies, incrementele build en RAM/flash gerapporteerd. De bestaande baseline/HUB75-builds blijven ongewijzigd.

## Hardwaregrenzen

De CI-build bewijst dat compiler, LVGL-configuratie en API's samenwerken. **Of de ESP32-WROOM de LVGL-heap tijdens runtime en het verversen van alle vijf panelen probleemloos aankan, moet op de fysieke opstelling worden gecontroleerd.** De seriële monitor toont de vrije heap en DMA-geschikte heap, zodat dit direct zichtbaar is.
