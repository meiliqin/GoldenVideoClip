/*
 * Copyright (C) 2020 Trinity. All rights reserved.
 * Copyright (C) 2020 Wang LianJie <wlanjie888@gmail.com>
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
 */

package com.standisland.god.demo.feature.sticker

import android.graphics.Typeface

class TypefaceConfig {
  var typeface: Typeface? = null
  var fakeBold = false

  constructor(typeface: Typeface?, fake_bold: Boolean) {
    this.typeface = typeface
    this.fakeBold = fake_bold
  }

  constructor(typeface: Typeface?) {
    this.typeface = typeface
  }

  constructor()
}