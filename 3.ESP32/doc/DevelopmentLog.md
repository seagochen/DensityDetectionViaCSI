**Table Of Content**

- [Install the Docker Image for ESP32 CSI project \[2023/09/25\]](#install-the-docker-image-for-esp32-csi-project-20230925)
  - [Install Docker](#install-docker)
    - [Install Docker on Ubuntu](#install-docker-on-ubuntu)
    - [Install Docker on Windows](#install-docker-on-windows)
  - [Install the Docker Image](#install-the-docker-image)
  - [Compile the ESP32 CSI project](#compile-the-esp32-csi-project)
    - [Use the Docker Image to Compile the ESP32 CSI project](#use-the-docker-image-to-compile-the-esp32-csi-project)
    - [Change the Chip Version](#change-the-chip-version)
    - [Flash the ESP32 CSI project](#flash-the-esp32-csi-project)
    - [Monitor the ESP32 CSI project](#monitor-the-esp32-csi-project)
    - [Clean the ESP32 CSI project](#clean-the-esp32-csi-project)
  - [Additional](#additional)
    - [Using Remote Serial Port](#using-remote-serial-port)
    - [Check the Serial Port Number in Windows](#check-the-serial-port-number-in-windows)
    - [Use Another Tool to Flash the Board](#use-another-tool-to-flash-the-board)




# Install the Docker Image for ESP32 CSI project [2023/09/25]

## Install Docker

### Install Docker on Ubuntu

```bash
sudo apt-get update
sudo apt-get install docker.io
```

### Install Docker on Windows

Download the Docker Desktop for Windows from [here](https://hub.docker.com/editions/community/docker-ce-desktop-windows/).


When downloaded docker-desktop-installer.exe, double click it to install. It will ask you to choose between WSL2 and Virtual Machine. From my experience, WSL2 is better than Hyper-V. So, I recommend you to choose WSL2.


## Install the Docker Image

Open the terminal (if you are using Windows, open the PowerShell) and type the following command.

```bash
bash
```

Then, type the following command.

```bash
docker pull espressif/idf:v5.0.4
```

to download the official Docker image for ESP32 development. As ESP32S3 is used in this project, the version of the Docker image is v5.0.4. For this project, we used the Docker image v5.0.4, and it worked well. However, if you want to use the latest version of the Docker image, you can use it. But, I cannot guarantee that it will work well.

## Compile the ESP32 CSI project

We have the following resources can be used for the ESP32 project.


- [ESP32 IDF](https://github.com/espressif/esp-idf)
- [ESP32 CSI](https://github.com/espressif/esp-csi)
- [Docker Image for ESP32 Development](https://hub.docker.com/r/espressif/idf)

The ESP32 IDF is the official development framework for ESP32. The ESP32 CSI is the official CSI library for ESP32. The ESP32 CSI library is based on the ESP32 IDF. So, we need to install the ESP32 IDF first, and then install the ESP32 CSI library.

However, when installing the ESP32 IDF, we may encounter some problems. That is why we use the Docker image for ESP32 related projects to avoid the problems. 

However, if you want to install the ESP32 IDF without using the Docker image, you can follow the instructions from [here](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html).

### Use the Docker Image to Compile the ESP32 CSI project

Suppose that you have already pulled the Docker image for ESP32 development. Then, you can use the following command to compile the ESP32 CSI project.

```bash
docker run -it --rm -v $PWD:/project -w /project espressif/idf:v5.0.4 idf.py build
```

* `docker run` is used to run the Docker image.
* `-it` is used to run the Docker image in the interactive mode.
* `--rm` is used to remove the Docker container after the Docker image is executed.
* `-v $PWD:/project` is used to mount the current directory to the `/project` directory in the Docker container.
* `-w /project` is used to set the working directory to the `/project` directory in the Docker container.
* `espressif/idf:v5.0.4` is the name of the Docker image.
* `idf.py build` is the command to compile the ESP32 CSI project.

For example, if you want to compile one of the examples in the ESP32 project, like `hello_world`, you can use the following command.

```bash
cd examples/get-started/hello_world
```

`CD` to the `hello_world` directory. Then, use the following command to compile the `hello_world` example.


```bash
docker run -it --rm -v $PWD:/project -w /project espressif/idf:v5.0.4 idf.py build
```

### Change the Chip Version

`espressif/idf` compiles the project into ESP32 as default, but for this project, it is a ESP32S3. So we have to dedicate the chip version.


```bash
docker run -it --rm -v $PWD:/project -w /project espressif/idf:v5.0.4 idf.py set-target esp32s3
```


### Flash the ESP32 CSI project


After the compilation is finished, you can find the `hello_world.bin` file in the `build` directory. Then, you can use the following command to flash the `hello_world.bin` file to the ESP32S3 board.

```bash
docker run -it --rm -v $PWD:/project -w /project espressif/idf:v5.0.4 idf.py flash
```

*For our project, the above command changes to the following command.*

```bash
docker run -it --rm -v $PWD:/project -w /project espressif/idf:v5.0.4 idf.py --port 'rfc2217://host.docker.internal:4000?ign_set_control' flash
```

### Monitor the ESP32 CSI project

After the flashing is finished, you can use the following command to monitor the ESP32S3 board.

```bash
docker run -it --rm -v $PWD:/project -w /project espressif/idf:v5.0.4 idf.py monitor
```

*For our project, the above command changes to the following command.*

```bash
docker run -it --rm -v $PWD:/project -w /project espressif/idf:v5.0.4 idf.py --port 'rfc2217://host.docker.internal:4000?ign_set_control' monitor
```

Then, you can see the following output.

![MonitorMode](./img/MonitorMode.png)

### Clean the ESP32 CSI project

If you want to clean the ESP32 CSI project, you can use the following command.

```bash
docker run -it --rm -v $PWD:/project -w /project espressif/idf:v5.0.4 idf.py fullclean
```

## Additional

### Using Remote Serial Port

The RFC2217 (Telnet) protocol can be used to remotely connect to a serial port. For more information please see the remote serial ports documentation in the ESP tool project. This method can also be used to access the serial port inside a Docker container if it cannot be accessed directly. Following is an example of how to use the Flash command from within a Docker container.

On host install and `start esp_rfc2217_server`:

On Windows, the package is available as a one-file bundled executable created by `pyinstaller` and it can be downloaded from the [esptool releases page](https://github.com/espressif/esptool/releases) in a ZIP archive along with other ESP tool utilities:

```bash
esp_rfc2217_server -v -p 4000 COM3
```

> Before you start to use the server, you have to make sure that board is connected to the host via the serial port. For our project, the 'USB-to-UART' is used to connect the ESP32S3 board to the host.

![ESP32S3](./img/ESP32S3_Board.jpg)

On Linux or macOS, the package is available as part of esptool, which can be found in the ESP-IDF environment or by installing using `pip`:

```bash
pip install esptool
```
And then starting the server by executing

```bash
esp_rfc2217_server.py -v -p 4000 /dev/ttyUSB0
```

Now the device attached to the host can be flashed from inside a Docker container by using:

```bash
docker run --rm -v <host_path>:/<container_path> -w /<container_path> espressif/idf idf.py --port 'rfc2217://host.docker.internal:4000?ign_set_control' flash
```

Please make sure that `<host_path>` is properly set to your project path on the host, and `<container_path>` is set as a working directory inside the container with the `-w` option. The `host.docker.internal` is a special Docker DNS name to access the host. This can be replaced with a host IP if necessary.


### Check the Serial Port Number in Windows

In Windows, you can check the serial port number by opening the Device Manager.

![Device Manager](./img/DeviceManager_CheckSerialPortNumOnWin.png)

Then, you can find the serial port number in the 'Ports (COM & LPT)'.


### Use Another Tool to Flash the Board

Instead of using the `idf.py` to flash the board, you can also use the `esptool.py` to flash the board. The `esptool.py` is a tool for communicating with ESP8266 and ESP32 chips. It is written in Python and works on Windows, macOS, Linux and BSD. It can communicate with the ESP32 chip over multiple protocols such as Serial UART, WiFi, and BLE. The `esptool.py` is a part of the ESP-IDF project. 

Use the following command to flash the board.

```bash
docker run -it --rm -v $PWD:/project -w /project espressif/idf:v5.0.4 esptool.py --chip esp32s3 --port COM3 --baud 115200 write_flash 0x0 hello_world.bin
```

Or directly, you can use the following command to flash the board.

```bash
esptool.py --chip esp32s3 --port COM3 --baud 115200 write_flash 0x0 hello_world.bin
```