import mongoose, { Document, Schema } from 'mongoose';

// GPS Device Schema (for collars)
export interface IDevice extends Document {
  userId: mongoose.Types.ObjectId;
  type: 'feeder' | 'water' | 'collar';
  name: string;
  deviceId: string; // Hardware ID
  status: 'online' | 'offline' | 'low_battery';
  battery: number;
  firmware: string;
  lastSeen: Date;
  settings: {
    trackingInterval: number;
    sleepMode: boolean;
  };
  createdAt: Date;
  updatedAt: Date;
}

const deviceSchema = new Schema<IDevice>({
  userId: {
    type: Schema.Types.ObjectId,
    ref: 'User',
    required: true,
    index: true,
  },
  type: {
    type: String,
    enum: ['feeder', 'water', 'collar'],
    required: true,
  },
  name: {
    type: String,
    required: true,
    trim: true,
  },
  deviceId: {
    type: String,
    required: true,
    unique: true,
    index: true,
  },
  status: {
    type: String,
    enum: ['online', 'offline', 'low_battery'],
    default: 'offline',
  },
  battery: {
    type: Number,
    min: 0,
    max: 100,
    default: 100,
  },
  firmware: {
    type: String,
    default: '1.0.0',
  },
  lastSeen: {
    type: Date,
    default: Date.now,
  },
  settings: {
    trackingInterval: {
      type: Number,
      default: 60,
    },
    sleepMode: {
      type: Boolean,
      default: false,
    },
  },
}, {
  timestamps: true,
});

deviceSchema.index({ userId: 1, type: 1 });

export const Device = mongoose.model<IDevice>('Device', deviceSchema);
