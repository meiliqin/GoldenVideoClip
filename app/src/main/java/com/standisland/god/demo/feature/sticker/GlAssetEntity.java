package com.standisland.god.demo.feature.sticker;


import com.google.gson.reflect.TypeToken;

import java.util.List;
import java.util.concurrent.ConcurrentMap;

/**
 * author : linzheng
 * e-mail : linzheng@corp.standisland.com
 * time   : 2020/6/10
 * desc   :
 * version: 1.0
 */
public class GlAssetEntity {

    public static TypeToken<ConcurrentMap<String, GlAssetEntity>> MAP_TOKEN = new TypeToken<ConcurrentMap<String, GlAssetEntity>>() {
    };
    public static TypeToken<List<GlAssetEntity>> LIST_TOKEN = new TypeToken<List<GlAssetEntity>>() {
    };


    private int priority;
    private String fileName;
    private String url;
    private long fileSize;
    private String image;
    private String ext;


    public int getPriority() {
        return priority;
    }

    public void setPriority(int priority) {
        this.priority = priority;
    }

    public String getFileName() {
        return fileName;
    }

    public void setFileName(String fileName) {
        this.fileName = fileName;
    }

    public String getUrl() {
        return url;
    }

    public void setUrl(String url) {
        this.url = url;
    }

    public long getFileSize() {
        return fileSize;
    }

    public void setFileSize(long fileSize) {
        this.fileSize = fileSize;
    }

    public String getImage() {
        return image;
    }

    public void setImage(String image) {
        this.image = image;
    }

    public String getExt() {
        return ext;
    }

    public void setExt(String ext) {
        this.ext = ext;
    }
}
