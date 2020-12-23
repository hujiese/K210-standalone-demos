package com.scut.hujie.k210nettyandroid;

import android.os.Handler;
import android.util.Log;

import io.netty.buffer.ByteBuf;
import io.netty.channel.ChannelHandlerContext;
import io.netty.handler.codec.ByteToMessageDecoder;
import io.netty.handler.codec.CorruptedFrameException;
import io.netty.handler.codec.DecoderException;
import io.netty.handler.codec.TooLongFrameException;
import java.nio.ByteOrder;
import java.util.List;

/**
 * Created by jack on 2020/12/23.
 */

public class MyLengthFieldBasedFrameDecoder extends ByteToMessageDecoder {
    private final ByteOrder byteOrder;
    private final int maxFrameLength;
    private final int lengthFieldOffset;
    private final int lengthFieldLength;
    private final int lengthFieldEndOffset;
    private final int lengthAdjustment;
    private final int initialBytesToStrip;
    private final boolean failFast;
    private boolean discardingTooLongFrame;
    private long tooLongFrameLength;
    private long bytesToDiscard;
    private Handler handler;

    public MyLengthFieldBasedFrameDecoder(int maxFrameLength, int lengthFieldOffset, int lengthFieldLength, Handler handler) {
        this(maxFrameLength, lengthFieldOffset, lengthFieldLength, 0, 0, handler);
    }

    public MyLengthFieldBasedFrameDecoder(int maxFrameLength, int lengthFieldOffset, int lengthFieldLength, int lengthAdjustment, int initialBytesToStrip, Handler handler) {
        this(maxFrameLength, lengthFieldOffset, lengthFieldLength, lengthAdjustment, initialBytesToStrip, true, handler);

    }

    public MyLengthFieldBasedFrameDecoder(int maxFrameLength, int lengthFieldOffset, int lengthFieldLength, int lengthAdjustment, int initialBytesToStrip, boolean failFast, Handler handler) {
        this(ByteOrder.BIG_ENDIAN, maxFrameLength, lengthFieldOffset, lengthFieldLength, lengthAdjustment, initialBytesToStrip, failFast, handler);
    }

    public MyLengthFieldBasedFrameDecoder(ByteOrder byteOrder, int maxFrameLength, int lengthFieldOffset, int lengthFieldLength, int lengthAdjustment, int initialBytesToStrip, boolean failFast, Handler handler) {
        if(byteOrder == null) {
            throw new NullPointerException("byteOrder");
        } else if(maxFrameLength <= 0) {
            throw new IllegalArgumentException("maxFrameLength must be a positive integer: " + maxFrameLength);
        } else if(lengthFieldOffset < 0) {
            throw new IllegalArgumentException("lengthFieldOffset must be a non-negative integer: " + lengthFieldOffset);
        } else if(initialBytesToStrip < 0) {
            throw new IllegalArgumentException("initialBytesToStrip must be a non-negative integer: " + initialBytesToStrip);
        } else if(lengthFieldOffset > maxFrameLength - lengthFieldLength) {
            throw new IllegalArgumentException("maxFrameLength (" + maxFrameLength + ") must be equal to or greater than lengthFieldOffset (" + lengthFieldOffset + ") + lengthFieldLength (" + lengthFieldLength + ").");
        } else {
            this.byteOrder = byteOrder;
            this.maxFrameLength = maxFrameLength;
            this.lengthFieldOffset = lengthFieldOffset;
            this.lengthFieldLength = lengthFieldLength;
            this.lengthAdjustment = lengthAdjustment;
            this.lengthFieldEndOffset = lengthFieldOffset + lengthFieldLength;
            this.initialBytesToStrip = initialBytesToStrip;
            this.failFast = failFast;
            this.handler = handler;
        }
    }

    protected final void decode(ChannelHandlerContext ctx, ByteBuf in, List<Object> out) throws Exception {
        Object decoded = this.decode(ctx, in);
        if(decoded != null) {
            out.add(decoded);
        }

    }

    private void discardingTooLongFrame(ByteBuf in) {
        long bytesToDiscard = this.bytesToDiscard;
        int localBytesToDiscard = (int)Math.min(bytesToDiscard, (long)in.readableBytes());
        in.skipBytes(localBytesToDiscard);
        bytesToDiscard -= (long)localBytesToDiscard;
        this.bytesToDiscard = bytesToDiscard;
        this.failIfNecessary(false);
    }

    private static void failOnNegativeLengthField(ByteBuf in, long frameLength, int lengthFieldEndOffset) {
        in.skipBytes(lengthFieldEndOffset);
        throw new CorruptedFrameException("negative pre-adjustment length field: " + frameLength);
    }

    private static void failOnFrameLengthLessThanLengthFieldEndOffset(ByteBuf in, long frameLength, int lengthFieldEndOffset) {
        in.skipBytes(lengthFieldEndOffset);
        throw new CorruptedFrameException("Adjusted frame length (" + frameLength + ") is less than lengthFieldEndOffset: " + lengthFieldEndOffset);
    }

    private void exceededFrameLength(ByteBuf in, long frameLength) {
        long discard = frameLength - (long)in.readableBytes();
        this.tooLongFrameLength = frameLength;
        if(discard < 0L) {
            in.skipBytes((int)frameLength);
        } else {
            this.discardingTooLongFrame = true;
            this.bytesToDiscard = discard;
            in.skipBytes(in.readableBytes());
        }

        this.failIfNecessary(true);
    }

    private static void failOnFrameLengthLessThanInitialBytesToStrip(ByteBuf in, long frameLength, int initialBytesToStrip) {
        in.skipBytes((int)frameLength);
        throw new CorruptedFrameException("Adjusted frame length (" + frameLength + ") is less than initialBytesToStrip: " + initialBytesToStrip);
    }

    protected Object decode(ChannelHandlerContext ctx, ByteBuf in) throws Exception {
        if(this.discardingTooLongFrame) {
            this.discardingTooLongFrame(in);
        }

        if(in.readableBytes() < this.lengthFieldEndOffset) {
            return null;
        } else {
            int actualLengthFieldOffset = in.readerIndex() + this.lengthFieldOffset;
            long frameLength = this.getUnadjustedFrameLength(in, actualLengthFieldOffset, this.lengthFieldLength, this.byteOrder);
            if(frameLength < 0L) {
                failOnNegativeLengthField(in, frameLength, this.lengthFieldEndOffset);
            }

            frameLength += (long)(this.lengthAdjustment + this.lengthFieldEndOffset);
            if(frameLength < (long)this.lengthFieldEndOffset) {
                failOnFrameLengthLessThanLengthFieldEndOffset(in, frameLength, this.lengthFieldEndOffset);
            }

            if(frameLength > (long)this.maxFrameLength) {
                this.exceededFrameLength(in, frameLength);
                return null;
            } else {
                int frameLengthInt = (int)frameLength;
                if(in.readableBytes() < frameLengthInt) {
                    return null;
                } else {
                    if(this.initialBytesToStrip > frameLengthInt) {
                        failOnFrameLengthLessThanInitialBytesToStrip(in, frameLength, this.initialBytesToStrip);
                    }

                    in.skipBytes(this.initialBytesToStrip);
                    int readerIndex = in.readerIndex();
                    int actualFrameLength = frameLengthInt - this.initialBytesToStrip;
                    ByteBuf frame = this.extractFrame(ctx, in, readerIndex, actualFrameLength);
                    in.readerIndex(readerIndex + actualFrameLength);
                    return frame;
                }
            }
        }
    }

    protected long getUnadjustedFrameLength(ByteBuf buf, int offset, int length, ByteOrder order) {
        buf = buf.order(order);
        long frameLength;
        switch(length) {
            case 1:
                frameLength = (long)buf.getUnsignedByte(offset);
                break;
            case 2:
                frameLength = (long)buf.getUnsignedShort(offset);
                break;
            case 3:
                frameLength = (long)buf.getUnsignedMedium(offset);
                break;
            case 4:
                frameLength = buf.getUnsignedInt(offset);
                break;
            case 5:
            case 6:
            case 7:
            default:
                throw new DecoderException("unsupported lengthFieldLength: " + this.lengthFieldLength + " (expected: 1, 2, 3, 4, or 8)");
            case 8:
                frameLength = buf.getLong(offset);
        }

        return frameLength;
    }

    private void failIfNecessary(boolean firstDetectionOfTooLongFrame) {
        if(this.bytesToDiscard == 0L) {
            long tooLongFrameLength = this.tooLongFrameLength;
            this.tooLongFrameLength = 0L;
            this.discardingTooLongFrame = false;
            if(!this.failFast || firstDetectionOfTooLongFrame) {
                this.fail(tooLongFrameLength);
            }
        } else if(this.failFast && firstDetectionOfTooLongFrame) {
            this.fail(this.tooLongFrameLength);
        }

    }

    protected ByteBuf extractFrame(ChannelHandlerContext ctx, ByteBuf buffer, int index, int length) {
        return buffer.slice(index, length).retain();
    }

    private void fail(long frameLength) {
        if(frameLength > 0L) {
            Log.e("ERROR- ", "Adjusted frame length exceeds " + this.maxFrameLength + ": " + frameLength + " - discarded");
//            throw new TooLongFrameException("Adjusted frame length exceeds " + this.maxFrameLength + ": " + frameLength + " - discarded");
        } else {
            Log.e("ERROR- ", "Adjusted frame length exceeds " + this.maxFrameLength + " - discarding");
//            throw new TooLongFrameException("Adjusted frame length exceeds " + this.maxFrameLength + " - discarding");
        }
        handler.obtainMessage(0x03).sendToTarget();
    }
}

