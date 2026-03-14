import mongoose, { Document, Schema } from 'mongoose';

// Dog Schema
export interface IDog extends Document {
  userId: mongoose.Types.ObjectId;
  name: string;
  breed: string;
  age: number;
  weight: number;
  photo: string;
  deviceId: mongoose.Types.ObjectId;
  createdAt: Date;
  updatedAt: Date;
}

const dogSchema = new Schema<IDog>({
  userId: {
    type: Schema.Types.ObjectId,
    ref: 'User',
    required: true,
    index: true,
  },
  name: {
    type: String,
    required: true,
    trim: true,
  },
  breed: {
    type: String,
    trim: true,
  },
  age: {
    type: Number,
    min: 0,
  },
  weight: {
    type: Number,
    min: 0,
  },
  photo: {
    type: String,
  },
  deviceId: {
    type: Schema.Types.ObjectId,
    ref: 'Device',
  },
}, {
  timestamps: true,
});

dogSchema.index({ userId: 1 });

export const Dog = mongoose.model<IDog>('Dog', dogSchema);
