package imgui.example.android

import android.Manifest
import android.content.Context
import android.content.pm.PackageManager
import android.util.Log
import android.util.Size
import androidx.camera.core.*
import androidx.camera.core.resolutionselector.*
import androidx.camera.lifecycle.ProcessCameraProvider
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import java.util.concurrent.ExecutorService
import java.util.concurrent.Executors

class CameraHelper(private val context: Context) {

    companion object {
        init {
            System.loadLibrary("jni-camera")
        }
    }

    private var cameraProvider: ProcessCameraProvider? = null
    private var imageAnalysis: ImageAnalysis? = null
    private var cameraExecutor: ExecutorService? = Executors.newSingleThreadExecutor()

    external fun onImageAvailable(data: ByteArray, width: Int, height: Int)

    fun openCamera() {
        if (ActivityCompat.checkSelfPermission(context, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
            Log.e("CameraHelperAndroid", "Разрешение на камеру не предоставлено")
            return
        }

        Log.i("CameraHelperAndroid", "openCamera")

        // Восстанавливаем поток выполнения, если он был закрыт
        if (cameraExecutor == null || cameraExecutor!!.isShutdown) {
            cameraExecutor = Executors.newSingleThreadExecutor()
        }

        val cameraProviderFuture = ProcessCameraProvider.getInstance(context)
        cameraProviderFuture.addListener({
            cameraProvider = cameraProviderFuture.get()
            startCamera()
        }, ContextCompat.getMainExecutor(context))
    }

    private fun startCamera() {
        Log.i("CameraHelperAndroid", "startCamera")
        val cameraProvider = cameraProvider ?: return
        val cameraSelector = CameraSelector.DEFAULT_BACK_CAMERA

        val resolutionSelector = ResolutionSelector.Builder()
            .setResolutionStrategy(ResolutionStrategy(Size(960, 720), ResolutionStrategy.FALLBACK_RULE_NONE))
            .build()

        imageAnalysis = ImageAnalysis.Builder()
            .setResolutionSelector(resolutionSelector)
            .setOutputImageFormat(ImageAnalysis.OUTPUT_IMAGE_FORMAT_RGBA_8888)
            .setBackpressureStrategy(ImageAnalysis.STRATEGY_KEEP_ONLY_LATEST)
            .build()

        imageAnalysis?.setAnalyzer(cameraExecutor!!) { imageProxy ->
            try {
                val rgbaBuffer = imageProxy.planes[0].buffer
                val rgbaData = ByteArray(rgbaBuffer.remaining())
                rgbaBuffer.get(rgbaData)

                Log.d("CameraHelperAndroid", "sended data")
                onImageAvailable(rgbaData, imageProxy.width, imageProxy.height)
            } catch (e: Exception) {
                Log.e("CameraHelperAndroid", "Error processing image", e)
            } finally {
                imageProxy.close()
            }
        }

        cameraProvider.unbindAll()
        cameraProvider.bindToLifecycle(context as androidx.lifecycle.LifecycleOwner, cameraSelector, imageAnalysis!!)
    }

    fun closeCamera() {
        Log.d("CameraHelperAndroid", "Closing camera")
        imageAnalysis?.clearAnalyzer()
        cameraProvider?.unbindAll()
        cameraExecutor?.shutdown()
        cameraExecutor = null
        imageAnalysis = null
        cameraProvider = null
    }
}
