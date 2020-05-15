# sleep-helper
M5GO平台的IoT设计，包括WiFi与红外控制。应用到了M5Stack提供的`ENV Unit`，`Light Unit`与`IR Unit`。
项目分为两部分，一部分是使用UIFlow编写的基础功能应用，另一部分是使用Arduino编写的WiFi控制红外信号传输。
## RemoteControl
使用UIFlow编写的程序，在M5GO上从左到右三个按钮分别对应三个界面：睡眠助手、天气助手、远程二维码。需在M5GO上连接`ENV Unit`与`Light Unit`。
### 睡眠助手
调用`Light Unit`的光线信息。光线较暗时会设定闹钟并有提示音，设定闹钟后在光线变亮时会有声音与LED提示起床。
### 天气助手
调用`ENV Unit`提供的各种环境信息，根据湿度和温度在界面提示信息。
### 远程二维码
扫描二维码进入远程控制界面。提供LED灯带开关与亮度调整功能以及显示各传感器信息。
## WiFiIR
使用Arduino编写的程序，在M5GO上实现远程WiFi遥控红外操作家电。需在M5GO上连接`ENV Unit`与`IR Unit`。该代码中加入的红外代码为格力Y502K遥控器的开/关代码。
### 红外学习
按下按钮A切换主界面与学习界面。在学习界面中对`IR Unit`发射红外信号，可以选择将该信号进行保存。保存后的信号可以在网页端进行操作。可以保存的信号数不设上限。
### 使用的库
- [IRremoteESP8266](https://github.com/crankyoldgit/IRremoteESP8266) 控制红外信号传输
- [Adafruit_NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel) 控制LED BAR
- [Adafruit_BMP280](https://github.com/adafruit/Adafruit_BMP280_Library) 读取ENV Unit的信息
