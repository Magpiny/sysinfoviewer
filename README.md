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

## Screenshots

![Screenshot from 2024-07-03 15-21-54](https://github.com/user-attachments/assets/0d9d958d-a9d7-4d39-b0e6-e464585a904d)

![sysinfo2](https://github.com/user-attachments/assets/80e3b91b-3b73-4c5b-a5d4-1d4aa7c3dc46)

![image](https://github.com/user-attachments/assets/a86a3fa8-d115-42e2-8e5b-a5780395c6c2)


## Prerequisites

Before building and installing SysInfoViewer, ensure you have the following prerequisites installed on your system:

- C++ Compiler (GCC 11 or later, supporting C++23)
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

After installation, you can run SysInfoViewer by executing:

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
