import mongoose, { Document, Schema } from 'mongoose';

// Alert Schema
export interface IAlert extends Document {
  userId: mongoose.Types.ObjectId;
  dogId: mongoose.Types.ObjectId;
  deviceId: mongoose.Types.ObjectId;
  type: 'safe_zone_enter' | 'safe_zone_exit' | 'low_battery' | 'offline' | 'geofence' | 'manual';
  title: string;
  message: string;
  location: {
    lat: number;
    lng: number;
  };
  acknowledged: boolean;
  timestamp: Date;
}

const alertSchema = new Schema<IAlert>({
  userId: {
    type: Schema.Types.ObjectId,
    ref: 'User',
    required: true,
    index: true,
  },
  dogId: {
    type: Schema.Types.ObjectId,
    ref: 'Dog',
    required: true,
  },
  deviceId: {
    type: Schema.Types.ObjectId,
    ref: 'Device',
  },
  type: {
    type: String,
    enum: ['safe_zone_enter', 'safe_zone_exit', 'low_battery', 'offline', 'geofence', 'manual'],
    required: true,
  },
  title: {
    type: String,
    required: true,
  },
  message: {
    type: String,
    required: true,
  },
  location: {
    lat: Number,
    lng: Number,
  },
  acknowledged: {
    type: Boolean,
    default: false,
  },
  timestamp: {
    type: Date,
    default: Date.now,
    index: true,
  },
});

// Indexes
alertSchema.index({ userId: 1, acknowledged: 1 });
alertSchema.index({ userId: 1, timestamp: -1 });

// TTL - keep alerts for 90 days
alertSchema.index({ timestamp: 1 }, { expireAfterSeconds: 90 * 24 * 60 * 60 });

export const Alert = mongoose.model<IAlert>('Alert', alertSchema);
