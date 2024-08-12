# SysInfoViewer

SysInfoViewer is a C++ application that provides detailed system information for Linux systems. It offers a user-friendly graphical interface to display various system metrics, including CPU usage, memory usage, disk usage, battery information, and process details.

## Features

- System Information Overview
- Real-time CPU Usage Monitoring
- Memory Usage Statistics
- Disk Usage Visualization
- Battery Status and Health
- Process List with Resource Usage
- Miscellaneous System Information
- Applications installed in the computer

## Sample Screenshots
![Screenshot from 2024-07-03 15-21-54](https://github.com/user-attachments/assets/0dd5c13f-dfc1-4110-9f06-b1d38bb7258b)

![Screenshot from 2024-07-29 21-45-03](https://github.com/user-attachments/assets/d2b934b3-102e-42a6-8f0e-545547d7b164)
![Screenshot from 2024-07-29 21-44-54](https://github.com/user-attachments/assets/dc771418-dd48-4398-9c2f-3dac01af1770)
![Screenshot from 2024-07-29 21-44-33](https://github.com/user-attachments/assets/a4259165-c210-45f9-884f-080bad918951)
![Screenshot from 2024-07-29 21-44-11](https://github.com/user-attachments/assets/7731bdf6-00a8-4075-86b5-1218e8093588)


## Prerequisites

Before building and installing SysInfoViewer, ensure you have the following prerequisites installed on your system:

- C++ Compiler (GCC 14 or later, supporting C++23)
- CMake (version 3.29.6 or later)
- wxWidgets library (version 3.2.5 or later)
- Git (for cloning the repository)

## Cloning the Repository

To clone the SysInfoViewer repository, run the following command:

```bash
git clone https://github.com/Magpiny/SysInfoViewer.git
cd SysInfoViewer
```

## Building the Application

1. Create a build directory and navigate to it:

```bash
mkdir build
cd build
```

2. Generate the build files using CMake:

```bash
cmake ..
```

3. Build the application:

```bash
make
```

## Installing the Application

After building the application, you can install it on your system by running:

```bash
sudo make install
```

This will install the SysInfoViewer executable and necessary resources to your system.

## Running the Application

After installation, you can run SysInfoViewer by executing this command from your terminal:

```bash
SysInfoViewer
```

If you haven't installed the application, you can run it from the build directory:

```bash
./SysInfoViewer
```

## License

This project is licensed under a custom license. Please see the [LICENSE](LICENSE) file for details.

## Contributing

Contributions to SysInfoViewer are welcome. Please feel free to submit pull requests or open issues to improve the application.

## Author

SysInfoViewer is developed by Wanjare Samuel. For any questions or concerns, please contact: wanjaresamuel@gmail.com

## Acknowledgments

- wxWidgets library for the GUI framework
- All contributors and users of SysInfoViewer
