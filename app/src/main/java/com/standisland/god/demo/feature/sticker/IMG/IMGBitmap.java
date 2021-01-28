package com.standisland.god.demo.feature.sticker.IMG;

/**
 * author : linzheng
 * e-mail : linzheng@corp.standisland.com
 * time   : 2020/7/6
 * desc   : 贴纸信息´
 * version: 1.0
 */
public class IMGBitmap {

    private String filePath;
    private int tabIndex, stickerIndex;
    private String tabName, stickerName;
    private String tabId,stickerId;

    public IMGBitmap(String filePath, int tabIndex, int stickerIndex, String tabName, String stickerName, String tabId, String stickerId) {
        this.filePath = filePath;
        this.tabIndex = tabIndex;
        this.stickerIndex = stickerIndex;
        this.tabName = tabName;
        this.stickerName = stickerName;
        this.tabId = tabId;
        this.stickerId = stickerId;
    }

    public String getTabId() {
        return tabId;
    }

    public String getStickerId() {
        return stickerId;
    }

    public IMGBitmap(String filePath) {
        this.filePath = filePath;
    }

    public String getFilePath() {
        return filePath;
    }

    public int getStickerIndex() {
        return stickerIndex;
    }

    public int getTabIndex() {
        return tabIndex;
    }

    public String getStickerName() {
        return stickerName;
    }

    public String getTabName() {
        return tabName;
    }

    public void setFilePath(String filePath) {
        this.filePath = filePath;
    }

}
