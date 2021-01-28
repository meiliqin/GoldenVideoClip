package com.standisland.god.demo.feature.effect.listener

import com.standisland.god.demo.feature.effect.bean.Effect


interface OnEffectTouchListener {

  fun onEffectTouchEvent(event: Int, effect: Effect)
}