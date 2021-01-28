package com.standisland.god.demo

import android.content.Context
import com.standisland.god.media.editor.TrinityVideoEditor
import com.standisland.god.media.editor.TrinityVideoExport
import com.standisland.god.media.editor.VideoEditor
import com.standisland.god.media.editor.VideoExport

object TrinityCore {

  /**
   * 创建视频编辑实例
   * @param context Android上下文
   * @return 返回创建的视频编辑实例
   */
  fun createEditor(context: Context): TrinityVideoEditor {
    val path = context.externalCacheDir?.absolutePath
    videoEditor = path?.let { VideoEditor(context, it) }
    return videoEditor as VideoEditor
  }

  fun createExport(context: Context): VideoExport {
    val path = context.externalCacheDir?.absolutePath
    return TrinityVideoExport(context,path!!)
  }

  private var videoEditor: TrinityVideoEditor? = null

  /**
   * 获取单例中的videoEditor对象
   */
  fun getEditor(): TrinityVideoEditor? {
    return videoEditor
  }


}