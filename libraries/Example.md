# AMB82-MINI 库模块示例功能解读

## 1. Analog（模拟信号处理）
- **PWM_BuzzerPlayMelody**：使用PWM控制蜂鸣器播放旋律
- **PWM_ServoControl**：使用PWM控制舵机

## 2. BLE（蓝牙低功耗）
- **BLEBatteryClient**：作为客户端读取电池电量信息
- **BLEBatteryService**：作为服务端提供电池电量服务
- **BLEBeacon**：蓝牙信标功能
- **BLEHIDGamepad**：蓝牙游戏手柄HID设备
- **BLEHIDKeyboard**：蓝牙键盘HID设备
- **BLEHIDMouse**：蓝牙鼠标HID设备
- **BLEScan**：蓝牙设备扫描
- **BLEUartClient**：蓝牙UART客户端
- **BLEUartService**：蓝牙UART服务端
- **BLEV7RC_CAR_VIDEO**：V7RC遥控车视频传输
- **BLEWifiConfig**：通过蓝牙配置WiFi
- **DHT_over_BLEUart**：通过蓝牙UART传输DHT传感器数据
- **DoorUnlockOverBLEUart**：通过蓝牙UART控制门锁
- **PWM_over_BLEUart**：通过蓝牙UART控制PWM输出

## 3. Debugging（调试）
- **ExampleDebug**：调试功能示例，包含自定义调试配置

## 4. FileSystem（文件系统）
- **CreateFolder**：创建文件夹
- **FileReadWrite**：文件读写操作
- **GetFileAttribute**：获取文件属性
- **GetFreeAndUseSpace**：获取存储空间使用情况
- **LastModifiedTime**：获取文件最后修改时间
- **ListRootFiles**：列出根目录文件
- **ReadHTMLFile**：读取HTML文件

## 5. FlashMemory（闪存）
- **ReadWriteStream**：流式读写闪存
- **ReadWriteWord**：按字读写闪存

## 6. GPIO（通用输入输出）
- **DHT_Tester**：DHT温湿度传感器测试
- **Interrupt_Ctrl_LED**：中断控制LED
- **MeasureDistance_HCSR04_Ultrasonic**：使用HC-SR04超声波传感器测量距离

## 7. GTimer（通用定时器）
- **TimerOneshot**：单次定时器
- **TimerPeriodical**：周期性定时器

## 8. Http（HTTP通信）
- **Capture_Image_HTTP_Post_Image_Llava_Server**：捕获图像并通过HTTP POST到Llava服务器
- **Capture_Image_HTTP_Post_Image_Llava_Server_FASTAPI**：使用FASTAPI捕获图像并POST到Llava服务器
- **HTTP_IFTTT_Post**：HTTP POST到IFTTT服务
- **HTTP_Post_MP4_Whisper_Server**：HTTP POST MP4到Whisper服务器
- **HTTP_Post_MP4_Whisper_Server_FASTAPI**：使用FASTAPI HTTP POST MP4到Whisper服务器
- **Http_fs_mp4**：从文件系统提供MP4的HTTP服务
- **RecordMP4_HTTP_Post_Whisper_Server**：录制MP4并POST到Whisper服务器
- **RecordMP4_HTTP_Post_Whisper_Server_FASTAPI**：使用FASTAPI录制MP4并POST到Whisper服务器
- **RetrieveHttpWebs**：获取网页内容

## 9. MQTTClient（MQTT客户端）
- **MQTT_Auth**：带认证的MQTT连接
- **MQTT_Basic**：基本MQTT功能
- **MQTT_GenAIVision**：MQTT传输生成式AI视觉数据
- **MQTT_Image**：MQTT传输图像
- **MQTT_Publish_In_Callback**：在回调中发布MQTT消息
- **MQTT_TLS**：使用TLS的MQTT连接

## 10. Multimedia（多媒体）
### Audio（音频）
- **AudioEffect**：音频效果处理
- **AudioVolumeAdjust**：音量调节
- **EchoCancellation**：回声消除
- **LoopbackTest**：音频回环测试
- **RTPAudioStream**：RTP音频流
- **RTSPAudioStream**：RTSP音频流

### CaptureJPEG（JPEG图像捕获）
- **HTTPDisplayJPEG**：通过HTTP显示JPEG图像
- **HTTPDisplayJPEGContinuous**：连续HTTP显示JPEG图像
- **HTTPDisplayJPEGContinuousMultitask**：多任务连续HTTP显示JPEG图像
- **SDCardSaveJPEG**：保存JPEG到SD卡
- **SDCardSaveJPEGTimelapse**：JPEG延时摄影保存到SD卡
- **SDCardSaveJPEGTimelapseWithNTPClient**：带NTP客户端的JPEG延时摄影保存到SD卡
- **SDCardSaveJPEGWithNTPClient**：带NTP客户端的JPEG保存到SD卡

### DemuxerRTSP（RTSP解复用）
- **DemuxerRTSP**：RTSP流解复用

### ISPControl（图像信号处理器控制）
- **Exposure**：曝光控制
- **ImageTuning**：图像调优
- **Mode**：模式设置
- **WhiteBalance**：白平衡控制
- **WideDynamicRange**：宽动态范围

### MotionDetection（运动检测）
- **CallbackPostProcessing**：回调后处理运动检测
- **LoopPostProcessing**：循环后处理运动检测
- **MaskingMP4Recording**：掩码MP4录制
- **MotionDetectGoogleLineNotify**：运动检测Google通知

### RecordMP4（MP4录制）
- **AudioOnly**：仅录制音频
- **AudioOnlyWithNTPClient**：带NTP客户端的仅音频录制
- **DoubleVideoWithAudio**：双视频流带音频录制
- **DoubleVideoWithAudioAndNTPClient**：带NTP客户端的双视频流带音频录制
- **SingleVideoWithAudio**：单视频流带音频录制
- **SingleVideoWithAudioAndNTPClient**：带NTP客户端的单视频流带音频录制
- **SingleVideoWithAudio_MP3Prompts**：带MP3提示的单视频流带音频录制
- **VideoOnly**：仅录制视频
- **VideoOnlyWithNTPClient**：带NTP客户端的仅视频录制

### SDCardPlayMP3（SD卡播放MP3）
- **SDCardPlayMP3**：从SD卡播放MP3

### StreamRTSP（RTSP流）
- **DoubleVideo**：双视频流RTSP
- **DoubleVideoWithAudio**：双视频流带音频RTSP
- **SingleVideoWithAudio**：单视频流带音频RTSP
- **V7RC**：V7RC遥控RTSP流
- **VideoOnly**：仅视频RTSP流

### VideoOSDImage（视频OSD图像）
- **VideoOSDImage**：视频叠加OSD图像

## 11. NTPClient（网络时间协议客户端）
- **Advanced**：高级NTP客户端功能
- **Basic**：基本NTP客户端功能

## 12. NeuralNetwork（神经网络）
### AudioClassification（音频分类）
- **AudioClassification**：音频分类识别

### FaceRecognition（人脸识别）
- **CaptureJPEGFaceRecognition**：JPEG图像人脸识别
- **DoorUnlockWithFaceRecognition**：人脸识别门锁控制
- **RTSPFaceDetection**：RTSP流人脸检测
- **RTSPFaceRecognition**：RTSP流人脸识别

### HandGestureDetection（手势识别）
- **HandGestureDetection**：手势识别

### MultimediaAI（多媒体AI）
- **GenAISpeech_Gemini**：使用Gemini的生成式AI语音
- **GenAISpeech_Gemini_LEDControl**：使用Gemini的生成式AI语音控制LED
- **GenAISpeech_Whisper**：使用Whisper的生成式AI语音
- **GenAIVideo**：生成式AI视频
- **GenAIVision**：生成式AI视觉
- **MediaPipe_Holistic_Landmarker**：MediaPipe全身标记点检测
- **TextToSpeech**：文本转语音
- **TextToSpeechNTP**：带NTP的文本转语音

### ObjectDetection（物体检测）
- **ObjectDetectionCallback**：回调式物体检测
- **ObjectDetectionImage**：图像物体检测
- **ObjectDetectionLoop**：循环式物体检测
- **ObjectDetectionSaveSDCard**：物体检测结果保存到SD卡（JPEG和MP4格式）
- **RTSPImageClassification**：RTSP流图像分类
- **UVCDObjectDetectionLoop**：USB视频设备物体检测循环

## 13. OTA（空中升级）
- **OTA**：基本OTA功能
- **SDCardOTA**：从SD卡进行OTA升级

## 14. PowerMode（电源模式）
- **DeepSleepMode**：深度睡眠模式
- **RetentionDeepSleepMode**：保留数据的深度睡眠模式
- **RetentionStandbyMode**：保留数据的待机模式
- **StandbyMode**：待机模式

## 15. QRCodeScanner（二维码扫描）
- **QRCodeScanner**：二维码扫描功能

## 16. RTC（实时时钟）
- **Simple_RTC**：简单RTC功能
- **Simple_RTC_Alarm**：带闹钟的RTC功能

## 17. SPI（串行外设接口）
- **Camera_2_Lcd**：摄像头图像显示到LCD
- **Camera_2_Lcd_JPEGDEC**：摄像头JPEG解码显示到LCD
- **DisplaySDJPG_ILI9341_TFT**：在ILI9341 TFT上显示SD卡中的JPEG
- **LCD_Screen_ILI9341_TFT**：ILI9341 TFT屏幕控制

## 18. USB（USB功能）
- **USB_Mass_Storage**：USB大容量存储设备
- **UVC_Device**：USB视频设备(UVC)

## 19. WS2812B（LED灯带）
- **WS2812B_Basics**：WS2812B LED基础控制
- **WS2812B_Patterns**：WS2812B LED模式显示

## 20. Watchdog（看门狗）
- **SimpleWDT**：简单看门狗定时器

## 21. WiFi（无线网络）
- **ConcurrentMode**：WiFi并发模式
- **ConnectToWiFi**：连接WiFi（支持无加密、WEP、WPA、WPA2企业级）
- **CreateWiFiAP**：创建WiFi接入点
- **IPv6TCPClient**：IPv6 TCP客户端
- **IPv6TCPServer**：IPv6 TCP服务器
- **IPv6UDPClient**：IPv6 UDP客户端
- **IPv6UDPServer**：IPv6 UDP服务器
- **SSLClient**：SSL客户端连接
- **ScanNetworks**：扫描网络
- **SimpleHttpRequest**：简单HTTP请求
- **SimpleHttpWeb**：简单HTTP网页服务（控制LED和接收数据）
- **SimpleTCPServer**：简单TCP服务器
- **SimpleUDP**：简单UDP通信
- **UDPCalculation**：UDP计算（接收延迟、接收超时、发送延迟）

## 22. Wire（I2C总线）
- **I2CScanner**：I2C设备扫描
- **LCD_HelloWorld**：LCD显示Hello World
- **MPU6050**：MPU6050陀螺仪加速度计（多种模式）
- **MasterReceiveData**：I2C主设备接收数据
- **MasterSendData**：I2C主设备发送数据
- **OLED_SSD1306**：SSD1306 OLED显示
- **VL53L0X**：VL53L0X距离传感器（连续和单次测量）
- **VL53L5CX**：VL53L5CX多区距离传感器（多种高级功能）

这些示例涵盖了AMB82-MINI开发板的各种功能，从基础的GPIO控制到高级的AI应用，为开发者提供了丰富的参考代码，可以根据项目需求选择相应的示例进行学习和开发。