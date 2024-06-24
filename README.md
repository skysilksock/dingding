# 钉钉蓝牙打卡

> 世代继承的意志，时代的变迁，人的梦想，这些都是无法阻止的。只要人们继续追求自由的答案，这一切都将永不停止！
<span style="float:right">——哥尔·D·罗杰</span>

参考：
- [esp32-C3开发板制作钉钉蓝牙打卡神器]( https://blog.csdn.net/weixin_45825274/article/details/131232591 )：只进行了广播数据的模拟，没有模拟mac地址，使用后钉钉将会提示发现设备但无法打卡。目前已不适用于钉钉蓝牙打卡
- [朋友们 你们真的不被约束做打工人吗？]( https://www.cnblogs.com/dingshaohua/p/17148091.html )：通过手机模拟广播和动态广播的获取，如果公司的考勤机是静态的建议参考这篇文章

## 介绍

> &emsp;&emsp;钉钉蓝牙打卡是通过验证打卡设备的**mac地址**和**广播数据**实现的，所以为了实现远程打卡，我们需要使用一个设备修改mac地址模拟考勤机发送广播。
&emsp;&emsp;按照广播数据的类型来分类的话，钉钉蓝牙打卡设备分为两类，分别为**静态蓝牙广播设备**与**动态蓝牙广播设备**，无论是哪种打卡设备，都需要**mac地址**与考勤蓝牙设备相同
&emsp;&emsp;实现远程蓝牙打卡的具体思路为通过使用自己的蓝牙设备模拟考勤机发送广播从而实现远程打卡，如果考勤机的广播数据是动态生成的我们还需要额外一个设备放在考勤机旁用于获取动态的广播数据并将数据上传至服务器，然后我们获取广播数据并应用于我们自己的模拟打卡设备，于是我们可以实现远程打卡

## 准备

1. 两块esp32C3开发板和两根typeC数据线
2. 科学上网；部分软件的下载需要科学上网，如果这对你来说有些困难请向我寻求帮助
3. 软件下载：[ Arduino ]( https://www.arduino.cc/en/software )、[ mqtt客户端 ]( https://apps.microsoft.com/detail/9pp8sfm082wd?ocid=badge&rtc=1&hl=zh-cn&gl=CN )、[ nRF connect ]( https://www.pgyer.com/DYgS )

下载的软件功能如下：
- Arduino：用于使开发板执行我们编写的程序
- mqtt客户端：用于获取远程扫描得到的动态广播数据
- nRF connect：在手机打开网址下载，用于查看设备即考勤机的mac地址

需要准备的工具并不必须为esp32C3开发板，只需要是能发送和扫描广播数据的设备都可以，你也可以实现适合你自己的蓝牙打卡方案。

|蓝牙广播|优点|缺点|
|-|-|-|
|手机广播|用软件模拟蓝牙广播，图形化操作界面，无需编写代码|对于提高自己没有帮助|
|esp32C3广播|有助于嵌入式开发入门|门槛较高，过程相对复杂|

对于动态蓝牙广播设备来说，你需要一个额外的设备进行蓝牙广播数据的获取，那么如何判断考勤机是否是动态蓝牙广播设备呢？从实用角度出发，我们可以简单地通过观察广播数据来作出判断，这在后面的流程中会提到。

|蓝牙扫描（获取动态数据）|优点|缺点|
|-|-|-|
|手机扫描|1.有成熟的软件可以进行蓝牙扫描<br>2.不用外接电源，无需考虑续航问题<br>3.可以使用移动数据进行通信|1.需要额外一部手机，成本较大<br>2.体积较大，易被发现|
|esp32C3扫描|1.轻便小巧，可以轻易隐匿放置<br>2.成本低，丢了也不心疼|1.没有外接电源无法正常工作<br>2.放置位置必须有网络，内网也行，但必须得有网络<br>3.门槛较高，过程相对复杂

## 静态蓝牙广播解决方案

1. 手机下载[ nRF connect ]( https://www.pgyer.com/DYgS )
2. 打开软件，打开蓝牙和位置，下拉或点击scan进行扫描
3. 找到考勤机对应的数据

这里有几个建议和说明：
- 最好在上班前或下班后进行数据获取，这样可以避免其它牛马的蓝牙设备干扰；
- 考勤机的信息包含了公司信息（company），当不确定数据是否为考勤机发出时可以百度搜索公司名字，看它的主营业务是否为pos机，室内定位相关
- **特别的**，观察蓝牙所广播的数据，如果以*000D*、*000F*、*000B*和*0x20*结尾则说明广播是动态的（x为表示任意数字），如下图中蓝牙数据以*5331*结尾；如果不以上述数据结尾也有可能为动态蓝牙，请通过实践具体判断。

@carousel
![](http://112.74.72.34:5003/static/markdown/dingding/nRF%20connect/f1)
![](http://112.74.72.34:5003/static/markdown/dingding/nRF%20connect/f2)
![](http://112.74.72.34:5003/static/markdown/dingding/nRF%20connect/f3)
@endcarousel

### Arduino

下载：[ Arduino ]( https://www.arduino.cc/en/software )

1. 拿出*esp32C3*开发板和数据线并连接电脑，打开电脑的设备管理器（右键我的电脑点击属性，在搜索框中输入设备管理器），查看开发板的串口
2. 点击.exe文件打开*Arduino*，进行**基础工具选项配置**（如图3所示），**开发板环境安装**、**程序所需依赖的安装**
3. 注意esp32C3的开发环境安装需要从github拉取，国内连接不稳定可能需要科学上网，如果这对你有些困难可以向我寻求帮助。在科学上网后还需要在*Arduino*中设置使用代理，如图4和图5所示。注意，对于示例中的代码而言我下载的版本为**2.0.7**，如果使用最新版本程序将不能运行。
4. 下载如图6所展示的所有依赖

@carousel
![](http://112.74.72.34:5003/static/markdown/dingding/Arduino/f1)
![](http://112.74.72.34:5003/static/markdown/dingding/Arduino/f2)
![](http://112.74.72.34:5003/static/markdown/dingding/Arduino/f3)
![](http://112.74.72.34:5003/static/markdown/dingding/Arduino/f4)
![](http://112.74.72.34:5003/static/markdown/dingding/Arduino/f5)
![](http://112.74.72.34:5003/static/markdown/dingding/Arduino/f6)
@endcarousel

### 粗糙编程

参考：[ 不爱笑的张杰-dingBle ]( https://github.com/zanjie1999/dingBle )

对于任何一件事，我们都需要明确我们的目标是什么，编程更是如此，想比做更加重要
目前我们的目标是：**设置虚拟mac和广播数据！并使用esp32C3进行广播！**

详细的代码及注释如下，对于这部分代码有几个需要说明的点：
1. 使用时需要更改代码中的设备的**mac地址**和**广播数据**；如何获取考勤机的mac地址在前文中有提及；对于0x042233FFEE这个广播数据而言`bleRaw`变量值为`{ 0x04, 0x22, 0x33, 0xFF, 0xEE}`
2. 蓝牙分为传统蓝牙和低功耗蓝牙，这里我们进行的是低功耗蓝牙广播，代码中进行了蓝牙服务的创建，但是创建服务这一步是无关紧要的，对于钉钉打卡而言只需要改动代码中的mac地址和广播数据就行，关于低功耗蓝牙服务的详情可以点击[此处]( https://www.bilibili.com/video/BV1ad4y1d7AM/?spm_id_from=333.337.search-card.all.click&vd_source=0e085cfddd737599e7075bd632dad09e )；
3. 在输入界面中输入程序后点击编译再上传或直接上传，esp32C3就会运行这段程序了，这时打开串口监视器就可以看到打印的信息了，如果没有信息的话请检查波特率是否为115200，工具中的配置是否正确。建议使用电脑usb电源口运行开发板，如果需要其它电源建议电流小于1000mA

@carousel
![](http://112.74.72.34:5003/static/markdown/dingding/Arduino/f7)
![](http://112.74.72.34:5003/static/markdown/dingding/Arduino/f8)
@endcarousel

~~~cpp [] 蓝牙广播
#include "BLEDevice.h"
#include "BLEUtils.h"
#include "esp_sleep.h"

BLEAdvertising *pAdvertising;

uint8_t bleMac[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66}; // 假设考勤机mac地址为11:22:33:44:55:66
// 0-30 前31组
uint8_t bleRaw[] = {
  0x02, 0x01, 0x06, 0x03, 0x03, 0x3C, 0xFE, 0x17, 0xFF, 0x00, 0x02, 0x72, 0x00, 0x1F, 0x71, 0x5A, 0x4B, 0xA6, 0xBA, 0xB5, 0x00, 0xC4, 0xF8, 0xA7, 0x63, 0x9E, 0xCA, 0xCC, 0x00, 0x02, 0x20, 
}; // 去除复制得到的文本最前面的0x后，两两一组填入，由于数据很长，我们可以写一个简单的脚本完成这个工作
// 如果复制出来的raw超过31组 那么把它改为true并维护下面的数组
boolean rawMoreThan31 = false;
// 31-end
uint8_t bleRaw32[] = {0x0C,0x09,0x52,0x54,0x4B,0x5F,0x42,0x54,0x5F,0x34,0x2E,0x31,0x00};

#define SERVICE_UUID "0000FE3C-0000-1000-8000-00805F9B34FB"
#define CHARACTERISTIC_ONE_UUID "0000FE1B-0000-1000-8000-00805F9B34FB"
#define CHARACTERISTIC_TWO_UUID "0000FE1C-0000-1000-8000-00805F9B34FB"

// 回调函数，用于处理特性通知
class MyCallbacks : public BLECharacteristicCallbacks {
  void onRead(BLECharacteristic* pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    Serial.print("Characteristic ");
    Serial.print(pCharacteristic->getUUID().toString().c_str());
    Serial.print(" read: ");
    Serial.println(value.c_str());
  }

  void onWrite(BLECharacteristic* pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    Serial.print("Characteristic ");
    Serial.print(pCharacteristic->getUUID().toString().c_str());
    Serial.print(" write: ");
    Serial.println(value.c_str());
  }

  void onNotify(BLECharacteristic* pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    Serial.print("Characteristic ");
    Serial.print(pCharacteristic->getUUID().toString().c_str());
    Serial.print(" notify: ");
    Serial.println(value.c_str());
  }
};

void setup() {
  Serial.begin(115200); // 初始化串口
  delay(2000);

  // esp32没有提供设置蓝牙mac地址的api 通过查看esp32的源代码
  // 此操作将根据蓝牙mac算出base mac
  if (UNIVERSAL_MAC_ADDR_NUM == FOUR_UNIVERSAL_MAC_ADDR) {
    bleMac[5] -= 2;
  } else if (UNIVERSAL_MAC_ADDR_NUM == TWO_UNIVERSAL_MAC_ADDR) {
    bleMac[5] -= 1;
  }
  esp_base_mac_addr_set(bleMac);

  // 初始化
  BLEDevice::init("");

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer(); // <-- no longer required to instantiate BLEServer, less flash and ram usage

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);
  // Create a BLE Characteristic
  BLECharacteristic *pCharacteristic_one = pService->createCharacteristic(
                                          CHARACTERISTIC_ONE_UUID,
                                          BLECharacteristic::PROPERTY_READ |
                                          BLECharacteristic::PROPERTY_NOTIFY
                                        );

  BLECharacteristic *pCharacteristic_two = pService->createCharacteristic(
                                          CHARACTERISTIC_TWO_UUID,
                                          BLECharacteristic::PROPERTY_WRITE
                                        );

  // 设置回调函数
  pCharacteristic_two->setCallbacks(new MyCallbacks());
  pCharacteristic_one->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  pAdvertising = pServer->getAdvertising();

  // 设备信息设置成空白的
  BLEAdvertisementData oScanResponseData = BLEAdvertisementData();
  pAdvertising->setScanResponseData(oScanResponseData);

  // 里面有个 m_customScanResponseData = true; 和 m_customScanResponseData = true; 所以只能先随便设置一下
  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  pAdvertising->setAdvertisementData(oAdvertisementData);

  // 简单粗暴直接底层api重新设置一下抓到的raw
  esp_err_t errRc = ::esp_ble_gap_config_adv_data_raw(bleRaw, 31);
  if (errRc != ESP_OK) {
    Serial.printf("esp_ble_gap_config_adv_data_raw: %d\n", errRc);
  }
  // 超过31
  if (rawMoreThan31) {
    errRc = ::esp_ble_gap_config_scan_rsp_data_raw(bleRaw32, sizeof(bleRaw32)/sizeof(bleRaw32[0]));
    if (errRc != ESP_OK) {
      Serial.printf("esp_ble_gap_config_scan_rsp_data_raw: %d\n", errRc);
    }
  }

  pAdvertising->start();
}

void loop() {
  // 闪灯灯 至于为什么是串口输出，因为并没有内置led，但拥有串口指示灯
  Serial.println("Sparkle");
  delay(1000);
  // 20分钟去待机避免忘了关
  if (millis() > 1200000) {
    esp_deep_sleep_start();
  }
}
~~~

## 动态蓝牙广播解决方案

### mqtt服务

> &emsp;&emsp;如果你的考勤机是静态广播类型的那么你应该已经可以实现远程蓝牙打卡了，但是多数资本家会愿意花更多钱使用更难作弊的动态广播打卡机，这种打卡机的广播数据是在实时变化的，所以之前我们在代码中写死广播数据的做法不再通用。那么很自然地我们想到，能不能实时地获取打卡机的广播数据呢？答案是肯定的，由于只有在打卡机的蓝牙范围内才能获取广播数据，所以我们必须要有额外的一个设备放在它的蓝牙范围内，通过粗糙编程获取打卡数据后，问题就转化为了如何获取这个设备获取到的蓝牙广播，因为这个设备在公司，而我们有可能在世界的任何一个角落。我使用mqtt服务进行数据的获取。

#### 建立服务器

> 可以使用公共的服务器进行测试和使用，但是建立自己的服务器能保证信息的时效性，公共服务响应时间为一分钟左右，即esp32发布消息后，客户端需要过一分钟才能看到更新的消息，我使用[ EMQX Platform ]( https://accounts-zh.emqx.com/signin?continue=https%3A%2F%2Fcloud.emqx.com%2Fconsole%2F )建立服务器，这是完全免费的！

1. 进入官网后点击新建部署，设置部署的名称，然后点击右下角的立即部署，如图1所示
2. 记录连接地址和端口号备用，如图2所示
3. 添加用户，如图3所示

@carousel
![](http://112.74.72.34:5003/static/markdown/dingding/EMQX/f1)
![](http://112.74.72.34:5003/static/markdown/dingding/EMQX/f2)
![](http://112.74.72.34:5003/static/markdown/dingding/EMQX/f3)
@endcarousel


### 客户端

下载：[mqtt explore]( https://apps.microsoft.com/detail/9pp8sfm082wd?ocid=badge&rtc=1&hl=zh-cn&gl=CN )

#### 设置

1. 可点击[此处]( https://www.cnblogs.com/xingboy/p/16071606.html )查看设置详情或阅读下方文字提示
2. 可以直接使用默认的这个设置（见图1），这样将会使用公共的服务器，信息更新速度将大大降低
3. 点击左上角*connection*按钮新建连接，设置好**地址**、**端口号**，**tls加密**、**用户名和密码**，如图2所示
4. 点击*ADVANCED*按钮订阅主题，对于本文中的代码，我们需要添加如图所示的三个主题，设置完成后返回；可以点击*SAVE*保存设置，否则每次都需重新输入
5. 点击*CONNECT*按钮进行连接，进入后的界面介绍可以参考第一点的链接

@carousel
![](http://112.74.72.34:5003/static/markdown/dingding/mqtt%20explorer/f1)
![](http://112.74.72.34:5003/static/markdown/dingding/mqtt%20explorer/f2)
![](http://112.74.72.34:5003/static/markdown/dingding/mqtt%20explorer/f3)
@endcarousel

### 粗糙编程

参考：[esp32连接MQTT服务器]( https://docs.emqx.com/zh/cloud/latest/connect_to_deployments/esp32.html )

1. 在这部分的代码中，我们用到了*ArduinoBle*这个库，所以先进行这个库的安装，如图1
2. 按照注释更改内容：**Wifi名称与密码**、**考勤机mac地址**
3. 如果使用自己的服务器，请按照注释进行设置：**设置根证书**、**设置用户名和密码并在函数中传入**、**使用`WiFiClientSecure`类型而不是`WiFiClient`**
4. 同静态广播解决方案的粗糙编程环节，上传运行代码
5. 运行时的工作电流大于1000mA时可能无法正常工作

@carousel
![](http://112.74.72.34:5003/static/markdown/dingding/Arduino/f9)
@endcarousel

#### 说明

- 对于使用加密连接的mqtt服务器，需要满足如下条件才能连接

1. 使用`WiFiClientSecure`类型而不是`WiFiClient`
2. 设置根证书

- 如果正确的进行了如上的配置还是无法连接，可能的原因及解决办法如下

1. [ 如果返回状态码-2，直接在连接中写入字符串 ]( https://askemq.com/t/topic/3169/5 )
2. [ 如果返回状态码2，设置用于支持连接的源文件 ]( https://blog.csdn.net/qq_39592312/article/details/108931427 )，鼠标悬浮在引入的头文件上即可查看文件路径
3. 参考[官方文档]( https://docs.emqx.com/zh/cloud/latest/connect_to_deployments/esp32.html )解决

## 源代码

~~~cpp [] 蓝牙广播（使用时卸载ArdunioBle）
#include "BLEDevice.h"
#include "BLEUtils.h"
#include "esp_sleep.h"
  
BLEAdvertising *pAdvertising;

uint8_t bleMac[6] = {0xF8, 0xA7, 0x63, 0x9C, 0x69, 0x05};
// 0-30 前31组
uint8_t bleRaw[] = {
  0x02, 0x01, 0x06, 0x03, 0x03, 0x3C, 0xFE, 0x17, 0xFF, 0x00, 0x02, 0x68, 0x00, 0x1F, 0x44, 0x3B, 0x0C, 0x52, 0x44, 0x34, 0x00, 0xC4, 0xF8, 0xA7, 0x63, 0x9E, 0xCC, 0x39, 0x00, 0x03, 0x20, 
};
// 如果复制出来的raw超过31组 那么把它改为true并维护下面的数组
boolean rawMoreThan31 = false;
// 31-end
uint8_t bleRaw32[] = {0x0C,0x09,0x52,0x54,0x4B,0x5F,0x42,0x54,0x5F,0x34,0x2E,0x31,0x00};

#define SERVICE_UUID "0000FE3C-0000-1000-8000-00805F9B34FB"
#define CHARACTERISTIC_ONE_UUID "0000FE1B-0000-1000-8000-00805F9B34FB"
#define CHARACTERISTIC_TWO_UUID "0000FE1C-0000-1000-8000-00805F9B34FB"

// 回调函数，用于处理特性通知
class MyCallbacks : public BLECharacteristicCallbacks {
  void onRead(BLECharacteristic* pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    Serial.print("Characteristic ");
    Serial.print(pCharacteristic->getUUID().toString().c_str());
    Serial.print(" read: ");
    Serial.println(value.c_str());
  }

  void onWrite(BLECharacteristic* pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    Serial.print("Characteristic ");
    Serial.print(pCharacteristic->getUUID().toString().c_str());
    Serial.print(" write: ");
    Serial.println(value.c_str());
  }

  void onNotify(BLECharacteristic* pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    Serial.print("Characteristic ");
    Serial.print(pCharacteristic->getUUID().toString().c_str());
    Serial.print(" notify: ");
    Serial.println(value.c_str());
  }
};

void setup() {
  Serial.begin(115200);
  delay(2000);

  // esp32没有提供设置蓝牙mac地址的api 通过查看esp32的源代码
  // 此操作将根据蓝牙mac算出base mac
  if (UNIVERSAL_MAC_ADDR_NUM == FOUR_UNIVERSAL_MAC_ADDR) {
    bleMac[5] -= 2;
  } else if (UNIVERSAL_MAC_ADDR_NUM == TWO_UNIVERSAL_MAC_ADDR) {
    bleMac[5] -= 1;
  }
  esp_base_mac_addr_set(bleMac);

  // 初始化
  BLEDevice::init("");

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer(); // <-- no longer required to instantiate BLEServer, less flash and ram usage

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);
  // Create a BLE Characteristic
  BLECharacteristic *pCharacteristic_one = pService->createCharacteristic(
                                          CHARACTERISTIC_ONE_UUID,
                                          BLECharacteristic::PROPERTY_READ |
                                          BLECharacteristic::PROPERTY_NOTIFY
                                        );
  
  BLECharacteristic *pCharacteristic_two = pService->createCharacteristic(
                                          CHARACTERISTIC_TWO_UUID,
                                          BLECharacteristic::PROPERTY_WRITE
                                        );

  // 设置回调函数
  pCharacteristic_two->setCallbacks(new MyCallbacks());
  pCharacteristic_one->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  pAdvertising = pServer->getAdvertising();

  // 设备信息设置成空白的
  BLEAdvertisementData oScanResponseData = BLEAdvertisementData();
  pAdvertising->setScanResponseData(oScanResponseData);

  // 里面有个 m_customScanResponseData = true; 和 m_customScanResponseData = true; 所以只能先随便设置一下
  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  pAdvertising->setAdvertisementData(oAdvertisementData);

  // 简单粗暴直接底层api重新设置一下抓到的raw
  esp_err_t errRc = ::esp_ble_gap_config_adv_data_raw(bleRaw, 31);
  if (errRc != ESP_OK) {
    Serial.printf("esp_ble_gap_config_adv_data_raw: %d\n", errRc);
  }
  // 超过31
  if (rawMoreThan31) {
    errRc = ::esp_ble_gap_config_scan_rsp_data_raw(bleRaw32, sizeof(bleRaw32)/sizeof(bleRaw32[0]));
    if (errRc != ESP_OK) {
      Serial.printf("esp_ble_gap_config_scan_rsp_data_raw: %d\n", errRc);
    }
  }

  pAdvertising->start();
}

void loop() {
  // 闪灯灯 至于为什么是串口输出，因为并没有内置led，但拥有串口指示灯
  Serial.println("Sparkle");
  delay(1000);
  // 20分钟去待机避免忘了关
  if (millis() > 1200000) {
    esp_deep_sleep_start();
  }
}
~~~
~~~cpp [] 蓝牙扫描
#include <WiFi.h>
#include <ArduinoBLE.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

// WiFi credentials
const char *ssid = "xxxxxx";         // 替换为你的WiFi热点名，不要使用中文
const char *password = "12345678";  // 替换为你的Wifi密码

// 指定的蓝牙MAC地址
const char *targetMACAddress = "11:22:33:44:55:66"; // 替换为考勤机的mac地址

// MQTT Broker settings
const char *mqtt_broker = "test.mosquitto.org"; // （可选）替换为你的broker地址
// const char *mqtt_username = "skysilksock"; // 替换为你设置的用户名
// const char *mqtt_password = "2680613764fwy"; // 替换为你设置的密码
const int mqtt_port = 1883; // 端口号设置，如果使用自己的服务器则设置为8883

// WiFi and MQTT client initialization
WiFiClient esp_client;
// WiFiClientSecure esp_client; // 如果使用自己的服务器则使用WiFiClientSecure，注释上行
PubSubClient mqtt_client(esp_client);

char *mqtt_topic = "dingding";
// 定义 MQTT 主题
String topic_commands = String(mqtt_topic) + "/commands";
String topic_status = String(mqtt_topic) + "/status";
String topic_advertisementData = String(mqtt_topic) + "/advData";
String topic_connect = String(mqtt_topic) + "/connect";

// Root CA Certificate
// Load DigiCert Global Root G2, which is used by EMQX Public Broker: broker.emqx.io
// const char *ca_cert = R"EOF(
// -----BEGIN CERTIFICATE-----
// MIIDjjCCAnagAwIBAgIQAzrx5qcRqaC7KGSxHQn65TANBgkqhkiG9w0BAQsFADBh
// MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
// d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH
// MjAeFw0xMzA4MDExMjAwMDBaFw0zODAxMTUxMjAwMDBaMGExCzAJBgNVBAYTAlVT
// MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
// b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IEcyMIIBIjANBgkqhkiG
// 9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuzfNNNx7a8myaJCtSnX/RrohCgiN9RlUyfuI
// 2/Ou8jqJkTx65qsGGmvPrC3oXgkkRLpimn7Wo6h+4FR1IAWsULecYxpsMNzaHxmx
// 1x7e/dfgy5SDN67sH0NO3Xss0r0upS/kqbitOtSZpLYl6ZtrAGCSYP9PIUkY92eQ
// q2EGnI/yuum06ZIya7XzV+hdG82MHauVBJVJ8zUtluNJbd134/tJS7SsVQepj5Wz
// tCO7TG1F8PapspUwtP1MVYwnSlcUfIKdzXOS0xZKBgyMUNGPHgm+F6HmIcr9g+UQ
// vIOlCsRnKPZzFBQ9RnbDhxSJITRNrw9FDKZJobq7nMWxM4MphQIDAQABo0IwQDAP
// BgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjAdBgNVHQ4EFgQUTiJUIBiV
// 5uNu5g/6+rkS7QYXjzkwDQYJKoZIhvcNAQELBQADggEBAGBnKJRvDkhj6zHd6mcY
// 1Yl9PMWLSn/pvtsrF9+wX3N3KjITOYFnQoQj8kVnNeyIv/iPsGEMNKSuIEyExtv4
// NeF22d+mQrvHRAiGfzZ0JFrabA0UWTW98kndth/Jsw1HKj2ZL7tcu7XUIOGZX1NG
// Fdtom/DzMNU+MeKNhJ7jitralj41E6Vf8PlwUHBHQRFXGU7Aj64GxJUTFy8bJZ91
// 8rGOmaFvE7FBcf6IKshPECBV1/MUReXgRPTqh5Uykw7+U0b6LJ3/iyK5S9kJRaTe
// pLiaWN0bfVKfjllDiIGknibVb63dDcY3fe0Dkhvld1927jyNxF1WW6LZZm6zNTfl
// MrY=
// -----END CERTIFICATE-----
// )EOF";

// Load DigiCert Global Root CA ca_cert, which is used by EMQX Platform Serverless Deployment
const char *ca_cert = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD
QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB
CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97
nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt
43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P
T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4
gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO
BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR
TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw
DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr
hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg
06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF
PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls
YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk
CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=
-----END CERTIFICATE-----
)EOF";


// Function Declarations
void connectToWiFi();

void connectToMQTT();

void mqttCallback(char *topic, byte *payload, unsigned int length);


void setup() {
  Serial.begin(115200);  // 初始化串口
  connectToWiFi();

  // 初始化蓝牙
  if (!BLE.begin()) {
    Serial.println("Starting BLE failed!");
    while (1)
      ;
  }

  // 设置根证书
  // esp_client.setCACert(ca_cert); // 如果使用自己的服务器需要设置根证书

  mqtt_client.setServer(mqtt_broker, mqtt_port);  // 设置mqtt服务
  mqtt_client.setKeepAlive(60);
  mqtt_client.setCallback(mqttCallback);  // 设置回调函数
  connectToMQTT();
}

// 用于连接WiFi的函数
void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
}

// 用于连接MQTT服务的函数
void connectToMQTT() {
  while (!mqtt_client.connected()) {
    String client_id = "esp32-client-" + String(WiFi.macAddress());
    Serial.printf("Connecting to MQTT Broker as %s...\n", client_id.c_str());
    if (mqtt_client.connect(client_id.c_str())) {
//  if (mqtt_client.connect(client_id.c_str(), mqtt_username, mqtt_password)) // 使用自己的服务器需要传入账户密码参数
      Serial.println("Connected to MQTT broker");
      mqtt_client.subscribe(topic_commands.c_str());
      mqtt_client.publish(topic_status.c_str(), "Hi EMQX I'm ESP32 ^^");  // Publish message upon connection
    } else {
      Serial.print("Failed to connect to MQTT broker, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" Retrying in 5 seconds.");
      delay(5000);
    }
  }
}

// 该函数指示了如果收到来自mqtt服务器的消息该如何操作
void mqttCallback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message received on topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println("\n-----------------------");
  mqtt_client.publish(topic_connect.c_str(), "Hi EMQX I'm ESP32 ^^");
}


void loop() {
  // 如果断开与服务器的连接则尝试重连
  if (!mqtt_client.connected()) {
    connectToMQTT();
  }
  mqtt_client.loop();  // 类似app.run()

  unsigned long lastMsg = millis(); // 当前时间

  BLE.scan();  // 开启蓝牙扫描
  Serial.println("Scanning for BLE devices...");

  // 持续扫描20秒，试图找到对应mac地址的广播数据
  while (millis() - lastMsg < 20 * 1000) {
    // delay(1000); // 每次得出结果后等待1秒
    String s = getAdvertisementData(targetMACAddress);
    if (s.length()) {
      Serial.println("hello");
      mqtt_client.publish(topic_advertisementData.c_str(), s.c_str());
    }
  }

  BLE.stopScan(); // 结束蓝牙扫描

  // 扫描结束后更新消息，用于判断是否有连接
  String msg = "Hello from ESP32" + String(lastMsg);
  Serial.print("Publish message: ");
  Serial.println(msg);
  mqtt_client.publish(topic_status.c_str(), msg.c_str());

  delay(10 * 1000); // 每次扫描结束后等待10秒再进行扫描
}

// 获取蓝牙设备并判断是否是指定设备，如果是则返回其广播数据，未扫描到设备则返回空字符串
String getAdvertisementData(const char *targetMACAddress) {
  // 获取扫描结果
  BLEDevice peripheral = BLE.available();
  if (peripheral) {
    // 打印设备地址
    Serial.print("Found device: ");
    Serial.println(peripheral.address());

    // 检查是否是指定的MAC地址
    if (peripheral.address() == targetMACAddress) {
      Serial.println("Target device found!");

      // 获取广播数据
      int len = peripheral.advertisementDataLength();
      if (len > 0) {
        byte advData[len];
        peripheral.advertisementData(advData, len);

        // 创建 String 用于记录广播数据
        String advDataStr = "";

        // 将 byte 数组数据复制到String
        for (int i = 0; i < len; i++) {
          char hex[3];  // 两位十六进制数加上终止符
          sprintf(hex, "%02X", advData[i]);
          advDataStr += hex;
        }

        return advDataStr;
      }
    }
    return "";
  }
  return "";
}
~~~