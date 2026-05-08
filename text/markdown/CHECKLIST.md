# 1. Mechanická kontrola dronu:
	- Zkontrolovat šrouby (motory, ramena, FC/ESC, držák baterie)
	- Zkontrolovat vrtule (orientace, upevnění, praskliny)
	- Kabeláž (poškození, kolem motorů, jestli je vše zapojené,...)

# 2. Kontrola elektroniky:
	- Ověření všeho multimetrem

# 3. Konfigurace ve flight controlleru:
## Setup Tab
	- orientace a senzory
	- Náklon doprava na obrazovce musí být náklon doprava v realitě!!!
	- Zvednutí nosu musí být zvednutí nosu!!!
	- Důkladně zkontrolovat!
	- případná oprava v Configuration --> Board and sensor alignment

	- Kalibrace akcelerometru!
	- Zkontrolovat gyro drift
		- model nesmí sám pomalu utíkat
	- Zkontrolovat CPU load
		- neměl by být příliš vysoký

## Ports Tab - UART
	- UART receiver - zapnout SERIAL RX (pouze na správném UART)
	- VTX control - peripherals --> SmartAudio

## Configuration Tab
### Air Mode
	- Doporučeno zapnout
	- Lepší kontrola při nízkém throttle
### Motor Output Limit
	- Pro první let možno omezit výkon:
		- např. 70-80 %
	- Dron bude klidnější
### Dynamic Idle
	- Nechat default
	- Důležité pro stabilitu
### OSD
	- Zapnout OSD feature
### Mixer
	- Nastavit Quad X (máme čtxři motory)
### ESC/Motor protocol
	- DSHOT (doporučeno DSHOT 300/DSHOT 600), NEPOUŽÍVAT PWM, OneShot, MultiShot
### Bidirectional DSHOT
	- POUZE pokud naše ESC podporuje Bluejay/BLHeli32 (Nemyslim si)
### Motor Direction
	- Props In (default) - přední vrtule točí dovnitř
	- Props Out - přední vrtule točí ven (dneska pry hodně popularni) -- Muzeme si vybrat co chceme
### Receiver protocol
	- Podle systému: ELRS - nastavit na CRSF (Podle me to mame)
### Arming
	- Minimum checks (kontrola gyro, throttle, failsafe, akcelerometr)
### Battery settings
	- Zkontrolovat baterku 
	- Warning voltage: 3,5 V/cell
	- Minimum voltage: 3,3 V/cell

## Power & Battery Tab
	- Zkontrolovat nepětí baterie

## Receiver Tab
	- Ověření radia
### AUX kanály
	- Ověřit funkci všech přepínačů
### RSSI / LQ
	- Ověřit:
		- mění se hodnoty
		- telemetry funguje
### Směry
	- Musí sedět směry natočení motion controlleru
### Reverz
	- Pokud je něco obráceně - oprava v radiu (ne v betaflightu)
### Endpoints
	- Min: 1000
	- Mid: 1500
	- Max: 2000 

## Modes Tab
	- Nastavení přepínačů
### PREARM
	- Doporučeno
	- Extra bezpečnost proti nechtěnému armu
### FLIP OVER AFTER CRASH
	- Turtle mode
	- Pouze pokud chceme
### ARM
	- Nejdůležitější switch
### ANGLE Mode
	- Dron se sám stabilizuje
	- Pro první let doporučené
### BEEPER
	- Extrémně užitečné při ztrátě dronu

## Motors Tab
	- PŘED TÍM SUNDAT VRTULE!!!
### Stabilizační reakce motorů
	- Naklonit dron rukou
	- Motory musí reagovat PROTI pohybu

	- Pokud reagují špatně:
		- špatná orientace FC
		- špatný mixer
		- hrozí instant flip of death
### Pořadí motorů
	- Kliknout na motor 1 - MUsí se roztočit správný motor
	- Pokud ne - wiring problém nebo musíme resource remapnout
### Směr motorů
	- Musí odpovídat diagramu (nevim jakymu)
### Idle
	- Nesmí být příliš nízký ani příliš vysoký
	- Příliš nízký - dron padá při prudkých manévrech
	- Příliš vysoký - nestabilita při přistání
	- Typicky 5-7 %
### Kontrola vibrací
	- Motory nesmí:
		- vibrovat
		- drhnout
		- přeskakovat

## ESC konfigurace
	- Pokud používáme DSHOT:
		- ESC calibration se NEDĚLÁ
	- Jinak:
	- Pomocí:
		- BLHeliSuite32
		- Bluejay
	- Nastavit:
		- Motor timing - nechat default
		- Beacon - pípání při neaktivitě
		- Direction - Reversed/normal
		- RPM telemetry - Zapnout, pokud použijeme bidirectional DSHOT

## Failsafe
	- Nejdůležitější bezpečnostní věc
	- Testovat:
		- Bez vrtulí
		- lehce throttle
		- vypnout radio
	- Správné chování:
		- motory se zastaví
	- Špatné chování:
		- motory běží dál
		
## VTX
	- Anténa MUSÍ být připojená
### Output power
	- Pro maiden:
		- 25-100 mW
### Pit mode
	- Doporučeno při bench testech
### Channel/Band
	- Ověřit správný kanál v goggles
## OSD
	- Přidat:
		- Average cell voltage
		- flight mode

## Filtry
	- Nechat default
	- Nic netunit před maidenem

## OSD
	- Doporučené prvky:
		- Voltage
		- mAH consumed
		- RSSI
		- LQ
		- warnings
		- timers
		- craft name

## PID tuning
	- Pro maiden: defaulty
	- Nic neladit

## Blackbox - DOPORUČENO
	- Pokud to FC podporuje:
		- zapnout blackbox

## CLI backup
	- Po dokončení:
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
