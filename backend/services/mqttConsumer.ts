/**
 * MQTT Consumer Service
 * Listens for GPS data from dog collars and processes it
 */

import mqtt, { MqttClient } from 'mqtt';
import { Device, Location, SafeZone, Alert, Dog } from '../models/index.js';

interface LocationPayload {
  deviceId: string;
  latitude: number;
  longitude: number;
  accuracy: number;
  altitude: number;
  speed: number;
  heading: number;
  battery: number;
  timestamp: number;
  wifiRssi: number;
}

class MqttConsumer {
  private client: MqttClient;
  private topicPrefix = 'dogs/collar-';

  constructor() {
    const broker = process.env.MQTT_BROKER || 'mqtt://localhost:1883';
    const username = process.env.MQTT_USER;
    const password = process.env.MQTT_PASSWORD;

    this.client = mqtt.connect(broker, {
      username,
      password,
      clientId: 'gps-backend-consumer',
    });

    this.setupHandlers();
  }

  private setupHandlers(): void {
    this.client.on('connect', () => {
      console.log('✅ MQTT Consumer connected');
      
      // Subscribe to all collar locations
      this.client.subscribe(`${this.topicPrefix}+/location`, (err) => {
        if (err) {
          console.error('❌ MQTT subscription error:', err);
        } else {
          console.log('✅ Subscribed to collar locations');
        }
      });
    });

    this.client.on('message', (topic, message) => {
      this.handleMessage(topic, message.toString());
    });

    this.client.on('error', (err) => {
      console.error('❌ MQTT error:', err);
    });
  }

  private async handleMessage(topic: string, payload: string): Promise<void> {
    try {
      // Extract device ID from topic: dogs/collar-001/location
      const parts = topic.split('/');
      const deviceId = parts[1];
      
      const data: LocationPayload = JSON.parse(payload);
      
      console.log(`📍 Location received from ${deviceId}:`, data.latitude, data.longitude);
      
      // Find the device in database
      const device = await Device.findOne({ deviceId });
      if (!device) {
        console.warn(`⚠️ Unknown device: ${deviceId}`);
        return;
      }

      // Save location
      const location = new Location({
        deviceId: device._id,
        userId: device.userId,
        latitude: data.latitude,
        longitude: data.longitude,
        accuracy: data.accuracy,
        altitude: data.altitude,
        speed: data.speed,
        heading: data.heading,
        battery: data.battery,
        timestamp: new Date(data.timestamp * 1000),
      });
      
      await location.save();

      // Update device status
      device.status = 'online';
      device.battery = data.battery;
      device.lastSeen = new Date();
      await device.save();

      // Check safe zones
      await this.checkSafeZones(device, data);

      // Check battery
      if (data.battery < 20 && device.status !== 'low_battery') {
        await this.createBatteryAlert(device);
      }

    } catch (error) {
      console.error('❌ Error processing message:', error);
    }
  }

  private async checkSafeZones(device: any, data: LocationPayload): Promise<void> {
    // Find active safe zones for this device
    const dog = await Dog.findOne({ deviceId: device._id });
    if (!dog) return;

    const safeZones = await SafeZone.find({
      dogId: dog._id,
      active: true,
    });

    for (const zone of safeZones) {
      const distance = this.calculateDistance(
        data.latitude,
        data.longitude,
        zone.centerLat,
        zone.centerLng
      );

      const isInside = distance <= zone.radius;
      
      // TODO: Track previous state to detect zone crossings
      // For now, just log
      console.log(`📍 Dog ${dog.name} is ${isInside ? 'inside' : 'outside'} zone ${zone.name} (${distance}m)`);
    }
  }

  private calculateDistance(lat1: number, lon1: number, lat2: number, lon2: number): number {
    // Haversine formula
    const R = 6371e3; // Earth's radius in meters
    const φ1 = (lat1 * Math.PI) / 180;
    const φ2 = (lat2 * Math.PI) / 180;
    const Δφ = ((lat2 - lat1) * Math.PI) / 180;
    const Δλ = ((lon2 - lon1) * Math.PI) / 180;

    const a =
      Math.sin(Δφ / 2) * Math.sin(Δφ / 2) +
      Math.cos(φ1) * Math.cos(φ2) * Math.sin(Δλ / 2) * Math.sin(Δλ / 2);
    const c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));

    return R * c;
  }

  private async createBatteryAlert(device: any): Promise<void> {
    device.status = 'low_battery';
    await device.save();

    const dog = await Dog.findOne({ deviceId: device._id });
    if (!dog) return;

    const alert = new Alert({
      userId: device.userId,
      dogId: dog._id,
      deviceId: device._id,
      type: 'low_battery',
      title: 'Low Battery Warning',
      message: `${dog.name}'s collar battery is low (${device.battery}%)`,
    });

    await alert.save();
  }

  start(): void {
    console.log('🚀 MQTT Consumer starting...');
  }
}

// Export singleton
export const mqttConsumer = new MqttConsumer();
