# sendIoTHubSenser
まずazureiotsdkのインストールを行う必要があります。（wiringPIのインストールも前もって実行してください）

１．buildにひつようなパッケージをインストールします

sudo apt-get update
sudo apt-get install -y git cmake build-essential curl libcurl4-openssl-dev libssl-dev uuid-dev

２．git　cloneでコードをダウンロードします。

git clone -b <yyyy-mm-dd> --recursive https://github.com/Azure/azure-iot-sdk-c.git

yyyy-mm-ddの箇所には以下のUrlをしらべ最新バージョンの日付を調べて入力します
https://github.com/Azure/azure-iot-sdk-c/releases/latest

例
git clone -b 2017-09-08 --recursive https://github.com/Azure/azure-iot-sdk-c.git

３.ダウンロードしたコマンドをビルドします
cd azure-iot-sdk-c
mkdir cmake
cd cmake
cmake ..
cmake --build .

４．ビルドしたライブラリをインストールします（もしかして不要かも・・・）

sudo　make install


５．このsendIoTHubSenserアプリケーションのビルドを行います。

/cmake　ディレクトリに移動し次のコマンドを打ちます

% cmake　../.

/cmakeに入ったまま、makeします

% make
