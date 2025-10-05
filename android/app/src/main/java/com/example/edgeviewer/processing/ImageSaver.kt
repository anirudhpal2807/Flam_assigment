package com.example.edgeviewer.processing

import android.content.ContentValues
import android.content.Context
import android.graphics.Bitmap
import android.graphics.Color
import android.os.Build
import android.os.Environment
import android.provider.MediaStore
import java.io.OutputStream
import java.nio.ByteBuffer
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

object ImageSaver {

    fun saveGrayscalePng(context: Context, gray: ByteBuffer, width: Int, height: Int, displayName: String? = null): String? {
        val bytes = ByteArray(gray.capacity())
        gray.rewind(); gray.get(bytes)

        val bmp = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888)
        var i = 0
        for (y in 0 until height) {
            for (x in 0 until width) {
                val g = bytes[i].toInt() and 0xFF
                bmp.setPixel(x, y, Color.argb(255, g, g, g))
                i++
            }
        }

        val name = displayName ?: timestampedName()
        val uri = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
            val values = ContentValues().apply {
                put(MediaStore.Images.Media.DISPLAY_NAME, "$name.png")
                put(MediaStore.Images.Media.MIME_TYPE, "image/png")
                put(MediaStore.Images.Media.RELATIVE_PATH, Environment.DIRECTORY_PICTURES + "/EdgeViewer")
                put(MediaStore.Images.Media.IS_PENDING, 1)
            }
            val resolver = context.contentResolver
            val collection = MediaStore.Images.Media.getContentUri(MediaStore.VOLUME_EXTERNAL_PRIMARY)
            val itemUri = resolver.insert(collection, values) ?: return null
            resolver.openOutputStream(itemUri).use { out -> bmp.compress(Bitmap.CompressFormat.PNG, 100, itOrNull(out)) }
            values.clear(); values.put(MediaStore.Images.Media.IS_PENDING, 0)
            resolver.update(itemUri, values, null, null)
            itemUri
        } else {
            @Suppress("DEPRECATION")
            val pictures = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES)
            val dir = java.io.File(pictures, "EdgeViewer").apply { mkdirs() }
            val file = java.io.File(dir, "$name.png")
            file.outputStream().use { out -> bmp.compress(Bitmap.CompressFormat.PNG, 100, out) }
            android.net.Uri.fromFile(file)
        }

        bmp.recycle()
        return uri.toString()
    }

    private fun itOrNull(out: OutputStream?): OutputStream {
        return out ?: object : OutputStream() { override fun write(b: Int) {} }
    }

    private fun timestampedName(): String = SimpleDateFormat("yyyyMMdd_HHmmss", Locale.US).format(Date())
}


