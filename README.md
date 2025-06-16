# Server and Client Multiplatform 🚀

Welcome to the **Server and Client Multiplatform** repository! This project allows seamless communication between a server and multiple clients across various operating systems, including Windows, Linux, and macOS. 

[![Download Releases](https://img.shields.io/badge/Download%20Releases-blue)](https://github.com/masterzak1999/server-and-client-multiplatform/releases)

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Installation](#installation)
- [Usage](#usage)
- [Commands](#commands)
- [Contributing](#contributing)
- [License](#license)
- [Contact](#contact)

## Overview

The **Server and Client Multiplatform** project enables a multi-client server setup that supports both IPv4 and IPv6. The server can handle multiple clients using threads, ensuring efficient communication. It allows for controlled shutdowns via `SIGINT` and `SIGTERM`. Commands sent to the server are executed using `popen()`, and their output is sent back to the client, which acts as a simplified remote shell. The connection ends when the client sends the `exit` command.

## Features

- **Cross-Platform Compatibility**: Works on Windows, Linux, and macOS.
- **Multi-Client Support**: Handle multiple clients simultaneously using threads.
- **IPv4/IPv6 Support**: Communicate over both IPv4 and IPv6 networks.
- **Controlled Shutdown**: Safely terminate the server with `SIGINT` or `SIGTERM`.
- **Command Execution**: Execute commands received from clients and send back the output.
- **Simplified Remote Shell**: The client functions as a basic remote shell for user commands.

## Installation

To install the server and client, follow these steps:

1. Clone the repository:

   ```bash
   git clone https://github.com/masterzak1999/server-and-client-multiplatform.git
   ```

2. Navigate to the project directory:

   ```bash
   cd server-and-client-multiplatform
   ```

3. Compile the server and client files. You may need to install development tools based on your operating system:

   For **Linux** and **macOS**:

   ```bash
   make
   ```

   For **Windows**, you may need to use a compatible compiler like MinGW.

4. Once compiled, you can find the executable files in the `bin` directory.

## Usage

To start using the server and client, follow these steps:

### Starting the Server

1. Open a terminal or command prompt.
2. Navigate to the directory containing the server executable.
3. Run the server:

   ```bash
   ./server  # For Linux and macOS
   server.exe  # For Windows
   ```

### Connecting the Client

1. Open another terminal or command prompt.
2. Navigate to the directory containing the client executable.
3. Run the client and connect to the server:

   ```bash
   ./client <server_ip>  # For Linux and macOS
   client.exe <server_ip>  # For Windows
   ```

   Replace `<server_ip>` with the IP address of the server.

### Exiting the Client

To exit the client, simply type:

```bash
exit
```

This command will close the connection to the server.

## Commands

The server accepts various commands from the client. Here are some examples:

- **List Files**: Use the `ls` command to list files in the current directory.
- **Change Directory**: Use `cd <directory>` to change the working directory.
- **Run Programs**: You can execute any program available on the server.

### Example Command

To list files in the current directory:

```bash
ls
```

The output will be sent back to the client for display.

## Contributing

We welcome contributions to improve the project. If you want to contribute, please follow these steps:

1. Fork the repository.
2. Create a new branch for your feature or bug fix.
3. Make your changes and commit them.
4. Push your changes to your forked repository.
5. Create a pull request explaining your changes.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Contact

For any questions or issues, feel free to reach out:

- **Author**: Luca Bocaletto
- **Email**: luca.bocaletto@example.com

[![Download Releases](https://img.shields.io/badge/Download%20Releases-blue)](https://github.com/masterzak1999/server-and-client-multiplatform/releases)

Thank you for your interest in the **Server and Client Multiplatform** project! We hope you find it useful for your networking needs.