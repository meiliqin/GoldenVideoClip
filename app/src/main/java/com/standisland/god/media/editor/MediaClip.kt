/*
 * Copyright (C) 2019 Trinity. All rights reserved.
 * Copyright (C) 2019 Wang LianJie <wlanjie888@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

package com.standisland.god.media.editor

open class MediaClip {


  public var mimeType = ""

  public var path = ""
  public var timeRange: TimeRange= TimeRange(0,0)
  public var rotate=0
  public var speed=1.0f
//  public var duration=0L
  constructor(path: String) {
    this.path = path
  }

  constructor(path: String, timeRange: TimeRange) {
    this.path = path
    this.timeRange = timeRange
  }
  constructor(path: String, timeRange: TimeRange,mimeType:String) {
    this.path = path
    this.timeRange = timeRange
    this.mimeType=mimeType
//    this.rotate=90
//    this.speed=1.5f
  }

  fun setStartTime(startTime:Long){
    timeRange.start=startTime
  }
  fun setEndTime(endTime:Long){
    timeRange.end=endTime
  }
  fun setTime(startTime:Long,endTime:Long){
    timeRange.start=startTime
    timeRange.end=endTime

  }



}