package com.standisland.god.demo.common.entity

import java.io.Serializable

class MediaItem(
    val path: String,
    val type: String,
    val width: Int,
    val height: Int
) : Serializable {
  var duration: Int = 0     //每张图片 3 秒
}