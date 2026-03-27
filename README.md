# LifeCore
### HackTues 12 — Team CoreSix

> **"LimitLess"** — A wearable blind navigation assistant that uses ultrasonic sensing and real-time feedback to help visually impaired people navigate their surroundings independently. Paired with a web dashboard for text-to-speech audio feedback and a community hub where users can share tips and experiences.

---

## The Idea

LifeCore is a two-part wearable system designed for visually impaired users. A small sensor unit worn on glasses, a headband or another wearable continuously detects obstacles and communicates distance to the user via audio feedback, with LED indicators providing an additional visual reference, on a communication unit that is on a belt/pocket. 

There are 4 distance zones (Safe, Caution, Close, Critical) that trigger different LED colors and buzzer patterns. The distance thresholds can be adjusted in real-time using a potentiometer for sensitivity control. The sensor unit and communication unit communicate wirelessly via Bluetooth Low Energy (BLE).

The communication unit also connects to a server-hosted website on Raspberry Pi that has user authentication and text-to-speech conversion, providing users with detailed spoken feedback about their surroundings.

The web app dashboard also serves as a community hub for users (regardless of their disabilities) to share tips, stories and other information about their disability.

Built for **Hack TUES 12** under the theme **"Code to Care"**, subtopic **"LimitLess"**.

---

## System Architecture

```
[SENSOR UNIT]                          [COMMUNICATION UNIT]
┌──────────────────┐                  ┌──────────────────────┐
│ ESP32 #1         │                  │ ESP32-S3 #2          │
│ + HC-SR04        │ ──── BLE ──────→ │ + LEDs (R/Y/G)       │
│   Ultrasonic     │                  │ + Piezo Buzzer       │
│     sensor       │                  │ + Potentiometer      │
└──────────────────┘                  |      (sensitivity)   |
                                      └──────────────────────┘
                                                │
                                                | WiFi
                                                ▼
                                      [.NET Backend Server]
                                      ┌──────────────────────┐
                                      │ Raspberry Pi         │
                                      │ Login / Register     │
                                      │ Dashboard            │
                                      │ Text-to-speech       │
                                      └──────────────────────┘
```

### Distance Zones

| Zone | Distance | LED | Buzzer |
|------|----------|-----|--------|
|  Safe | > 100cm | Green | Silent |
|  Caution | 50–100cm | Yellow | Slow beep (500Hz) |
|  Close | 20–50cm | Red | Fast beep (800Hz) |
|  Critical | < 20cm | All flash | Continuous alarm (1200Hz) |

*All thresholds scale dynamically with the potentiometer sensitivity setting. Sensitivity can be between 0.5 and 2.0.*

---

## Repository Structure

```
CoreSix-HackTues12/
│
├── LifeCore_v1/        # Hardware v1 — hardcoded distance thresholds (Arduino UNO)
├── LifeCore_v2/        # Hardware v2 — potentiometer sensitivity control added
├── LifeCore_v3/        # Hardware v3 — migrated from Arduino UNO to ESP32
├── LifeCore_v4/        # Hardware v4 — wireless, two ESP32s communicating via BLE
│
├── frontend/           # Web dashboard
├── server/             # .NET backend — auth, API (C#)
├── k8s/                # Kubernetes deployment manifests
│
├── docker-compose.yaml # Local development setup
├── .gitignore
└── LICENSE             # MIT
```

### Hardware Versions

| Version | Board | Key Feature |
|---------|-------|-------------|
| `v1` | Arduino UNO | Basic obstacle detection, hardcoded thresholds |
| `v2` | Arduino UNO | Potentiometer for adjustable sensitivity |
| `v3` | ESP32 | Same functionality as v2, migrated to ESP32 |
| `v4` | 2x ESP32 | Wireless — sensor unit and feedback unit communicate via Bluetooth Low Energy (BLE) |

---

## Hardware Components (v4)

**Sensor unit (ESP32):**
- ESP32 development board
- HC-SR04 ultrasonic distance sensor

**Communication unit (OLIMEX ESP32-S3):**
- ESP32 development board
- 3x LEDs (red, yellow, green) + 330Ω resistors
- Piezo buzzer
- 10kΩ potentiometer (sensitivity control)

## Software - The Website
- Dashboard that displays all posts, comments, etc.
- Login / Register system
- Both of those write and read data from a non-relational database (MongoDB)

## Running the Software

### Prerequisites
- Node.js (for frontend development)
- .NET SDK (for backend development)

### Running individually

**Backend:**
```bash
cd server
dotnet run
```

**Frontend:**
```bash
cd frontend
npm install
npm start
```

---

### Link to website
- https://lifecoresu.netlify.app

## Team CoreSix

Built at **Hack TUES 12** by Team CoreSix.
- **Ivaneta Ivanova** -> Hardware, Simulation / Wiring Diagrams, README
- **Kristiyan Kobarelov** -> Hardware Assistance, Testing, Presentation
- **Martin Valentinov** -> Hardware, Server Hosting, Backend, Logic
- **Viktor Sirakov** -> Server Hosting Assistance, VCT / DevOps
- **Yordan Tsonev** -> Frontend, Script, Chat System

---

## License

MIT — see [LICENSE](LICENSE) for details.
