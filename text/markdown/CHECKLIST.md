# 1. Mechanická kontrola dronu:
	- Zkontrolovat šrouby (motory, ramena, FC/ESC, držák baterie)
	- Zkontrolovat vrtule (orientace, upevnění, praskliny)
	- Kabeláž (poškození, kolem motorů, jestli je vše zapojené,...)

# 2. Kontrola elektroniky:
	- Ověření všeho multimetrem
	
- Jelikož nemáme GPS tak musíme udělat tohle: iNAV má v základu nastaveno, že ti nedovolí odarmovat, pokud nemá GPS fix. To musíš vypnout.

    Checklist update: V CLI (příkazová řádka) zadej:
    set nav_extra_arming_safety = OFF
    save
    Bez tohoto příkazu dron nenastartuješ, protože bude stále marně hledat satelity.

OPRAVA MOTORŮ:
	- otevřít Mixer
	- Output Mapping:
		- nastavit S1 na Motor 3
		- nastavit S2 na Motor 4
		- nastavit S3 na Motor 1
		- nastavit S4 na Motor 2
	- POTÉ OTESTOVAT, JESTLI TO FUNGUJE

# 3. Konfigurace ve flight controlleru:
## Před konfigurací
    - Sundat vrtule při všech bench testech
    - Zapnout rádio před připojením baterie
    - Ověřit správný target a verzi firmware
    - Po flashi:
        - Full chip erase
        - neobnovovat dump z jiného targetu
##Setup Tab
    - Orientace FC a senzory
    - Zkontrolovat orientaci modelu
    - Náklon doprava na obrazovce = náklon doprava v realitě
    - Zvednutí nosu = zvednutí nosu v realitě
    - Yaw musí odpovídat skutečnosti
    - Důkladně zkontrolovat všechny osy
    - Případná oprava:
        - Configuration → Board and Sensor Alignment
    - Kalibrace
        - Kalibrace na dokonale rovné ploše
        - Nekalibrovat v ruce
        - Bez vibrací
    - Zkalibrovat:
        - Accelerometer
        - Kontrola senzorů
        - Gyro nesmí mít extrémní drift
        - Horizon musí být rovný
        - Model se nesmí výrazně samovolně otáčet
    - CPU Load
        - Zkontrolovat CPU load
        - Ideálně pod 40–50 %
        - Pokud je vysoký:
            - snížit looptime
            - vypnout nepotřebné funkce
            - omezit filtry
## Ports Tab
    - UART konfigurace
    - Receiver:
        - zapnout Serial RX
        - pouze na správném UARTu
    - VTX control:
        - Peripherals → SmartAudio nebo IRC Tramp
    - Telemetry:
        - zapnout podle použitého receiveru
## Configuration Tab
### Mixer
    - Nastavit:
        - Quad X
    - ESC/Motor protocol
    - Používat:
        - DSHOT300 nebo DSHOT600
    - Nepoužívat:
        - PWM
        - OneShot
        - MultiShot
### Bidirectional DSHOT
    - Zapnout pouze pokud ESC podporuje:
        - Bluejay
        - BLHeli32
### Motor Direction
    - Vybrat:
        - Props In
        - nebo Props Out
    - Musí souhlasit:
        - směr motorů
        - diagram
        - orientace vrtulí
### Receiver protocol
    - ELRS:
        - CRSF
    - Ověřit správný protocol podle receiveru
### Air Mode
    - Doporučeno zapnout
    - Lepší kontrola při nízkém throttle
### Dynamic Idle
    - Nechat default
### Motor Output Limit
    - Pro maiden možno omezit:
        - 70–80 %
        - Dron bude klidnější
### Arming
    - Nechat základní safety checks:
        - gyro
        - throttle
        - failsafe
        - accelerometer
### Battery Settings
    - Warning voltage:
        - 3.5 V/cell
    - Minimum voltage:
        - 3.3 V/cell
    - Ověřit počet článků
### Current Sensor
    - Ověřit:
        - current reading
        - mAh consumed
        - Zkalibrovat current scale
### OSD
    - Zapnout OSD feature
    - Doporučené OSD prvky
        - Voltage
        - Average cell voltage
        - mAh consumed
        - RSSI
        - LQ
        - Warnings
        - Timers
        - Craft name
        - Flight mode
## Receiver Tab
    - Ověření rádia
    - Zkontrolovat všechny kanály
    - Ověřit správné směry
    - Reverz
    - Pokud je něco obráceně:
        - opravit v rádiu
        - ne v iNavu
    - Endpoints
        - Min:
        - 1000
        - Mid:
        - 1500
        - Max:
        - 2000
    - AUX kanály
    - Ověřit funkci všech switchů
        - RSSI / LQ
    - Ověřit:
        - mění se hodnoty
        - telemetry funguje
### Receiver failsafe
    - Vypnout rádio
    - Zkontrolovat:
        - receiver přejde do failsafe
        - kanály nezůstávají zamrzlé na poslední hodnotě
## Modes Tab
    - PREARM
        - Doporučeno
        - Extra ochrana proti nechtěnému armu
    - ARM
        - Hlavní arm switch
    - ANGLE Mode
        - Doporučeno pro maiden
    - HORIZON Mode
        - Volitelné
    - BEEPER
        - Velmi doporučeno
    - TURTLE MODE
        - Pouze pokud chceme
        - Nepoužívat
    - RTH
    - POS HOLD
    - CRUISE
    - NAV modes
## Motors Tab
    - DŮLEŽITÉ
    - PŘED TESTY SUNDAT VRTULE
    - Stabilizační reakce motorů
    - Naklonit dron rukou
    - Motory musí reagovat PROTI pohybu
    - Pokud reagují špatně:
        - špatná orientace FC
        - špatný mixer
        - hrozí instant flip of death
    - Pořadí motorů
    - Motor slider 1:
        - musí roztočit správný motor
        - Pokud ne:
        - wiring problém
        - resource remap
    - Směr motorů
        - Musí odpovídat diagramu
    - Idle
    - Typicky:
        - 5–7 %
    - Příliš nízký:
        - desync
        - padání při prudkých manévrech
    - Příliš vysoký:
        - tvrdé přistání
        - nestabilita
    - Kontrola vibrací
    - Motory nesmí:
        - vibrovat
        - drhnout
        - přeskakovat
    - Kontrola teplot
        - Motory nesmí být horké po krátkém běhu
## ESC konfigurace
    - Pokud používáme DSHOT
    - ESC calibration se NEDĚLÁ
    - ESC software
        - BLHeliSuite32
        - Bluejay
    - Nastavení
        - Motor timing:
            - default
        - Beacon:
            - doporučeno
        - Direction:
            - normal/reversed
        - RPM telemetry:
            - pouze při bidirectional DSHOT
## Failsafe
    - Nejdůležitější bezpečnostní věc
    - Test
        - Bez vrtulí
        - Lehce throttle
        - Vypnout rádio
        - Správné chování
            - Motory se zastaví
        - Špatné chování
            - Motory běží dál
        - Doporučení
            - FPV quad bez GPS
            - DROP
        - Nedoporučeno
            - LAND
## VTX
    - DŮLEŽITÉ
    - Anténa MUSÍ být připojená
    - Output power
        - Maiden:
            - 25–100 mW
    - Pit mode
        - Doporučeno při bench testech
    - Channel/Band
        - Ověřit správný kanál v goggles
    - Filtry
        - Nechat default
        - Netunit před maidenem
## PID tuning
    - Pro maiden:
        - default
    - Nic neladit
## Vibrace
    - iNav je citlivý na vibrace
    - Ověřit:
        - soft mounting FC
        - vrtule
        - motor bells
        - resonance frame
    - Hover nesmí:
        - oscilovat
        - toilet bowl efekt
        - samovolně driftovat extrémně
## Blackbox
    - Pokud FC podporuje:
        - zapnout blackbox logging
## CLI backup
	- Po dokončení:
	    - Příkaz: "set nav_extra_arming_safety = OFF" a "save"
	    - Příkaz: "status"
		- Příkaz: "dump all"

# 4. ESC a motory:
	- Kalibrace ESC
	- Nesmí se přehřívat, vibrovat,...
	- Kontrola teploty motorů

# 5. Kontrola baterie:
	- Nabití baterie
	- Uchycení baterie

# 6. Otestování radio spojení:
	- Zkouška, jestli se připojení neztratí
	- Ověřit dosah
	- Kontrola, jestli se posílají data přes ESP-NOW
