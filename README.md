# IoT Monitoring & Control System

A comprehensive IoT ecosystem designed for environmental monitoring, real-time tracking, and remote device management. This project integrates high-performance ESP32 firmware with a modern web dashboard for seamless interaction.

---

## Key Features

* **Real-time Monitoring:** Track sensor data and device status with low latency.
* **Remote Control:** Toggle actuators and manage system parameters directly from the web.
* **Advanced Firmware:** Built on FreeRTOS for robust multitasking and stability.
* **Smart Configuration:** Supports local WiFi setup via Access Point (AP) mode.
* **Seamless OTA:** Automated "Over-The-Air" updates to keep hardware up to date.
* **Secure Access:** User authentication via system accounts or Google Login.

---

## Tech Stack

### Firmware (ESP32)
* **Framework:** PlatformIO with FreeRTOS.
* **Connectivity:** WiFi (Station & AP Mode), HTTP/HTTPS.
* **Updates:** Custom OTA update client.

### Software (Web Dashboard)
* **Frontend:** Next.js (App Router), Tailwind CSS.
* **Deployment:** Vercel.

### Database & Backend
* **Firebase Auth:** Secure user management and OAuth.
* **Firebase Realtime Database:** Instant data synchronization for IoT nodes.
* **Cloud Storage:** Hosting firmware binaries for remote updates.

---

## Project Structure

```text
.
├── MonitorSystem_Firmware/     # ESP32 Source Code
│   ├── src/                    # Main logic (.cpp)
│   ├── include/                # Header files (.h)
│   ├── lib/                    # Custom local libraries
│   ├── data/                   # SPIFFS/LittleFS assets
│   └── platformio.ini          # Environment configuration
├── MonitorSystem_WebApp/       # Next.js Web Application
│   ├── app/                    # Routing and UI components
│   ├── public/                 # Static assets
│   └── tailwind.config.js      # Styling configuration
├── release/                    # Production builds
│   └── firmware.bin            # Compiled binary for OTA
├── latest.json                 # OTA Metadata for version tracking
└── README.md                   # Documentation
```

## OTA Update Mechanism

The ESP32 checks the `latest.json` file on the server to compare the current version with the latest available release.

**Example `latest.json`:**

```json
{
  "version": "1.0.2",
  "url": "https://your-domain.com/release/firmware.bin",
  "min_battery": 20,
  "changelog": "Fixed AP mode stability and improved sensor sampling rate."
}
```

## Getting Started
1. Hardware Setup:
Open MonitorSystem_Firmware in VS Code with PlatformIO.
Configure your Firebase credentials in the header files.
Build and upload the initial image via USB.

2. Web Dashboard:
Navigate to MonitorSystem_WebApp.
Install dependencies: npm install.
Set up .env.local with your Firebase project keys.
Run the development server: npm run dev.

## How to Run

### 1. Firmware (ESP32)
The firmware is managed using PlatformIO Core. You will need Python to install the PlatformIO CLI (`pio`).

**Prerequisites:** Python 3.x installed on your system.

**Steps:**
1. Install PlatformIO globally using Python's package manager (`pip`):

```bash
pip install -U platformio
 %Navigate into the firmware directory:
cd MonitorSystem_Firmware
```
Build the project and upload it to your ESP32 board (ensure your board is connected via USB):

```Bash
pio run -t upload
```
(Optional: You can also use the pio device monitor command to open the serial monitor and view logs).

### 2. Software (Web Dashboard)
The web dashboard is built with Next.js and requires Node.js.
Prerequisites: Node.js (which includes npm) installed on your system.
Steps:
Navigate into the web application directory:

```Bash
cd MonitorSystem_WebApp
Install all the required dependencies:
```

```Bash
npm install
Run the development server:
```

```Bash
npm run dev
```

Open your browser and go to http://localhost:3000 to view the dashboard.
