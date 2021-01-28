package com.standisland.god.demo

import android.app.Application
import android.text.TextUtils
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch
import java.io.*

class GodFFApplication : Application() {

    override fun onCreate() {
        super.onCreate()
        instance = this
        xcrash.XCrash.init(this);
        val effectLocalDir = externalCacheDir?.absolutePath + "/effect"
        val effectDir = File(effectLocalDir)
        if (!effectDir.exists()) {
            GlobalScope.launch(Dispatchers.IO) {
                copyAssets("effect", effectLocalDir)
            }
        }
    }

//    override fun onCreateProfile(p0: Application?, p1: String?): AppProfile {
//       return MainAppProfile(ProcessTag.getProcessTag(this))
//    }

    companion object {
        init {
//            System.loadLibrary("trinity")
//            System.loadLibrary("c++_shared")
//            System.loadLibrary("marsxlog")
        }
        var instance: GodFFApplication? = null
            private set

    }

    /**
     * 拷贝assets文件下文件到指定路径
     * @param assetDir  源文件/文件夹
     * @param dir  目标文件夹
     */
    private fun copyAssets(assetDir: String, targetDir: String) {
        if (TextUtils.isEmpty(assetDir) || TextUtils.isEmpty(targetDir)) {
            return
        }
        val separator = File.separator
        try {
            // 获取assets目录assetDir下一级所有文件以及文件夹
            val fileNames = assets.list(assetDir) ?: return
            // 如果是文件夹(目录),则继续递归遍历
            if (fileNames.isNotEmpty()) {
                val targetFile = File(targetDir)
                if (!targetFile.exists() && !targetFile.mkdirs()) {
                    return
                }
                for (fileName in fileNames) {
                    copyAssets(assetDir + separator + fileName, targetDir + separator + fileName)
                }
            } else { // 文件,则执行拷贝
                copy(assetDir, targetDir)
            }
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }

    private fun copy(source: String, targetPath: String) {
        if (TextUtils.isEmpty(source) || TextUtils.isEmpty(targetPath)) {
            return
        }
        val dest = File(targetPath)
        dest.parentFile?.mkdirs()
        try {
            val inputStream = BufferedInputStream(assets.open(source))
            val out = BufferedOutputStream(FileOutputStream(dest))
            val buffer = ByteArray(2048)
            var length: Int
            while (true) {
                length = inputStream.read(buffer)
                if (length < 0) {
                    break
                }
                out.write(buffer, 0, length)
            }
            out.close()
            inputStream.close()
        } catch (e: FileNotFoundException) {
            e.printStackTrace()
        } catch (e: IOException) {
            e.printStackTrace()
        }
    }

}