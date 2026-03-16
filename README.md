# GPS Dog Collar System

A complete GPS tracking system for dogs, integrated with the existing Smart Pet platform.

## Features

- Real-time GPS tracking
- MQTT communication
- Geofencing (safe zones)
- Battery monitoring
- Mobile app integration
- Admin dashboard
- 3D printable case

## Architecture

```
Collar → MQTT → Backend → WebSocket → Mobile/Dashboard
```

## Components

### Hardware
- ESP32 microcontroller
- u-blox NEO-6M GPS
- LiPo battery

### Firmware
- Location tracking
- MQTT publish
- Power management
- OTA updates

### Backend
- MQTT consumer service
- Location storage
- Safe zone checking
- Alert generation

### Mobile
- Real-time map
- Dog profiles
- Geofence management

## Cost Estimate

| Component | Cost |
|-----------|------|
| ESP32 | $6 |
| GPS Module | $15 |
| Battery | $10 |
| SIM (optional) | $8 |
| PCB | $5 |
| 3D Case | $3 |
| **Total** | **$47** |

## Getting Started

1. Flash firmware to ESP32
2. Set up MQTT broker
3. Configure backend
4. Pair collar with app

## Files

- `firmware/collar-firmware.cpp` - Device firmware
- `backend/models/` - MongoDB schemas
- `backend/services/mqttConsumer.ts` - MQTT handler
- `3d-design/collar-case.scad` - 3D printable case

## Wokwi Simulation

This firmware can be simulated in Wokwi without physical hardware.

### Simulated Hardware Components

- **ESP32 DevKit V1** - Main microcontroller
- **GPS Module (NMEA)** - Simulated GPS for location tracking
- **LED** - Status indicator (red)
- **Push Button** - Manual trigger
- **Potentiometer** - Battery level simulation

### Running the Simulation

1. Open [Wokwi](https://wokwi.com)
2. Create a new ESP32 project
3. Upload the `firmware/diagram.json` file
4. Build the firmware:
   ```bash
   # Using PlatformIO (if available)
   pio run
   ```
5. Upload the compiled firmware to Wokwi
6. The simulation will start automatically

### Pin Connections

| ESP32 Pin | Component |
|-----------|-----------|
| TX0 (16) | GPS RX |
| RX0 (17) | GPS TX |
| 2 | Status LED |
| 0 | Push Button |
| 34 | Battery (analog) |
| 3V3 | VCC |
| GND | Ground |

### Testing

The simulation will show:
- GPS coordinates being "received"
- LED blinking on status changes
- Button press events
- Battery level readings
