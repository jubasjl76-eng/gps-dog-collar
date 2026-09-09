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

1. `pio run -d firmware` and flash to the ESP32
2. Join the `smartpet-<mac>` AP on first boot and provision (kennelId, deviceId,
   Wi-Fi, MQTT host, claim password)
3. Claim the collar from the Smart Pet console and set its safe zones

## Firmware

`firmware/main.cpp` is built on
[smart-pet-device-sdk](https://github.com/jubasjl76-eng/smart-pet-device-sdk)
and publishes `kennel/{kennelId}/gps/{deviceId}/location`. See
`firmware/README.md`. The pre-SDK single-file firmware is kept as
`firmware/collar-firmware.legacy.cpp`.

The collar talks MQTT to
[smart-pet-backend](https://github.com/jubasjl76-eng/smart-pet-backend);
safe-zone / geofence logic lives there (Phase 9.2). The `backend/` folder in
this repo (MongoDB models, an old MQTT consumer) is dead and will be removed.

## Files

- `firmware/main.cpp` - device firmware (SDK)
- `firmware/collar-firmware.legacy.cpp` - pre-SDK firmware, kept for reference
- `3d-design/collar-case.scad` - 3D printable case
