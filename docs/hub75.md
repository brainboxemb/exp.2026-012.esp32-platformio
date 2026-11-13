# HUB75 — vijf panelen (bestaande opstelling)

Deze omgeving is bedoeld voor de **bestaande opstelling met vijf onderling doorverbonden 64×32 HUB75-panelen**. Je hoeft geen paneel los te koppelen. Het is nog steeds een kleine hardwarekwalificatietest, geen race-displayfirmware.

## Afmetingen en mapping

- Elektrisch: **5 × 64×32** HUB75, doorverbonden.
- Fysiek staan de vijf panelen in één kolom: **64×160 pixels**.
- De complete opstelling is 90° gedraaid, waardoor het display logisch **160×64 pixels** is.
- HUB75 DMA ziet de fysieke keten als **320×32**.
- Mapping: equivalent aan `CHAIN_TOP_RIGHT_DOWN` en `setRotation(1)` (90°), uitgaande van de bestaande keteningang **achteraan links-onder**.
- Onder `NO_GFX` past de demo deze mapping rechtstreeks toe zonder Adafruit GFX. De pure C++ mapping staat in `include/hub75_geometry.h`.

Dit is de bestaande **verwachte** oriëntatie, nog niet op de fysieke opstelling bewezen. De testbeelden laten onmiddellijk zien of de volgorde, spiegeling of rotatie afwijkt.

## PlatformIO-omgevingen

| Omgeving | Code | Afhankelijkheden |
| --- | --- | --- |
| `esp32dev` (standaard) | `src/main.cpp` | Arduino, seriële echo |
| `esp32dev-hub75` | `src/hub75_demo.cpp` | Arduino + HUB75 DMA 3.0.10 |

De HUB75 DMA-library komt van de vastgepinde upstream Git-tag `3.0.10`. Dankzij `NO_GFX` heeft dit project **geen Adafruit GFX-library** nodig. Er wordt geen LVGL, WiFi of applicatieframebuffer gekoppeld.

## Compileren en uploaden

Vanuit de repositoryroot:

```powershell
pio run -e esp32dev-hub75
pio run -e esp32dev-hub75 -t upload
pio device monitor -b 115200
```

In VS Code: **PlatformIO → Project Tasks → esp32dev-hub75 → Upload and Monitor**. De standaardconfiguratie `esp32dev` blijft de schone seriële Arduino-baseline.

## Bedrading (ESP32-WROOM)

De software gebruikt de eerder gekozen GPIO-verdeling:

| Signaal | GPIO | Signaal | GPIO |
| --- | ---: | --- | ---: |
| R1 | 18 | G1 | 26 |
| B1 | 27 | R2 | 14 |
| G2 | 12 | B2 | 13 |
| A | 23 | B | 19 |
| C | 5 | D | 17 |
| LAT | 4 | OE | 15 |
| CLK | 16 | E | niet aangestuurd |

Bij de 64×32-panelen wordt `E` niet aangestuurd (in de bestaande opstelling naar GND). **GPIO12 en GPIO15 zijn ESP32-strapping-pinnen**; controleer vooral dat GPIO12 bij reset niet door het paneel hoog wordt gehouden. De HUB75-panelen moeten een geschikte, voldoende zware **5V-voeding** krijgen, met gemeenschappelijke GND met de ESP32. Voed ze niet via de ESP32.

## Testbeelden

De firmware wisselt elke twee seconden automatisch tussen:

1. **Vijf kleurvlakken** (rood, groen, blauw, cyaan, oranje), elk op één logisch 32×64-paneel; asymmetrische witte hoekmarkeringen geven de bovenrichting aan.
2. **Oriëntatietest**, met groene bovenlijn, blauwe onderlijn, rode linkerrand, gele rechterrand en witte diagonaal.
3. Alle vijf rood.
4. Alle vijf groen.
5. Alle vijf blauw.
6. Alle vijf zwart.

De helderheid begint op **24/255** om de voedingsbelasting te beperken. Bij initialisatie rapporteert de seriële monitor de hoeveelheid DMA-geschikt geheugen voor en na het aanmaken van de HUB75-driver. Het programma meldt wanneer DMA-initialisatie mislukt, in plaats van door te gaan met een ongeldig display.

Vijf panelen kosten aanzienlijk meer DMA-geheugen dan één paneel. De CI-firmwaregrootte bewijst **niet** dat de ESP32-WROOM voldoende intern DMA-geheugen heeft voor deze configuratie; dat vereist de fysieke proef.

## Controle van de geometrie

`tests/test_hub75_geometry.cpp` is een native test van de coördinatentransformatie. Hij controleert de vier hoeken, grenzen, individuele 32px-panelstroken en dat **alle 10.240 logische pixels exact één uniek DMA-pixel adresseren**.

GitHub Actions voert deze mappingtest uit voordat de PlatformIO-builds starten. De test bewijst de softwarematige coördinatenmapping, **niet** de fysieke kabel- of paneeloriëntatie.

## Vergelijking met baseline

GitHub Actions bouwt nog steeds twee omgevingen op dezelfde runner: minimale `esp32dev` en `esp32dev-hub75`. De workflow meet schone en incrementele builds en rapporteert de firmwaregrootte. Daarmee blijft de oorspronkelijke Arduino-baseline vergelijkbaar, terwijl de hardwaretest representatief is voor de echte vijfpanelenopstelling.
