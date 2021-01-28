@file:Suppress("DEPRECATION")

package com.standisland.god.media.decoder

import android.graphics.SurfaceTexture
import android.media.MediaCodec
import android.media.MediaFormat
import android.view.Surface
import com.tencent.mars.xlog.Log
import java.io.IOException
import java.lang.Exception
import java.lang.IllegalStateException
import java.nio.ByteBuffer
import java.nio.ByteOrder

/**
 * 基于android的 MediaCodec 来实现硬解码
 *
 */
class MediaCodecDecode {

    private var mMediaCodec: MediaCodec? = null
    private var mFormat: MediaFormat? = null
    private var mSurfaceTexture: SurfaceTexture? = null
    private var mOutputSurface: Surface? = null
    private var mOutputBufferInfo = MediaCodec.BufferInfo()
    private val mBuffer = ByteBuffer.allocateDirect(16)
    private val mChangeBuffer = ByteBuffer.allocateDirect(12)
    private val mMatrix = FloatArray(16)
    private var mOnFrameAvailable = false

    init {
        mBuffer.order(ByteOrder.BIG_ENDIAN)
        mChangeBuffer.order(ByteOrder.BIG_ENDIAN)
    }

    fun start(
        textureId: Int, codecName: String, width: Int, height: Int,
        csd0: ByteBuffer?, csd1: ByteBuffer?
    ): Int {
        try {
            Log.i(
                "MediaCodecDecode",
                "enter MediaCodec Start textureId: $textureId codecName: $codecName width: $width height: $height"
            )
            mMediaCodec = MediaCodec.createDecoderByType(codecName)
            mFormat = MediaFormat()
            mFormat?.setString(MediaFormat.KEY_MIME, codecName)
            mFormat?.setInteger(MediaFormat.KEY_WIDTH, width)
            mFormat?.setInteger(MediaFormat.KEY_HEIGHT, height)
            when (codecName) {
                "video/avc" -> {
                    if (csd0 != null && csd1 != null) {
                        mFormat?.setByteBuffer("csd-0", csd0)
                        mFormat?.setByteBuffer("csd-1", csd1)
                    }
                }

                "video/hevc" -> {
                    if (csd0 != null) {
                        mFormat?.setByteBuffer("csd-0", csd0)
                    }
                }

                "video/mp4v-es" -> {
                    mFormat?.setByteBuffer("csd-0", csd0)
                }

                "video/3gpp" -> {
                    mFormat?.setByteBuffer("csd-0", csd0)
                }
            }
            mSurfaceTexture = SurfaceTexture(textureId)
            mSurfaceTexture?.setOnFrameAvailableListener {
                mOnFrameAvailable = true
            }
            mOutputSurface = Surface(mSurfaceTexture)
            mMediaCodec?.configure(mFormat, mOutputSurface, null, 0)
            mMediaCodec?.start()
        } catch (e: IOException) {
            e.printStackTrace()
            Log.e("MediaCodecDecode", e.message)
            return -1
        }
        Log.i("MediaCodecDecode", "leave MediaCodec Start")
        return 0
    }

    fun stop() {
        Log.i("MediaCodecDecode", "enter MediaCodec Stop")
        flush()
        mMediaCodec?.stop()
        mMediaCodec?.release()
        mSurfaceTexture?.release()
        mSurfaceTexture = null
        mOutputSurface?.release()
        mOutputSurface = null
        mMediaCodec = null
        mFormat = null
        mOnFrameAvailable = false
        Log.i("MediaCodecDecode", "leave MediaCodec Stop")
    }

    fun seek() {
        while (true) {
            val id = mMediaCodec?.dequeueOutputBuffer(mOutputBufferInfo, 0) ?: -1
            if (id < 0) {
                break
            }
            Log.i(
                "MediaCodecDecode",
                "flush id: $id infoTime: ${mOutputBufferInfo.presentationTimeUs / 1000}"
            )
            releaseOutputBuffer(id, false);
        }
    }

    fun flush() {
        try {
            mMediaCodec?.flush()
            Log.i("MediaCodecDecode", "flush ok")
        } catch (e: IllegalStateException) {
            e.printStackTrace()
            Log.i("MediaCodecDecode", "flush error - " + e.message)
        }
    }

    fun dequeueInputBuffer(timeout: Long): Int {
        return try {
            val result = mMediaCodec?.dequeueInputBuffer(timeout) ?: -1000
            Log.i("MediaCodecDecode", "dequeueInputBuffer ok");
            result
        } catch (e: IllegalStateException) {
            e.printStackTrace()
            Log.i("MediaCodecDecode", "dequeueInputBuffer error - " + e.message)
            -1
        }
    }

    /**
     * 获取一个空的输入buffer
     */
    fun getInputBuffer(id: Int): ByteBuffer? {
        val buffer = mMediaCodec?.inputBuffers?.get(id)
        buffer?.clear()
        return buffer
    }

    fun queueInputBuffer(id: Int, size: Int, pts: Long, flags: Int) {
        try {
            mMediaCodec?.queueInputBuffer(id, 0, size, pts, flags)
        } catch (e: IllegalStateException) {
            e.printStackTrace()
            Log.i("MediaCodecDecode", "queueInputBuffer error - " + e.message)
        }
    }

    fun dequeueOutputBufferIndex(timeout: Long): ByteBuffer? {
        return try {
            val id = mMediaCodec?.dequeueOutputBuffer(mOutputBufferInfo, timeout) ?: -1000
            mBuffer.clear();
            mBuffer.position(0)
            mBuffer.putInt(id)
            if (id >= 0) {
//        Log.e("trinity", "output id: $id time: ${mOutputBufferInfo.presentationTimeUs / 1000} flags: ${mOutputBufferInfo.flags} buffer: ${mBuffer.remaining()}")
                mBuffer.putInt(mOutputBufferInfo.flags)
                mBuffer.putLong(mOutputBufferInfo.presentationTimeUs)
            }
            mBuffer
        } catch (e: IllegalStateException) {
            e.printStackTrace()
            Log.i("MediaCodecDecode", "dequeueOutputBufferIndex error - " + e.message)
            null
        }
    }

    fun releaseOutputBuffer(id: Int, boolean: Boolean) {
        try {
            mMediaCodec?.releaseOutputBuffer(id, boolean)
        } catch (e: Exception) {
            e.printStackTrace()
            Log.i("MediaCodecDecode", "releaseOutputBuffer error - " + e.message)
        }
    }

    fun formatChange(): ByteBuffer {
        val newFormat = mMediaCodec?.outputFormat
        val width = newFormat?.getInteger(MediaFormat.KEY_WIDTH) ?: -1
        val height = newFormat?.getInteger(MediaFormat.KEY_HEIGHT) ?: -1
        val colorFormat = newFormat?.getInteger(MediaFormat.KEY_COLOR_FORMAT) ?: -1
        mChangeBuffer.position(0)
        mChangeBuffer.putInt(width)
        mChangeBuffer.putInt(height)
        mChangeBuffer.putInt(colorFormat)
        return mChangeBuffer
    }

    fun getOutputBuffer(id: Int): ByteBuffer? {
        return mMediaCodec?.outputBuffers?.get(id)
    }

    fun updateTexImage() {
        mSurfaceTexture?.updateTexImage()
    }

    fun getTransformMatrix(): FloatArray {
        mSurfaceTexture?.getTransformMatrix(mMatrix)
        return mMatrix
    }

    fun frameAvailable(): Boolean {
        return mOnFrameAvailable
    }
}