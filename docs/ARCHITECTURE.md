# GPS Dog Collar - Technology Investigation & Architecture

## 1. Communication Protocols Comparison

### Device → Server Communication

| Protocol | Battery Consumption | Reliability | Real-time | Scalability | Cost | Recommendation |
|----------|-------------------|------------|-----------|-------------|------|----------------|
| **MQTT** | ⭐⭐ Very Low | High | ✅ Excellent | High | Low | ✅ **Recommended** |
| HTTPS Polling | ⭐⭐⭐ Medium | High | ❌ Poor | Medium | Medium | ❌ Not suitable |
| LoRaWAN | ⭐⭐ Very Low | Medium | ❌ Poor | High | Low | ❌ Too slow |
| NB-IoT | ⭐⭐ Low | High | ✅ Good | High | Medium | ⚠️ Carrier needed |
| LTE-M | ⭐⭐ Low | High | ✅ Good | High | High | ⚠️ Expensive |
| Bluetooth | ⭐⭐⭐⭐ Minimal | Low | ❌ Short range | N/A | Low | ⚠️ Phone needed |

### Server → Mobile Real-time

| Protocol | Latency | Scalability | Complexity | Recommendation |
|----------|---------|-------------|------------|----------------|
| **WebSockets** | ⭐⭐⭐ Ultra Low | High | Medium | ✅ **Recommended** |
| Server-Sent Events | ⭐⭐⭐ Low | Medium | Low | ⚠️ Alternative |
| HTTP Polling | ⭐ Low | High | Low | ❌ Not suitable |

---

## 2. Recommended Architecture

```
                    ┌─────────────────┐
                    │   MQTT Broker   │
                    │   (Mosquitto)  │
                    └────────┬────────┘
                             │
              ┌──────────────┼──────────────┐
              │              │              │
              ▼              ▼              ▼
        ┌──────────┐  ┌──────────┐  ┌──────────┐
        │  Collar  │  │  Collar  │  │  Collar  │
        │ Device   │  │ Device   │  │ Device   │
        └──────────┘  └──────────┘  └──────────┘

MQTT Topic: dogs/{deviceId}/location

                    ┌─────────────────┐
                    │  IoT Consumer  │
                    │    Service      │
                    └────────┬────────┘
                             │
              ┌──────────────┼──────────────┐
              ▼              ▼              ▼
        ┌──────────┐  ┌──────────┐  ┌──────────┐
        │   Mongo  │  │   Redis  │  │   Web    │
        │   DB    │  │  Pub/Sub │  │  Socket  │
        └──────────┘  └──────────┘  └──────────┘
                             │
              ┌──────────────┼──────────────┐
              ▼              ▼              ▼
        ┌──────────┐  ┌──────────┐  ┌──────────┐
        │  Mobile  │  │ Dashboard │  │   Push   │
        │    App   │  │          │  │Notifs    │
        └──────────┘  └──────────┘  └──────────┘
```

---

## 3. Hardware Components

### Microcontroller

| Chip | WiFi | BLE | Cellular | Power | Cost | Notes |
|------|------|-----|----------|-------|------|-------|
| **ESP32** | ✅ | ✅ | ❌ | ⭐⭐ | $6 | ✅ Recommended - cheap, well supported |
| ESP32-S3 | ✅ | ✅ | ❌ | ⭐⭐ | $8 | More powerful, good for ML |
| Nordic nRF9160 | ❌ | ✅ | ✅ LTE-M | ⭐⭐⭐ | $20 | Cellular built-in, expensive |
| Raspberry Pi Pico W | ✅ | ❌ | ❌ | ⭐⭐ | $6 | No BLE, less IO |

**Recommendation:** **ESP32** - Best balance of cost, features, and community support

### GPS Module

| Module | Accuracy | Cold Start | Power | Cost | Notes |
|--------|----------|------------|-------|------|-------|
| **u-blox NEO-6M** | 2.5m | 27s | ⭐⭐ | $15 | ✅ Recommended - reliable |
| u-blox NEO-7M | 2.5m | 20s | ⭐⭐ | $18 | Faster, slightly more expensive |
| u-blox M8N | 2.5m | 26s | ⭐⭐⭐ | $25 | Multi-constellation |
| Quectek L76B | 2.5m | 30s | ⭐⭐ | $12 | Cheap alternative |
| ATGM336H | 2.5m | 32s | ⭐⭐ | $10 | Budget option |

**Recommendation:** **u-blox NEO-6M** - Best balance of cost and reliability

### Connectivity Options

| Option | Coverage | Data Cost | Power | Speed | Notes |
|--------|----------|----------|-------|-------|-------|
| **WiFi** | Local | Free | ⭐⭐ | Fast | ✅ Home use only |
| BLE | 10m | Free | ⭐⭐⭐⭐ | Fast | ⚠️ Phone nearby |
| LTE-M | Global | ~$2/mo | ⭐⭐ | Good | ✅ **Recommended for outdoor** |
| NB-IoT | Regional | ~$1/mo | ⭐⭐⭐ | Slow | ⚠️ Not real-time |

**Recommendation:** **WiFi + BLE fallback** for MVP, add **LTE-M** for production

---

## 4. Power Optimization Strategy

### Adaptive Tracking Algorithm

```cpp
// GPS polling intervals based on activity
const int TRACKING_INTERVAL_MOVING = 10;   // 10 seconds when moving
const int TRACKING_INTERVAL_IDLE = 120;    // 2 minutes when idle
const int TRACKING_INTERVAL_SLEEP = 300;   // 5 minutes in sleep mode
const int BATTERY_CRITICAL = 20;            // Critical battery threshold
```

### Battery Estimates

| Mode | Current | 2000mAh Battery Life |
|------|---------|---------------------|
| Active tracking | 100mA | ~20 hours |
| Normal tracking | 50mA | ~40 hours |
| Power save | 10mA | ~8 days |
| Deep sleep | 1mA | ~80 days |

---

## 5. MQTT Topic Structure

```
dogs/
├── {deviceId}/
│   ├── location     # Publish GPS data
│   ├── status       # Device status/battery
│   ├── command      # Receive commands
│   └── config       # Device configuration
```

### Payload Example

```json
{
  "deviceId": "collar-001",
  "latitude": 40.4167,
  "longitude": -3.7033,
  "accuracy": 5.2,
  "altitude": 650,
  "speed": 2.5,
  "heading": 180,
  "battery": 78,
  "timestamp": 1710000000,
  "gsmSignal": -65
}
```

---

## 6. Cost Estimate (Per Device)

| Component | Cost |
|-----------|------|
| ESP32 Dev Board | $6 |
| u-blox NEO-6M GPS | $15 |
| 3.7V 2000mAh LiPo | $10 |
| SIM800L (optional) | $8 |
| PCB + Components | $5 |
| 3D Printed Case | $3 |
| **Total** | **$47** |

---

## 7. Next Steps

1. ✅ Architecture decided: MQTT + WebSockets
2. ✅ Hardware: ESP32 + NEO-6M
3. ⏳ Create firmware project
4. ⏳ Add backend modules
5. ⏳ Extend mobile app
6. ⏳ Extend dashboard
7. ⏳ Design 3D case
