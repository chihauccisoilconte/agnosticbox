# Agnostic Box PCB Bill of Materials

Source files checked:

- `agnostic box PCB.kicad_sch`
- `agnostic box PCB.kicad_pcb`

This BOM is based on the KiCad schematic symbols and PCB footprints. The schematic uses generic values for several parts, so values that are only visible in PCB reference labels are noted below.

## Components

| Qty | References | Component | Value / specification | Footprint / package | Notes |
| ---: | --- | --- | --- | --- | --- |
| 1 | A1 | Arduino Nano compatible module | Arduino Nano or Arduino Nano ESP32 | `Module:Arduino_Nano` | PCB silkscreen says `Arduino Nano (or Nano esp32)`. |
| 1 | U1 | GY-521 accelerometer/gyro module | GY-521 / MPU5060 label in schematic | `gy521:GY-521` | Manufacturer field: `GODREAM FORDREAM CO., LIMITED`; MP field: `GY-521`. |
| 1 | J1 | PAM8302A audio amplifier connection | PAM8302A | `Connector_PinSocket_2.54mm:PinSocket_1x05_P2.54mm_Vertical` | The PCB footprint is a 1x05 2.54 mm socket/header for the amplifier module. |
| 1 | BT1 | Battery connector | 2-pin battery connector | `Connector_PinSocket_2.54mm:PinSocket_1x02_P2.54mm_Vertical` | PCB silkscreen label: `battery`. |
| 3 | D1, D2, D3 | LED | 5 mm through-hole LED | `LED_THT:LED_D5.0mm` | PCB labels: `LED1(D4)`, `LED2(D5)`, `LED3(D6)`. LED color is not specified in the KiCad files. |
| 3 | R1, R2, R3 | Resistor | 220 ohm | `Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal` | The schematic value is generic `R`; `220` comes from PCB labels `R1-220`, `R2-220`, `R3-220`. |
| 3 | R4, R5, R6 | Resistor | 10k ohm | `Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal` | The schematic value is generic `R`; `10k` comes from PCB labels `R4-10k`, `R5-10k`, `R6-10k`. |
| 3 | RV1, RV2, RV3 | Potentiometer | Resistance value not specified | `Potentiometer_THT:Potentiometer_Alps_RK163_Single_Horizontal` | PCB labels: `RV1(A1)`, `RV2(A2)`, `RV3(A3)`. |
| 3 | SW1, SW2, SW3 | Push button | 6 mm through-hole push button | `Button_Switch_THT:SW_PUSH_6mm` | PCB labels: `SW1(D10)`, `SW2(D11)`, `SW3(D12)`. |

## PCB-Only Mechanical Features

| Qty | Item | Footprint | BOM status |
| ---: | --- | --- | --- |
| 3 | M3 countersunk mounting holes | `MountingHole:MountingHole_3.2mm_M3_DIN965` | Excluded from KiCad BOM; no screws or hardware values are specified in the board file. |

## Notes

- No manufacturer part numbers are specified for the resistors, LEDs, switches, potentiometers, battery connector, Arduino module, or PAM8302A connector in the KiCad schematic.
- The resistor values `220 ohm` and `10k ohm` were taken from PCB reference labels, not from schematic value fields.
- Potentiometer resistance values are not specified in either the schematic values or PCB labels.
