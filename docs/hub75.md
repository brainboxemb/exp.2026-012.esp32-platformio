# HUB75 — eenpaneelskwalificatie

Deze omgeving kwalificeert alleen de afhankelijkheden en DMA-uitvoer voor **één elektrisch 64×32 HUB75-paneel**. De vijf-panelenmapping en de bestaande native displaycode vallen er nog buiten.

## PlatformIO-omgevingen

| Omgeving | Code | Afhankelijkheden |
| --- | --- | --- |
| `esp32dev` (standaard) | `src/main.cpp` | Arduino, seriële echo |
| `esp32dev-hub75` | `src/hub75_demo.cpp` | Arduino + HUB75 DMA 3.0.10 |

De HUB75 DMA-library wordt via de upstream Git-tag `3.0.10` opgehaald; de oude PlatformIO Registry-identificatie `mrfaptastic/...` is niet meer vindbaar. Dit is een vastgepinde versie, geen onbeheerde `master`-dependency.\n\nDe HUB75-test gebruikt `NO_GFX`. Daardoor is **Adafruit GFX niet nodig**: het voorbeeld gebruikt uitsluitend de native `fillScreenRGB888`-driverfunctie. Het uiteindelijke display kan later nog steeds een RGB565-framebuffer gebruiken.

## Compileren

```powershell
pio run -e esp32dev
pio run -e esp32dev-hub75
pio run -e esp32dev-hub75 -t size
```

Voor upload naar de aangesloten ESP32:

```powershell
pio run -e esp32dev-hub75 -t upload
pio device monitor -b 115200
```

Zorg dat je in VS Code bij **Project Tasks** expliciet `esp32dev-hub75` selecteert wanneer je deze firmware uploadt; de standaardomgeving blijft de seriële basistest.

## Bedrading (ESP32-WROOM)

De firmware gebruikt bewust de eerdere bedrading van het HUB75-project:

| Signaal | GPIO | Signaal | GPIO |
| --- | ---: | --- | ---: |
| R1 | 18 | G1 | 26 |
| B1 | 27 | R2 | 14 |
| G2 | 12 | B2 | 13 |
| A | 23 | B | 19 |
| C | 5 | D | 17 |
| LAT | 4 | OE | 15 |
| CLK | 16 | E | niet aangestuurd |

Voor een standaard 64×32-paneel wordt `E` niet aangestuurd. Verbind `E` alleen volgens de specificaties van jouw paneel (in de bestaande opstelling: GND). **GPIO12 en GPIO15 zijn ESP32-strapping-pinnen.** Als het paneel GPIO12 hoog houdt tijdens reset kan de ESP32 onjuist opstarten. Controleer daarom de elektrische aansluiting voordat je spanning aanlegt.

**Voeding:** geef het HUB75-paneel een geschikte afzonderlijke **5V-voeding**. Voed het paneel niet via een ESP32 GPIO of de 3,3V-pin; verbind wel de massa van paneel en ESP32 met elkaar.

## Verwacht gedrag

De seriële monitor toont `ESP32 HUB75 single-panel test`. Bij een geslaagde initialisatie toont het paneel achtereenvolgens **rood, groen, blauw, wit, zwart** (1,5 seconde per kleur). De helderheid staat bewust op 48/255. Een mislukte driverinitialisatie geeft een foutbericht en probeert niet opnieuw.

De software blijft een hardware-smoketest: CI kan alleen vaststellen dat de code compileert. Correcte scan-volgorde, kleurkanaalbedrading en het echte beeld moeten op het fysieke paneel worden gecontroleerd.

## Vergelijking met baseline

GitHub Actions bouwt `esp32dev` en `esp32dev-hub75` in dezelfde runner. De workflow meet voor beide omgevingen een **schone rebuild nadat de dependencies zijn geïnstalleerd**, daarnaast een incrementele build en de firmwaregrootte. De eerste HUB75-build installeert ook de extra library; verwar die downloadtijd niet met compileertijd.

De oorspronkelijke baseline vóór HUB75 had in CI:
- schone rebuild: **9,93 s**;
- geen wijzigingen: **1,15 s**;
- RAM **22.116 bytes**, flash **287.012 bytes**.

De getallen op de nieuwe CI-run zijn leidend voor een eerlijke vergelijking op dezelfde runner.
