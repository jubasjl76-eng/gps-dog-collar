# Collar firmware

`main.cpp` on [smart-pet-device-sdk](https://github.com/jubasjl76-eng/smart-pet-device-sdk).
The SDK handles Wi-Fi/SoftAP provisioning, NTP, MQTT, LWT, OTA, command/ack and
the offline journal. This sketch reads the NEO-6M GPS, publishes `location`,
adapts the interval to movement, and watches the battery.

## Topic migration

The pre-SDK firmware published `dogs/collar-001/location`. This publishes
`kennel/{kennelId}/gps/{deviceId}/location` — the `gps` device type and its
routing already exist in the SDK (`spd_topics.h`). No `dogs/...` topics remain.
Commands arrive on `kennel/{k}/gps/{id}/command`; `get_location` publishes a fix
immediately, and the built-in `set_interval` / `restart` / `ota` / `identify`
still work.

## Build

```bash
pio run -d firmware
```

`platformio.ini` pins the SDK, forces `-std=gnu++17`, and uses `min_spiffs.csv`
for A/B OTA. **Re-pin the SDK to a `main` SHA once
`smart-pet-device-sdk#2` (publishLocation) merges** — it is currently pinned to
the `feat/publish-location` branch.

## Wiring (diagram.json)

| GPIO | part |
|---|---|
| 16 | GPS TX → ESP RX (UART1) |
| 17 | GPS RX ← ESP TX |
| 34 | battery voltage divider (analog) |
| 2  | status LED |
| 0  | button — publish a fix now |

## First boot / provisioning

No credentials are compiled in. On first boot the device opens a Wi-Fi AP
`smartpet-<mac>`; join it and fill in kennelId, deviceId, Wi-Fi, MQTT host and
the claim password. Values persist in NVS.

## Calibrate on a real board

| what | how |
|---|---|
| `batteryPct()` map | measure the divider output at a full and an empty pack, map the raw ADC to those. |
| `MOVE_THRESHOLD_M` | walk the collar; tune so a stationary dog does not read as "moving" (GPS jitter is a few metres). |

Also verify on hardware: a real satellite fix outdoors (cold start can take
minutes), the geofence firing when the dog leaves a safe zone (server side,
Phase 9.2), deep-sleep wake + republish, and an OTA pull.

## CI

- `pio run -d firmware` builds for `esp32dev` on every push (via
  `smart-pet-ci/pio-ci`).
- `firmware/collar.test.yaml` is a Wokwi smoke test (boot → provisioning portal
  opens). It runs when a `WOKWI_CLI_TOKEN` repo secret is set.
