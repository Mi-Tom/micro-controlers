# POZNÁMKY
---
## Součástky
- Hlavní řídící mikrokontroler dronu
    - **FlyWarp F405 V5 STACK**
        - Hlavní řídící čip
        - Firmware: FLYWOOF405, verze 4.5.3
            - **STEM32F405RGT6**     
        - Gyroskop
            - **ICM42688**
- ESC
    -  **BLS 65A**

- Gyroskop + Akcelerometr
		- **MPU6500**
				- POZOR, NENÍ TO 6050!!!
				- Vzorec pro Kalmanův filtr (Hodnoty budou přesné a stabilní --> dron bude ve vzduchu stabilně létat)
---
		- Výpočet úhlu z akcelerometru:
				roll_acc  = atan2(ay, az) * 180 / π
				pitch_acc = atan2(-ax, sqrt(ay² + az²)) * 180 / π
		
		- Integrace gyroskopu:
				roll_acc  = atan2(ay, az) * 180 / π
				pitch_acc = atan2(-ax, sqrt(ay² + az²)) * 180 / π
			
		- Complementary filter (hlavní vzorec):
				roll  = α * (roll  + gx * dt) + (1 - α) * roll_acc
				pitch = α * (pitch + gy * dt) + (1 - α) * pitch_acc

		- Co znamenají symboly:
				α = 0.95 až 0.99   (váha gyra)
				dt = čas v sekundách mezi měřeními

				gx, gy = gyro rychlost (°/s)
				ax, ay, az = akcelerace

		- Chyba pro PID regulátor (pro dron):
				error_roll  = target_roll  - roll
				error_pitch = target_pitch - pitch

		- Kalman:
				angle = angle + dt * (gyro - bias)
				angle = angle + K * (acc_angle - angle)
---

## Kdo co dělá:
---
### Gabriel
- Návrh obalu na kameru (k dronu) - začátek v úterý
---

### Dominik
- Návrh kouzelné tyčky (včetně softwaru)
---

### Honza
- Návrh dronu (postupně) -- Rozpracovano ve stredu, je navrzeno telo
---

## Rozměry některých součástek:
### MCU
- 36,42 mm x 36,49 mm (36,5 mm)
---

## Motor
- Průměr: 29,72 mm
- Spodek: 16,31 mm x ----
- Šrouby: Výška - 9,49 mm, Průměr - 3 mm (2,87 mm)
---

## Co je potřeba udělat (pokud se nudíš)
- Napsat Git
- Naprogramovat TempleOS
