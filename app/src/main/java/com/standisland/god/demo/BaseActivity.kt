package com.standisland.god.demo

import android.Manifest
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.text.TextUtils
import android.view.Menu
import android.view.MenuItem
import android.view.View
import android.widget.Toast
import androidx.annotation.LayoutRes
import androidx.appcompat.app.AppCompatActivity

/**
 * 可选择媒体的activity
 */
abstract class BaseActivity : AppCompatActivity(){
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M && checkSelfPermission(permissions[0]) != PackageManager.PERMISSION_GRANTED && checkSelfPermission(
                permissions[1]
            ) != PackageManager.PERMISSION_GRANTED
        ) {
            requestPermission()
        }
        setContentView(getLayoutId())
    }

    private fun requestPermission() {
        requestPermission(permissions)
    }

    protected fun requestPermission(permissions: Array<String>?) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            requestPermissions(permissions!!,
                REQUEST_CODE
            )
        }
    }

    protected fun selectFile() {
        val intent = Intent(Intent.ACTION_GET_CONTENT)
        intent.addCategory(Intent.CATEGORY_OPENABLE)
        intent.type = "*/*"
        intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION)
        this.startActivityForResult(intent, 123)
    }

    override fun onActivityResult(
        requestCode: Int,
        resultCode: Int,
        data: Intent?
    ) {
        super.onActivityResult(requestCode, resultCode, data)
//        if (data != null && data.data != null) {
//            val filePath = ContentUtils.getPath(this, data.data)
//            Log.i(TAG, "filePath=$filePath")
//            onSelectedFile(filePath)
//        }
    }

    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        menuInflater.inflate(R.menu.menu_setting, menu)
        return true
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        when (item.itemId) {
            R.id.menu_select -> selectFile()
            else -> {
            }
        }
        return super.onOptionsItemSelected(item)
    }

    protected fun showToast(msg: String?) {
        if (TextUtils.isEmpty(msg)) {
            return
        }
        Toast.makeText(this, msg, Toast.LENGTH_SHORT).show()
    }

    protected fun showSelectFile() {
        showToast(getString(R.string.please_select))
    }

    protected fun <T : View?> getView(viewId: Int): T {
        return findViewById<View>(viewId) as T
    }

    @LayoutRes
    abstract fun getLayoutId() : Int


    open fun onSelectedFile(filePath: String?){}

    companion object {
        private val TAG = BaseActivity::class.java.simpleName
        private const val REQUEST_CODE = 1234
        private val permissions = arrayOf(
            Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.CAMERA,
            Manifest.permission.RECORD_AUDIO
        )
    }
}