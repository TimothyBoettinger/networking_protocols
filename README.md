# Raw Ethernet Frame Parser (Layer 2)

A lightweight, educational C program designed to capture and inspect raw Ethernet frames directly from the network interface.

**Author:** Timothy Boettinger
**Date:** December 2025

## 📖 About The Project

I created this project to move beyond theoretical diagrams (like those found in CCNA or university lectures) and understand how data actually moves across the wire.

Instead of relying on standard networking headers (like `<net/ethernet.h>`), I manually defined the **IEEE 802.3 Ethernet Frame** structure. This approach required a deeper understanding of:
* **Raw Sockets:** Communicating directly with the Linux kernel to bypass the standard network stack.
* **Memory Mapping:** Using type-punning to overlay custom structs onto raw binary buffers.
* **Endianness:** Manually handling Network Byte Order (Big Endian) vs. Host Byte Order (Little Endian).

## ⚡ Features

* **Raw Packet Capture:** Uses `socket(AF_PACKET, SOCK_RAW, ...)` to sniff all traffic (`ETH_P_ALL`).
* **Manual Struct Definition:** Custom-built `MyEthernetHeader` struct to visualize memory alignment.
* **MAC Address Parsing:** Decodes and prints Source and Destination addresses in standard hex notation.
* **Protocol Identification:** Extracts and converts the EtherType field (e.g., IPv4, ARP, IPv6) using custom endianness logic.

## 🛠️ Getting Started

### Prerequisites
* **Linux Environment** (This program uses Linux-specific headers: `<sys/socket.h>`, `<netpacket/packet.h>`).
* **GCC Compiler**
* **Root Privileges** (Required to open raw sockets).

### Compilation
Clone the repository and compile the source code:

```bash
gcc snifferv3.c -o eth_parser
