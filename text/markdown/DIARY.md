# Deník
## 5. března 2026
- vytvořen tým 
- vytvořen git repozitář

## 13. března 2026
- seznámení s programem Fusion 360
- instalace a seznámení s programy pro programování mikrokontrolerů
- Gaba má Zadig na svém počítači
- seznámení s konfigurací pinů,... na STM32

## 14. března 2026
- přehrán firmware mikrokontroleru (nalezen původní)
- zjištěna závada mikrokontroleru - nefunguje gyroskop (ale funguje barometr)
- začátek práce na 3D modelech
- začátek návrhu schéma motion controlleru (Kouzelné tyčky)
- zjištěny díly, které jsou potřeba k motion controlleru
- návrh umístění kamery do dronu

## 15. března 2026
- začátek návrhu dronu 
- navržena základní kostra dronu
- návrh držáku na kameru pro dron

## 16. března 2026
- otestován senzor MPU6500
- začátek prací na softwaru pro motion controller
- ladicí práce na držáku na kameru pro dron

## 17. března 2026
- Zjištěno, že senzor je 6500 a ne 6050
- otestován první program - teď to funguje
- 1. ÚSPĚCH - Dostali jsme první data z gyroskopu a akcelerometru
- Zjištěno, jak funguje Kalmanův filtr - připsány hodnoty do NOTES.md
- Zapsány ukázkové vzorce
- Zprovoznen provizorni kod pro komplementarni filtr

## 18. března 2026
- Zdokonaleno nacitani dat z gyroskopu a akcelerometru
- Pridana funkce pro potenciometr, ktery ovlada rychlost dronu (zatim bez zpetneho chodu a omezeni)
- Pridan obvod referencniho napeti, zjisteno napeti a stav nabiti baterie
- Pridano tlacitko a LED pro zobrazeni stavu nabiti baterie
- Pridana komunikace ESP NOW --> nyni se posilaji finalni data z motion controlleru do ESP (2)
- Rozpracovan stojan na mobil, vysilac (3D model)
- Rozmontovan prijimac kamera --> Navrhnuta nova, lepsi krabicka 
