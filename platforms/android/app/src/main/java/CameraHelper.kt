package imgui.example.android

import android.Manifest
import android.content.Context
import android.content.pm.PackageManager
import android.graphics.Bitmap
import android.graphics.ImageFormat
import android.graphics.YuvImage
import android.media.Image
import android.os.Build
import android.util.Log
import android.util.Size
import androidx.camera.core.*
import androidx.camera.lifecycle.ProcessCameraProvider
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import java.nio.ByteBuffer
import java.util.concurrent.Executors

class CameraHelper(private val context: Context) {

    companion object {
        init {
            System.loadLibrary("jni-camera")
        }
    }

    private var cameraProvider: ProcessCameraProvider? = null
    private val cameraExecutor = Executors.newSingleThreadExecutor()

    external fun onImageAvailable(data: ByteArray, width: Int, height: Int)

    fun openCamera() {
        if (ActivityCompat.checkSelfPermission(context, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
            Log.e("CameraHelper", "Разрешение на камеру не предоставлено")
            return
        }

        val cameraProviderFuture = ProcessCameraProvider.getInstance(context)
        cameraProviderFuture.addListener({
            cameraProvider = cameraProviderFuture.get()
            startCamera()
        }, ContextCompat.getMainExecutor(context))
    }

    private fun startCamera() {
        val cameraProvider = cameraProvider ?: return
        val cameraSelector = CameraSelector.DEFAULT_BACK_CAMERA

        val imageAnalysis = ImageAnalysis.Builder()
            .setTargetResolution(Size(640, 480))
            .setOutputImageFormat(ImageAnalysis.OUTPUT_IMAGE_FORMAT_RGBA_8888)
            .setBackpressureStrategy(ImageAnalysis.STRATEGY_KEEP_ONLY_LATEST)
            .build()

        imageAnalysis.setAnalyzer(cameraExecutor) { imageProxy ->
            val rgbaBuffer = imageProxy.planes[0].buffer
            val rgbaData = ByteArray(rgbaBuffer.remaining())
            rgbaBuffer.get(rgbaData)

            onImageAvailable(rgbaData, imageProxy.width, imageProxy.height)
            imageProxy.close()
        }

        cameraProvider.unbindAll()
        cameraProvider.bindToLifecycle(context as androidx.lifecycle.LifecycleOwner, cameraSelector, imageAnalysis)
    }

    fun closeCamera() {
        cameraProvider?.unbindAll()
        cameraExecutor.shutdown()
    }
}
