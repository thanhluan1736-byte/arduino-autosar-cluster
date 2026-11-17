# arduino-autosar-cluster

A mini **AUTOSAR Classic instrument cluster**, written bare metal in C, for two boards: an **Arduino Uno** (ATmega328P, 8-bit AVR) and the **MaaZEDU S32K144** board (NXP S32K144, ARM Cortex-M4). Only the MCAL and a few hardware hooks differ; the RTE, the SWCs, the services, IoHwAb and the CDDs are the same source files for both. Neither build uses the Arduino core or the NXP SDK. Every layer is hand written, from the MCAL register drivers up to the application SWCs, and the RTE is generated from an ARXML model by arxml2rte, my own small RTE generator (a separate repository, not public yet). The generated RTE is committed in `src/Rte`, so the project builds without it.

It uses only parts from the Keywish RFID starter kit:

- a joystick as the accelerator and brake pedals
- a 28BYJ-48 stepper motor as the speedometer needle
- the LCD1602 with I2C backpack (PCF8574) for speed, odometer and trip
- the on-board LED and the kit's passive buzzer for the speed warning

## What it does

| At power-on | While driving |
|---|---|
| The LCD shows a start screen for 1.5 s | Joystick up = throttle, down = brake, centre = coasting |
| A 0.3 s beep and the needle sweep 0 → 240 km/h → 0 (buzzer and gauge self-test) | The needle shows the simulated vehicle speed |
| The overspeed LED lights during the sweep (bulb check) | Speed warning from 65 km/h: LED on, `!` on the LCD, and a chime that gets more urgent (table below) |
| `NvM: data lost` for 2 s if the odometer was lost from EEPROM | Short press of the joystick button: switch between ODO and TRIP |
| The odometer and trip are restored from EEPROM | Long press (1.5 s) on the TRIP page: reset the trip |

LCD layout (16 x 2):

```text
SPD   87.3 km/h!      line 1: speed, "!" at 65 km/h or more
ODO     12.3 km       line 2: odometer, or "TRIP     3.4 km" after a short press
```

Speed warning (`SWC_Gauge`): one beep per heartbeat, and the heart beats faster as the speed rises, like a racing heart.

| Speed | Beep every | Heart rate |
|---|---|---|
| 65 km/h or more | 2.3 s | ~26 bpm |
| 90 km/h or more | 1.5 s | 40 bpm |
| 120 km/h or more | 1.0 s | 60 bpm |
| Above 150 km/h | 0.6 s | 100 bpm |

A level rises as soon as the speed reaches its threshold and drops only 2 km/h below it, so the beat does not flicker at the boundary. Each beep lasts 100 ms. Set `GAUGE_WARN_BEEPS` to 2 (and widen the windows) for a "lub-dub" double beat.

The odometer is saved to EEPROM every km and 1 s after the car stops. It survives a reset or power-off.

## Repository layout

```text
src/                 portable: SWCs, RTE (generated), services (Os, EcuM, NvM, Crc, Det),
                     IoHwAb, CDDs (stepper, LCD, log), common types
platform/avr/        Arduino Uno: MCAL on the ATmega328P registers, SchM, Os port, UART hook
platform/s32k144/    MaaZEDU board: MCAL on the S32K144 registers, startup, linker script, Makefile
arduino/cluster_ecu/ Arduino IDE sketch = src/ + platform/avr/ flattened (tools/export_arduino.py)
model/               ARXML model of the SWCs and connectors
docs/                wiring diagrams
```

Each platform provides the same MCAL headers (`Mcu.h`, `Port.h`, `Dio.h`, `Adc.h`, `Gpt.h`, `Eep.h`, `I2c.h`, `Pwm.h`) plus `SchM.c`, `Os_Port.h` (interrupt lock and idle sleep) and `Cdd_LogHw.c` (UART). The portable code only calls these APIs.

## Architecture

```text
 ┌──────────────────────────── Application (SWC) ────────────────────────────┐
 │ SWC_DriverInput ─Pedal─► SWC_VehicleModel ─Speed─► SWC_Gauge              │
 │        │                          └────────Speed─► SWC_Odometer ◄─TripReset┐│
 │        └──────────ButtonEvent──────────(+Speed)───► SWC_Display ◄─Distance┘│
 ├──────────────────── RTE (generated from model/Cluster.arxml) ─────────────┤
 ├─────────── Services ──────────┬──── ECU abstraction ────┬────── CDD ──────┤
 │ Os  EcuM  NvM  Crc  Det  SchM │ IoHwAb                  │ Cdd_Stepper     │
 │                               │                         │ Cdd_Lcd         │
 │                               │                         │ Cdd_Log (UART)  │
 ├──────────────── MCAL (platform/avr  or  platform/s32k144) ────────────────┤
 │ Mcu   Port   Dio   Adc   Gpt   Eep   I2c   Pwm                            │
 └──────────────── ATmega328P  or  S32K144 registers ────────────────────────┘
```

| Layer | Modules | AUTOSAR concepts shown |
|---|---|---|
| MCAL | `Mcu`, `Port`, `Dio`, `Adc`, `Gpt`, `Eep`, `I2c`, `Pwm` | Configuration tables (`*_Cfg.c`), `Dio_WriteChannelGroup`, interrupt-driven ADC groups, non-blocking EEPROM writes that skip unchanged bytes, interrupt-driven I2C jobs (plus a polled mode for start-up), DET checks |
| Services | `Os`, `EcuM`, `NvM`, `Crc`, `Det`, `SchM` | Time-triggered OS with a task table and overrun counter, startup sequence, redundant NvM block with CRC16 and ROM defaults, exclusive areas |
| ECU abstraction | `IoHwAb` | Raw ADC → throttle / brake %, button debounce; SWCs never touch registers |
| CDD | `Cdd_Stepper`, `Cdd_Lcd`, `Cdd_Log` | Needle position control, LCD frame buffer that sends only changed characters (one per call, never blocks the OS), UART log with a ring buffer |
| RTE | `src/Rte` (generated) | Sender-Receiver ports, `TimingEvent`s, `RTE_E_NEVER_RECEIVED` handling |
| SWC | 5 SWCs in `src/Swc` | Sequence counters for events, saturation, hysteresis, safe defaults |

OS tasks (`src/Service/Os_Cfg.c`):

| Task | Period | Runs |
|---|---|---|
| `OsTask_10ms` | 10 ms | `IoHwAb_MainFunction`, `Eep_MainFunction`, `NvM_MainFunction` |
| `OsTask_1ms` | 1 ms | `Rte_MainFunction` (SWC runnables at 10 / 20 / 100 ms) |
| `OsTask_2ms` | 2 ms | `Cdd_Stepper_MainFunction` (one needle step), `Cdd_Lcd_MainFunction` (at most one changed character) |
| `OsTask_1000ms` | 1 s | `EcuM_MainFunction` (status on the UART log) |

## Wiring (Arduino Uno)

![Wiring diagram](docs/wiring.svg)

| Kit part | Part pin | Arduino Uno | MCU pin |
|---|---|---|---|
| Joystick | VRy | A1 | PC1 (ADC1) |
| Joystick | VRx (optional) | A0 | PC0 (ADC0): both axes are read, the one pushed further is the pedal |
| Joystick | SW | A2 | PC2 |
| Joystick | +5V / GND | 5V / GND | |
| ULN2003 board (28BYJ-48) | IN1 / IN2 / IN3 / IN4 | D8 / D9 / D10 / D11 | PB0..PB3 |
| ULN2003 board | + / − | 5V of the breadboard power module / GND | |
| Buzzer (passive or active) | + (signal) | D3 | PD3 / OC2B: 4 kHz square wave from Timer2 (`Pwm`) |
| Overspeed telltale | on-board LED "L" | D13 | PB5 |
| LCD1602 I2C backpack | SDA / SCL | A4 / A5 | PC4 / PC5 (TWI) |
| LCD1602 I2C backpack | VCC / GND | 5V / GND | |

These are the same pins as lessons 11, 18 and 19 of the kit.

**LCD troubleshooting:**

- **Backlight on but no characters:** turn the blue contrast potentiometer on the backpack.
- **`[Lcd] no ACK at I2C address 0x27` in the log:** check SDA/SCL. A backpack with a PCF8574A uses `0x3F`; change `CDD_LCD_I2C_ADDRESS` in `src/Cdd/Cdd_Lcd.h`.

> Power the stepper from the kit's breadboard power module, and connect all the GNDs together. If the motor and the Uno share USB power, the Uno can reset and the fault looks like a software bug.

**Needle zero:** the 28BYJ-48 has no end stop. Turn the needle to 0 km/h by hand before you power on. The firmware assumes it starts at 0.

## Build and flash (Arduino Uno)

### Arduino IDE

`arduino/cluster_ecu/` is a ready-to-open Arduino IDE sketch: all sources flattened into one folder, generated by `python tools/export_arduino.py`. Open `cluster_ecu.ino`, select **Arduino Uno** and the port, then press Upload. The `.ino` file is intentionally empty: `main()` is in `Main.c`, so the Arduino core's `setup()` / `loop()` are never linked.

`arduino/buzzer_test/` beeps on D3, D8 and D9 in turn, which is a quick way to check the buzzer wiring.

### PlatformIO

Requirement: [PlatformIO](https://platformio.org/), either the VS Code extension or `pip install platformio`.

```bash
pio run                      # build
pio run -t upload            # flash the Uno over USB (Arduino bootloader)
pio device monitor           # UART log, 115200 baud
```

Current size: 683 B RAM (33 %), 9.6 KB flash (30 %).

The ARXML model is written by `tools/make_arxml.py` (SWCs, ports and connectors are listed at the top of the script). After a change, rebuild the model and regenerate the RTE. The second script needs the arxml2rte generator in a folder next to this one (or its path in `-Generator`):

```bash
python tools/make_arxml.py model/Cluster.arxml
```

```powershell
powershell -ExecutionPolicy Bypass -File tools/gen_rte.ps1
```

## MaaZEDU S32K144 version

Same behaviour, same LCD pages and speed warning, on the MaaZEDU board. The log goes to the board's own USB COM port (LPUART1, 115200 baud) and the on-board red LED is the overspeed telltale.

> **Not yet tested on a board.** The firmware builds without warnings and the vector table and flash configuration field were checked in the binary, but it has not been run on a MaaZEDU board yet.

![MaaZEDU wiring diagram](docs/wiring_maazedu.svg)

| Kit part | Part pin | MaaZEDU J8 | MCU pin |
|---|---|---|---|
| Joystick | +5V pin | **J8-22 (3.3V)** | VDD |
| Joystick | GND | J8-24 | GND |
| Joystick | VRy | J8-18 | PTC14 = ADC0_SE12 |
| Joystick | VRx | J8-20 | PTC15 = ADC0_SE13 |
| Joystick | SW | J8-5 (GPIO0) | PTE0, internal pull-up |
| LCD1602 I2C backpack | SDA / SCL | J8-12 / J8-10 | PTA2 / PTA3 = LPI2C0 (ALT3) |
| LCD1602 I2C backpack | VCC / GND | J8-22 (3.3V) / J8-24 | |
| ULN2003 board | IN1 / IN2 / IN3 / IN4 | J8-19 / J8-17 / J8-15 / J8-13 | PTD9 / PTD10 / PTD11 / PTD12 |
| ULN2003 board | + / − | 5V of the breadboard power module (or J8-1) / GND | |
| Buzzer | + (or S) / − | J8-11 (GPIO3) / J8-24 | PTD13 |
| Overspeed telltale | on-board RGB LED, red | | PTD15, active low |
| Log | USB of the board | | LPUART1 PTC6 / PTC7 |

**3.3 V logic.** The J8 signal pins accept at most 3.3 V. That is why the joystick and the LCD backpack are powered from J8-22 (3.3V), not 5V:

- **Joystick:** at 3.3 V its output spans the full ADC range (12 bit, VREFH = 3.3 V).
- **LCD backpack:** it has pull-up resistors to its own VCC on SDA/SCL, so at 5 V it would put 5 V on PTA2/PTA3. At 3.3 V the PCF8574 works, but a 5 V LCD can look faint: turn the contrast pot first. If the text stays invisible, power the LCD at 5 V **through an I2C level shifter** (BSS138 module, 3.3 V side to J8).
- **ULN2003:** its inputs switch at 3.3 V; only the motor needs 5 V. Connect the module GND to J8-24.

### Build and flash (S32K144)

Requirements: the ARM GCC and `make` shipped with S32 Design Studio 3.6 (`C:\NXP\S32DS.3.6.11`), and the SEGGER J-Link software for flashing. No S32DS project and no NXP SDK are needed: the register definitions, startup code and linker script are in `platform/s32k144/`.

```powershell
powershell -ExecutionPolicy Bypass -File platform/s32k144/build.ps1      # -> platform/s32k144/build/cluster_ecu.hex
powershell -ExecutionPolicy Bypass -File platform/s32k144/flash.ps1      # J-Link SWD, then reset + run
```

Open the board's COM port at 115200 baud (Tera Term, PuTTY) to see the log. Current size: 11.1 KB flash, 540 B RAM.

### What changes on the S32K144

| Module | Arduino Uno (AVR) | MaaZEDU (S32K144) |
|---|---|---|
| Mcu | 16 MHz crystal, MCUSR reset flags | FIRC 48 MHz, FIRCDIV2 for the peripherals, RCM reset flags, DWT cycle counter for delays |
| Gpt (OS tick) | Timer0 CTC, 4 µs ticks | LPIT0 channel 0, 48 MHz |
| Adc | 10 bit, AVcc reference | ADC0 12 bit, calibrated at start-up, 4-sample hardware average |
| Pwm (buzzer) | Timer2 OC2B hardware PWM, 4 kHz | LPIT0 channel 1 toggles the pin at 8 kHz, a 4 kHz square wave on any GPIO |
| I2c | TWI, one byte per interrupt | LPI2C0, command words fed into its 4-entry TX FIFO from the interrupt |
| Eep | 1 KB EEPROM, byte writes | D-flash (FlexNVM) via FTFC: erase a 2 KB sector, program 8-byte phrases; each NvM copy owns a sector |
| SchM / Os_Port | SREG, `sleep` | PRIMASK, `wfi` |
| Startup | avr-libc | own vector table, flash configuration field (FSEC = 0xFE, unsecure), ECC RAM init, watchdog off |

The S32K144 `Eep` only uses the D-flash. If an earlier project partitioned the FlexNVM as emulated EEPROM, `Eep` disables itself (log line `[Eep] FlexNVM is partitioned...`) and the odometer is not saved.

## Calibration

| What | Where |
|---|---|
| The stick accelerates when pushed down | `IOHWAB_JOY_Y_INVERT` in `src/Ecual/IoHwAb.h` |
| The needle turns the wrong way | `CDD_STEPPER_REVERSE` in `src/Cdd/Cdd_Stepper.h` |
| Dial angle / full scale | `CDD_STEPPER_MAX_STEPS` (1365 steps = 240°), `GAUGE_FULL_SCALE_KMH10` |
| Acceleration, braking, drag, top speed | `VEH_*` in `src/Swc/Swc_Cfg.h` |
| Speed warning thresholds and beep patterns | `GAUGE_WARN_*` in `src/Swc/Swc_Cfg.h` |

## UART log

At 115200 baud the log shows these lines:

- `[EcuM] reset: ...`: the reset reason.
- `[NvM] block 2: ...`: which EEPROM copy was used, or ROM defaults.
- `[Lcd] ...`: whether the LCD answered on the I2C bus.
- `[Cluster] speed ... km/h  odo ... km  trip ... km`: printed every second.
- `[EcuM] t=... s  overruns=...  det=...`: printed every second.
- `[DET] module=... api=... err=...`: printed for every development error. Module IDs: Port 124, Dio 120, Adc 123, Gpt 100, Eep 90, NvM 20, I2c 254, Cdd_Lcd 253.

## Known limitations

- **The S32K144 build has not been run on hardware yet** (see the MaaZEDU section).
- **First boot shows `NvM: data lost`.** A fresh EEPROM is blank, so both NvM copies fail the CRC check. The odometer starts at 0 and the block is rewritten at once, so the next boot is clean.
- **The vehicle speed uses three P-ports** (`Pp_SpeedGauge`, `Pp_SpeedOdo`, `Pp_SpeedDisplay`). arxml2rte names a VFB buffer after the provider port only. A single P-port connected to two R-ports would therefore generate two buffers with the same name.
- **One runnable per SWC.** This is a rule of the arxml2rte validator. Button and trip-reset events are therefore sent as a value plus a sequence counter, and receivers poll for a new sequence number instead of using `DataReceivedEvent`s.
- **The SWCs call some BSW modules directly** (`IoHwAb`, `Cdd_Stepper`, `Cdd_Lcd`, `NvM`). In full AUTOSAR these calls go through RTE Client-Server ports, which arxml2rte does not support yet.
- **The EEPROM wears.** Saving every km rewrites the low byte about 100 000 times per 100 000 km. A production ECU would save at ignition off and spread the writes (Fee / Ea wear levelling).
