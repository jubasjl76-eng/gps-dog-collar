# GPS Dog Collar - Technology Investigation Report

## 1. Communication Protocols Comparison

### Device → Server Communication

| Protocol | Battery Consumption | Reliability | Real-time | Scalability | Cost |
|----------|-------------------|-------------|-----------|--------------|------|
| **MQTT** | ⭐⭐⭐ Low | High | Excellent | Excellent | Low |
| HTTPS Polling | ⭐⭐ Medium | High | Poor | Medium | Medium |
| WebSockets | ⭐⭐ Medium | High | Excellent | Medium | Medium |
| LoRaWAN | ⭐⭐⭐ Very Low | Medium | Poor | Good | Very Low |
| NB-IoT | ⭐⭐⭐ Low | High | Good | Excellent | Very Low |
| LTE-M | ⭐⭐ Low | High | Excellent | Excellent | Low |

### Server → Mobile Real-time

| Protocol | Latency | Scalability | Complexity |
|----------|---------|-------------|------------|
| **WebSockets** | <50ms | Medium | Low |
| **MQTT** | <50ms | Excellent | Medium |
| Server-Sent Events | <100ms | Medium | Low |
| Polling | 1-30s | High | Very Low |

---

## Recommendation

**Device → Server:** MQTT (QoS 1)
- Ultra-low battery consumption
- Reliable with QoS levels
- Perfect for intermittent data
- Low bandwidth cost

**Server → Mobile:** WebSockets + MQTT
- Real-time location updates
- Push notifications via MQTT
- Fallback to polling if needed

---

## 2. Hardware Components

### Microcontroller Comparison

| MCU | GPS | WiFi | BT | Power | Cost |
|-----|-----|------|-----|-------|------|
| **ESP32** | External | ✅ | ✅ | 80-250mA | €8-12 |
| **Nordic nRF9160** | External | ❌ | ❌ | 5-50mA | €15-20 |
| **Raspberry Pi Pico W** | External | ✅ | ✅ | 40-100mA | €6-10 |

**Selected:** **ESP32** - Best balance of features, cost, and community support

### GPS Module Comparison

| Module | Accuracy | Cold Start | Power | Cost |
|--------|----------|------------|-------|------|
| **u-blox NEO-6M** | 2.5m | 27s | 45mA | €10-15 |
| **u-blox M8N** | 2.5m | 26s | 67mA | €15-20 |
| **Quectel L76B** | 2.5m | 30s | 25mA | €8-12 |
| **ATGM336H** | 2.5m | 28s | 30mA | €6-10 |

**Selected:** **Quectel L76B** or **ATGM336H** - Low power, good accuracy

### Connectivity

| Option | Coverage | Power | Data Cost | Best For |
|--------|----------|-------|-----------|----------|
| **WiFi** | Limited | Medium | Free (when home) | Indoor/Backyard |
| **LTE-M** | Excellent | Low | ~€2-5/mo | Everywhere |
| **NB-IoT** | Good | Very Low | ~€1-2/mo | Everywhere |
| **Bluetooth** | Very Limited | Very Low | Free | Proximity |

**Selected:** **LTE-M** with WiFi fallback - Best balance

---

## 3. Final Architecture

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│  Dog Collar │────▶│  MQTT Broker │────▶│ IoT Consumer│
│  (ESP32)    │     │  (EMQX)     │     │  Service    │
└─────────────┘     └─────────────┘     └──────┬──────┘
                                                │
                                                ▼
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│  Mobile App │◀────│WebSocket GW │◀────│  MongoDB    │
│  (ReactNat) │     │  (Socket.io)│     │             │
└─────────────┘     └─────────────┘     └─────────────┘
```

---

## 4. Battery Optimization Strategy

| Mode | Interval | Battery Life |
|------|----------|--------------|
| Active (Moving) | 10 sec | ~2 days |
| Idle (Sleeping) | 2 min | ~7 days |
| Low Power | 10 min | ~14 days |
| Standby | Only on alert | ~30 days |

**Adaptive Tracking:**
- Dog moving → 10 second updates
- Dog stationary → 2 minute updates
- Low battery (<20%) → 10 minute updates
- Power save mode → Only on request
