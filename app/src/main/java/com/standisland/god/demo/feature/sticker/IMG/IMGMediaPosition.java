package com.standisland.god.demo.feature.sticker.IMG;

/**
 * author : linzheng
 * e-mail : linzheng@corp.standisland.com
 * time   : 2020/7/6
 * desc   :
 * version: 1.0
 */
public class IMGMediaPosition {

    private long start;
    private long end;


    public IMGMediaPosition(long start, long end) {
        this.start = start;
        this.end = end;
    }

    public long getStart() {
        return start;
    }

    public void setStart(long start) {
        this.start = start;
    }

    public long getEnd() {
        return end;
    }

    public void setEnd(long end) {
        this.end = end;
    }
}
