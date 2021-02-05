
基于 [trinity](https://github.com/wlanjie/trinity/issues) 进行二次开发，目前除trinity已有的剪辑功能外，做了以下工作：

* 抽帧预览
* 逐帧seek，播放进度拖拽
* 添加文字贴纸功能 sticker.cc
* 文字的移动，旋转，缩放，调时间轴，删除和添加 video_editor.cc image_process.cc
* 支持素材的裁剪，旋转，调速
* 做音视频同步 player.cc
* 做音频变速
* 做素材转场 transition.cc

### 项目结构

* extra 共有头文件及so资源包
* app 为demo迭代
* core cpp/java库核心代码，绝大部分是c++代码
      

### 需求
整体需求大致分为 3 部分
1. 包含解码相关但不限于，视频源加载/信息获取/帧预览/进度条管理
2. 包含编码相关但不限于，素材管理，以json组织传输；视频合成
3. feature 功能的开发，面向时间线叠加渲染
    1. 调整素材
   
    2. 文字贴纸
 
    3. 特效
    
    4. 美颜滤镜
    
    5. 音乐出来
    
    
    
### DEMO 目录
1. common 存放共有逻辑，比如 json 共有结构，共有 view 等
2. input 视频加载，开放帧获取，信息获取，进度条等，对应需求 "1"
3. output 视频输出，对应需求 "2"
4. feature，各功能点代码严格分开
    1. effect 特效所有代码
    2. filter 滤镜
    3. music 音乐处理
    4. subtitle 字幕
    5. .... 自行添加 





### 部分源码解读：

软硬解在jni层的不同处理？
1. 可从上下文判断软硬模式
2. 软解视频使用 YuvRender 渲染器，
3. 硬解则基于 OpenGl封装 FrameBuffer从缓存管道中读取
4. 上述2/3 获取到纹理之后交给 ImageProcess 处理器处理，改处理器会读取所有 feature 特征，针对每个特征进行画帧
5. 最后在把纹理绘制到屏幕上。


Editor 扮演什么角色？
1. 包含播放功能，同时用户管理feature提供的资源，把每一个媒体当作clip进行加载
2. 每一个 feature 通过业务封装成 Json 格式传递给 jni 层，对应Jni中 editor_resource_
3. editor_resource_ 把每一个配置 json 写入保存到文件中


Clip 扮演什么角色 ？ 
1. 我们支持多个媒体资源拼接插入，每一个媒体资源（图片/视频）对应 java/c++ 层的同名 MediaClip
2. 目前仅仅支持  JPEG 格式/ PNG 格式/mp4 格式 


JNI层使用什么手段通讯？
1. 类 Android 层的 Handler 机制，使用 Message+MessageQueue 队列实现，可查看 message 包
2. MessageNode为节点信息，next指针支持链表接口，Message 为真正消息，类Android Message
3. handler#postMessage 为发送信息，handler#handleMessage 为处理信息
4. messageQueue 包含进队出队中止等实现

关于缓存队列的管理
Frame 帧数据
FramePool 帧缓存池，用于缓存 Frame数据，记录个数
1. PacketPool *packet_pool;
2. FramePool *audio_frame_pool;保存所有已解码音频帧
3. FramePool *video_frame_pool; 保存所有已解码视频帧
FrameQueue，帧队列
1. FrameQueue *audio_frame_queue; 存放大于当前播放位置的已解码音频帧
2. FrameQueue *video_frame_queue; 存放大于当前播放位置的已解码视频帧
PacketQueue，压缩包队列
1. PacketQueue *video_packet_queue;
2. PacketQueue* audio_packet_queue;




关于解码线程
音频解码 audio_decode_thread
视频硬解 video_decode_hw_thread
视频软解 video_decode_sw_thread


播放流程 av_play_play
1. avformat_alloc_context 获取 AVFormatContext 上下文
2. av_find_best_stream 获取视频 steam 流
3. av_find_best_stream 获取音频stream 流
4. 如果存在音频，则 avcodec_open2 初始化音频解码器 
5. 如果存在视频，则获取 AVCodecParameters 判断解码器类型，初始化解码器（软/硬）
6. 设置音频视频的 pack 队列信息，包括 buffer 时间，queue 时长，空队列回调，满队列回调
7. 启动读取 steam 流线程 read_thread
8. 启动视频/音频解码线程 video_decode_sw_thread/video_decode_hw_thread/audio_decode_thread
9. 设置播放状态


音频解码做了什么 audio_decode_thread
1. 获取音频 filter 上下文
2. 从 audio_frame_pool 获取音频帧
3. 当不是中止状态时进入循环
    1. 如果是暂停态，则休眠 100 ms
    2. 接收音频解码器返回的帧数据，保存在音频帧中
    3. 如果成功获取到音频解码帧，如果小于 seek_to 则在 audio_frame_pool 丢掉，否则则进入 audio_frame_queue 队列。跳过
    4. 如果读取失败或结束了，则从 audio_packet_queue 获取数据。
        1. 如果 audio_packet_queue 没有数据 且 文件已经到头了，则清除上下文，跳过
        2. 如果有数据且等于 flush_packet，清空 audio_frame_queue，刷新buffer，跳过
        3. 把 packet 发送给解码器解码，释放 packet

视频软解做了什么 video_decode_sw_thread
1. 获取播放上下文
2. 从 video_frame_pool 获取视频帧
3. 当不是中止状态时进入循环
    1. 如果只播放音频，则把视频 video_packet_queue 都丢掉，保证音视频同步，跳过
    2. 接收视频解码器返回的帧数据，保存在视频帧中
    3. 如果成功获取到视频解码帧，则进 video_frame_queue 队列，休眠 2000 微妙， video_frame_pool index 往后移动一位
    4. 如果读取失败或结束了，则从 video_packet_queue 获取数据。
        1. 如果 video_packet_queue 没有数据 且 文件已经到头了，则清除上下文，跳过
        2. 如果有数据且等于 flush_packet，清空 video_packet_queue，刷新buffer，跳过
        3. 把 packet 发送给解码器解码，释放 packet

视频硬解做了什么 video_decode_hw_thread
1. 获取播放上下文
2. 从 video_frame_pool 获取视频帧
3. 当不是中止状态时进入循环
    1. 如果只播放音频，则把视频 video_packet_queue 都丢掉，保证音视频同步，跳过
    2. 接收视频解码器返回的帧数据，保存在视频帧中
    3. 如果已经读取到尾部，则跳过
    4. 如果已经读区到数据
        1. 如果支持精准 seek模式，当前帧时间小于 seek_to 时间，则释放 buffer，否则 video_frame_queue 进队之后只显示一针，状态调整为 SEEK_COMPLETE
        2. 如果不是精准 seek模式，当 seek_to 大于当前帧时间是，每隔 100 ms 显示一帧，video_frame_queue 进队之后，状态调整为 SEEK_COMPLETE
    5. 如果成功获取到视频解码帧，则进 video_frame_queue 队列，休眠 2000 微妙， video_frame_pool index 往后移动一位
    6. 如果读取失败或结束了，则从 video_packet_queue 获取数据。
        1. 如果 video_packet_queue 没有数据 且 文件已经到头了，则清除上下文，跳过
        2. 如果有数据且是 seek 场景，清空 video_packet_queue，刷新buffer，跳过
        3. 把 packet 发送给解码器解码，释放 packet
    7. 获取 video_packet_queue 数据
        1. 如果文件已经到尾部了，则设置 end_of_stream = true；否则则休眠 10 ms，跳过
        2. 如果有数据且等于 flush_packet，则跳过
        3. 把 packet 发送给解码器解码，释放 packet


异步读文件数据 read_stream_thread
1. 获取播放上下文
2. 当不是中止状态时进入循环
    1. 如果当前 seeking == 1，清除音视频队列信息及缓存，把解码器 seek ，设置 seek == 2，利用 av_seek_frame seek 到 seek_to 位置
    2. 如果 audio_packet_queue + video_packet_queue 大小大于 buffers ，则跳过
    3. 从 packet_pool 获取一个 packet 数据
    4. 从 packet 中读取 frame 数据
        1. 如果成功读取，包含视频流，则放入 video_packet_queue ，包含音频流，则 放入 audio_packet_queue
        2. 如果数据流不可用，则清空 packet_pool
        3. 如果文件结束了，则设置 eof = true

音视频同步算法逻辑

```
/**
 * 音视频同步逻辑
 * diff 为当前队列最后面的帧的pts减去主时钟
 * 根据 RFC-1359 国际规范，音频和视频的时间戳差值在 -100ms ～ +25ms 中间用户无法感知
 * 所以根据这个差值来实现音视频同步。
 * min 为同步阈值
 * max为异常阈值
 */
int64_t diff = videoPts - masterClock;
int64_t min = 25000;
int64_t max = 1000000;

if(diff > -min && diff < min){
    startAudioRender();
    CreateRenderFrameBuffer();
    RenderFrameBuffer();
    clock_set(av_play_context_->video_clock, videoPts);
    LOGI("DrawVideoFrame, 正常播放[-25ms,25ms]");
}else if(diff >= min && diff < max){
    LOGI("DrawVideoFrame, 视频帧提前[ %lld ms ],通过 sleep [ %lld ms]来等待主时钟",diff / 1000,(diff - min) / 1000);
    usleep(static_cast<useconds_t>(diff - min));
    startAudioRender();
    CreateRenderFrameBuffer();
    RenderFrameBuffer();
    clock_set(av_play_context_->video_clock, videoPts - (diff - min));
}else{
    if(diff > 0){
        LOGI("DrawVideoFrame, 视频帧提前超过[ 100 ms ],丢帧");
    }else{
        LOGI("DrawVideoFrame, 视频帧滞后超过[ 25 ms ],丢帧");
    }
    ReleaseVideoFrame();
}
```
  