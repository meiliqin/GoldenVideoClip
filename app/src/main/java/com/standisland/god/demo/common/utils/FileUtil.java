package com.standisland.god.demo.common.utils;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Bitmap.CompressFormat;
import android.os.Build;
import android.os.Environment;
import android.os.StatFs;
import android.text.TextUtils;
import android.util.Log;
import android.webkit.MimeTypeMap;

import java.io.BufferedWriter;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.Locale;



/**
 * 文件工具类
 */
public class FileUtil {
    
    public static final String TAG = "FileUtil";

    /**
     * SD卡是否正常挂载
     *
     * @return
     */
    public static boolean externalStorageAvailable() {
        return Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED);
    }

    /**
     * 获取SD卡可用的存储空间
     *
     * @return
     */
    public static long getAvailableExternalStorageSize() {
        String state = Environment.getExternalStorageState();
        if (Environment.MEDIA_MOUNTED.equals(state)) {
            File sdcardDir = Environment.getExternalStorageDirectory();
            StatFs sf = new StatFs(sdcardDir.getPath());
            long blockSize;
            long availCount;
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2) {
                blockSize = sf.getBlockSizeLong();
                availCount = sf.getAvailableBlocksLong();
            } else {
                blockSize = sf.getBlockSize();
                availCount = sf.getAvailableBlocks();
            }
            return availCount * blockSize;
        }
        return -1;
    }

    /**
     * 判断文件是否存在
     *
     * @param filePath
     * @return
     */
    public static boolean isFileExists(String filePath) {
        try {
            File f = new File(filePath);
            if (!f.exists()) {
                return false;
            }

        } catch (Exception e) {
            return false;
        }
        return true;
    }

    /**
     * 删除一个文件或者目录
     *
     * @param file file to delete
     * @return true when it is file and it is deleted successfully
     */
    public static boolean deleteFileOrDictionary(File file) {
        if (null == file) {
            return false;
        }
        if (file.isDirectory()) {
            deleteDictionary(file, true);
        } else if (file.isFile()) {
            deleteFile(file);
        }
        return false;
    }

    public static void deleteFile(String path) {
        try {
            if (!TextUtils.isEmpty(path) && FileUtil.isFileExists(path)) {
                FileUtil.deleteFile(new File(path));
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * 删除一个文件
     *
     * @param file file to delete
     * @return true when it is file and it is deleted successfully
     */
    public static boolean deleteFile(File file) {
        if (file.isFile()) {
            try {
                return file.delete();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        return false;
    }

    /**
     * 删除一个目录
     *
     * @param dir       dictionary to delete
     * @param recursion false means that just delete the dictionary when empty
     * @return true when it is dictionary and it is deleted successfully
     */
    public static boolean deleteDictionary(File dir, boolean recursion) {

        if (!dir.isDirectory()) {
            return false;
        }

        if (!recursion) {
            try {
                return dir.delete();
            } catch (Exception e) {
                return false;
            }
        }

        boolean deleteFiles = true;
        LinkedList<File> dirStack = new LinkedList<File>();
        HashSet<String> trackedDirs = new HashSet<String>();
        File tempDir = null;
        dirStack.push(dir);
        do {
            tempDir = dirStack.peek();
            final String tempDirPath = tempDir.getAbsolutePath();
            if (trackedDirs.contains(tempDirPath)) {
                dirStack.pop();
                trackedDirs.remove(tempDirPath);
                tempDir.delete();
            } else {
                trackedDirs.add(tempDirPath);
                deleteFiles = deleteDictionaryInternal(dirStack, tempDir, null);
            }
            if (!deleteFiles) {
                break;
            }
        } while (!dirStack.isEmpty());
        return deleteFiles && dir.exists();
    }

    /**
     * 删除字典
     *
     * @param dir       dictionary to delete
     * @param recursion false means that just delete the dictionary when empty
     * @return true when it is dictionary and it is deleted successfully
     */
    public static void deleteDictionary(File dir, boolean recursion, HashSet<String> filter) {

        if (!dir.isDirectory()) {
            return;
        }

        if (!recursion) {
            try {
                dir.delete();
            } catch (Exception e) {
                e.printStackTrace();
            }
            return;
        }

        boolean deleteFiles = true;
        LinkedList<File> dirStack = new LinkedList<File>();
        HashSet<String> trackedDirs = new HashSet<String>();
        File tempDir = null;
        dirStack.push(dir);
        do {
            tempDir = dirStack.peek();
            final String tempDirPath = tempDir.getAbsolutePath();
            if (trackedDirs.contains(tempDirPath)) {
                // try delete the empty dictionary
                dirStack.pop();
                trackedDirs.remove(tempDirPath);
                tempDir.delete();
            } else {
                trackedDirs.add(tempDirPath);
                deleteFiles = deleteDictionaryInternal(dirStack, tempDir, filter);
            }
            if (!deleteFiles) {
                break;
            }
        } while (!dirStack.isEmpty());
    }

    /**
     * 过滤删除字典
     *
     * @param dirStack
     * @param dir
     * @param filter
     * @return
     */
    private final static boolean deleteDictionaryInternal(LinkedList<File> dirStack, File dir, HashSet<String> filter) {
        boolean deleteFiles = true;
        File[] filesOrDirs = dir.listFiles();
        if (filesOrDirs != null) {
            for (File filesOrDir : filesOrDirs) {
                if (filter != null && filter.contains(filesOrDir.getAbsolutePath())) {
                    continue;
                }

                if (filesOrDir.isDirectory()) {
                    dirStack.push(filesOrDir);
                } else {
                    try {
                        filesOrDir.delete();
                    } catch (Exception e) {
                        deleteFiles = false;
                        break;
                    }
                }
            }
        }
        return deleteFiles;
    }

    /**
     * 复制文件
     *
     * @param srcFile  The file to copy
     * @param destFile The file to save
     * @return Return false if fail
     */
    public static boolean copyFile(File srcFile, File destFile) {
        boolean result = false;
        try {
            InputStream in = new FileInputStream(srcFile);
            try {
                result = copyToFile(in, destFile);
            } finally {
                in.close();
            }
        } catch (IOException e) {
            e.printStackTrace();
            result = false;
        }
        return result;
    }

    /**
     * 从流中复制一个文件
     *
     * @param inputStream The inputstream to write the #destFile.
     * @param destFile    The file to save.
     * @return Return false if failed
     */
    public static boolean copyToFile(InputStream inputStream, File destFile) {
        try {
            if (destFile.exists()) {
                destFile.delete();
            }
            FileOutputStream out = new FileOutputStream(destFile);
            try {
                byte[] buffer = new byte[4096];
                int bytesRead;
                while ((bytesRead = inputStream.read(buffer)) >= 0) {
                    out.write(buffer, 0, bytesRead);
                }
            } finally {
                out.flush();
                try {
                    out.getFD().sync();
                } catch (IOException e) {
                }
                out.close();
            }
            return true;
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        }
    }

    /**
     * 保存一个bitmap到文件里
     *
     * @param bitmap
     * @param destFile
     * @return
     */
    public static boolean copyToFile(Bitmap bitmap, File destFile, CompressFormat format, int quality) {
        if (bitmap == null || bitmap.isRecycled()) {
            return false;
        }
        try {
            if (destFile.exists()) {
                destFile.delete();
            }
            File parent = new File(destFile.getParent());
            if (!parent.exists()) {
                parent.mkdirs();
            }
            FileOutputStream out = new FileOutputStream(destFile);
            try {
                bitmap.compress(format, quality, out);
            } finally {
                out.flush();
                try {
                    out.getFD().sync();
                } catch (IOException e) {
                }
                out.close();
            }
            return true;
        } catch (IOException e) {
            Log.e(TAG,"Bitmap", e);
            return false;
        }
    }

    /**
     * 保存一个bitmap到文件里
     *
     * @param bitmap
     * @param destFile
     * @return
     */
    public static boolean copyToFile(Bitmap bitmap, File destFile, int quality) {
        return copyToFile(bitmap, destFile, CompressFormat.PNG, quality);
    }

    /**
     * 保存一个bitmap到文件里
     *
     * @param bitmap
     * @param destFile
     * @return
     */
    public static boolean copyToFile(Bitmap bitmap, File destFile) {
        return copyToFile(bitmap, destFile, 75);
    }

    /**
     * 写入文件
     *
     * @param str
     * @param destFile
     * @return
     */
    public static boolean copyToFile(String str, File destFile) {

        if (TextUtils.isEmpty(str)) {
            return false;
        }

        File parent = destFile.getParentFile();
        if (!parent.exists()) {
            parent.mkdirs();
        }
        boolean written = false;
        try {
            BufferedWriter fw = new BufferedWriter(new FileWriter(destFile.getPath(), true));
            fw.write(str);
            fw.flush();
            fw.close();

            written = true;
        } catch (Exception e) {
            e.printStackTrace();
        }

        return written;
    }

    /**
     * 写入文本
     *
     * @param bytes
     * @param destFile
     * @return
     */
    public static boolean copyToFile(byte[] bytes, File destFile) {

        if (bytes == null) {
            return false;
        }

        try {
            if (destFile.exists()) {
                destFile.delete();
            }
            destFile.createNewFile();
            FileOutputStream out = new FileOutputStream(destFile);
            try {
                out.write(bytes);
            } finally {
                out.flush();
                try {
                    out.getFD().sync();
                } catch (IOException e) {
                }
                out.close();
            }
            return true;
        } catch (IOException e) {
            Log.e(TAG,"byte[]", e);
            return false;
        }
    }

    /**
     * 获取文本内容
     *
     * @param file
     * @return
     */
    public static String getFileContent(File file) {

        String content = null;
        if (file == null || !file.isFile()) {
            return content;
        }

        final long maxSize = 5 * 1024 * 1024; // 5M
        if (file.length() > maxSize) {
            return content;
        }

        try {
            ByteArrayOutputStream contentStream = new ByteArrayOutputStream();
            InputStream inputStream = new FileInputStream(file);
            try {
                byte[] buffer = new byte[4096];
                int bytesRead;
                while ((bytesRead = inputStream.read(buffer)) >= 0) {
                    contentStream.write(buffer, 0, bytesRead);
                }
                content = new String(contentStream.toByteArray());
            } finally {
                inputStream.close();
                contentStream.close();
            }
        } catch (Exception e) {
            Log.e(TAG,"", e);
            content = null;
        }

        return content;
    }

    /**
     * 读取raw资源文本
     *
     * @param context
     * @param res
     * @return
     */
    public static String getFileContentFromRaw(Context context, int res) {

        String content = null;
        if (res <= 0) {
            return content;
        }
        try {
            ByteArrayOutputStream contentStream = new ByteArrayOutputStream();
            InputStream inputStream = context.getResources().openRawResource(res);
            try {
                byte[] buffer = new byte[4096];
                int bytesRead;
                while ((bytesRead = inputStream.read(buffer)) >= 0) {
                    contentStream.write(buffer, 0, bytesRead);
                }
                content = new String(contentStream.toByteArray());
            } finally {
                inputStream.close();
                contentStream.close();
            }
        } catch (Exception e) {
            Log.e(TAG,"", e);
            content = null;
        }

        return content;
    }

    /**
     * 创建一个目录
     *
     * @param path
     */
    public static void mkdirs(String path) {
        File dir = new File(path);
        if (!dir.exists()) {
            dir.mkdirs();
        }
    }

    /**
     * 安全关闭输入流
     *
     * @param stream
     */
    public static void saveClose(InputStream stream) {
        if (stream != null) {
            try {
                stream.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    /**
     * 安全关闭输出流
     *
     * @param stream
     */
    public static void saveClose(OutputStream stream) {
        if (stream != null) {
            try {
                stream.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    /**
     * 获取文件夹大小
     *
     * @param file File实例
     * @return long 单位为M
     * @throws Exception
     */
    public static long getFolderSize(File file) {
        long size = 0;
        try {
            File[] fileList = file.listFiles();
            if (fileList != null) {
                for (int i = 0; i < fileList.length; i++) {
                    if (fileList[i].isDirectory()) {
                        size = size + getFolderSize(fileList[i]);
                    } else {
                        size = size + fileList[i].length();
                    }
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        return size;
    }


    //获取文件夹大小,返回大小为Bit
    public static long getFolderSizeBit(File file) {
        if(file==null){
            return 0;
        }
        long size = 0;
        File[] fileList = file.listFiles();
        if (fileList != null) {
            for (File aFileList : fileList) {
                if (aFileList.isDirectory()) {
                    size = size + getFolderSizeBit(aFileList);
                } else {
                    size = size + aFileList.length();
                }
            }
        }
        return size;
    }

    //获取文件夹大小,返回大小为带一位小数的MB
    public static double getFolderSizeMB(File file) {
        double ret = (double)(getFolderSizeBit(file) / 1048576);
        return (double)(Math.round(ret*10)) / 10;
    }

    public static byte[] fileToByte(String filePath) throws IOException {
        File file = new File(filePath);
        long fileSize = file.length();
        if (fileSize > Integer.MAX_VALUE) {
            System.out.println("file too big...");
            return null;
        }
        FileInputStream fi = new FileInputStream(file);
        byte[] buffer = new byte[(int) fileSize];
        int offset = 0;
        int numRead = 0;
        while (offset < buffer.length
                && (numRead = fi.read(buffer, offset, buffer.length - offset)) >= 0) {
            offset += numRead;
        }
        // 确保所有数据均被读取
        if (offset != buffer.length) {
            throw new IOException("Could not completely read file "
                    + file.getName());
        }
        fi.close();
        return buffer;
    }



    // 获取文件扩展名
    public static String getExtensionName(String filename) {
        if ((filename != null) && (filename.length() > 0)) {
            int dot = filename.lastIndexOf('.');
            if ((dot > -1) && (dot < (filename.length() - 1))) {
                return filename.substring(dot + 1);
            }
        }
        return "";
    }



    public enum SizeUnit {
        Byte,
        KB,
        MB,
        GB,
        TB,
        Auto,
    }



    public static boolean hasExtentsion(String filename) {
        int dot = filename.lastIndexOf('.');
        if ((dot > -1) && (dot < (filename.length() - 1))) {
            return true;
        } else {
            return false;
        }
    }

    // 获取文件名
    public static String getFileNameFromPath(String filepath) {
        if ((filepath != null) && (filepath.length() > 0)) {
            int sep = filepath.lastIndexOf('/');
            if ((sep > -1) && (sep < filepath.length() - 1)) {
                return filepath.substring(sep + 1);
            }
        }
        return filepath;
    }

    // 获取不带扩展名的文件名
    public static String getFileNameNoEx(String filename) {
        if ((filename != null) && (filename.length() > 0)) {
            int dot = filename.lastIndexOf('.');
            if ((dot > -1) && (dot < (filename.length()))) {
                return filename.substring(0, dot);
            }
        }
        return filename;
    }
}

