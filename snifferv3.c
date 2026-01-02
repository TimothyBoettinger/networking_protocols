/* Program to demystify Ethernet Frames and Internet Protocols
 * Author: Timothy Boettinger
 * Date: 2025, December 29th
 *
 * I stuggled to grasp the concept of Ethernet Frames as presented
 * by the CCNA and various diagrams representing a rectangle/bar 
 *
 * By taking the time to analyze the structure and how a computer
 * typically processes these, I decided to write in C to gain 
 * further mastery with the additional hope of demystifying the
 * images required for me to understand.
 *
 * Thus I will attempt to document and thoroughly comment my 
 * assessments throughout this journey. 
 *
 * This is by no means an exhaustive list of issues that
 * I have come across, though serves as a window into the
 * thought process and questions answered along the way.
 *
 * My original program linked to useful libraries such as:
 *  - <net/ethernet.h> - for use of ETH_P_ALL
 *  - <arpa/inet.h> - of which I discovered the link to Legacy ARPANET
 * After reading about the implementation of these through manual pages:
 *  - man socket
 *  - man packet -> man 7 packet
 *  - man htons -> needed for protocol calls
 * It was quite magical to get all of this to work properly and see visually
 * the bytes being recieved to my device.
 *
 * For reference I am referring to the following information according to the 
 * IEEE 802.3 Ethernet Frame Format
 * - Ethernet Frame:
 *  - Preamble == 7 Bytes || 56 bits
 *  - Start of Frame Delimiter(SFD) == 1 Byte || 8 bits
 *      ~ Often these two are presented together as 8 Bytes || 64 bits
 *      ~ Considered a part of the preamble
 *  - Destination Address == 6 Bytes || 48 bits
 *      ~ Contains MAC Address of the machine where the data is 'destined'
 *  - Source Address == 6 Bytes || 48 bits
 *      ~ Contains MAC Address of the source machine.
 *      ~ Always Unicast
 *      ~ Least significant byte is == 0 ? Now is this big || little endian, depends on type of hardware
 *  - Length == 2 Bytes || 16 bits
 *      ~ being a 2 byte field this length can hold value between 0 and 65536 but cannot be larger
 *      ~ written in C as:
 *          ` unsigned = cannot be smaller than 0 therefore this option eliminates the (-) && is always (+)
 *          ` short = Though I am unfamiliar with this length designation, I have learned this to be useful in networking
 *                    designated a variable with 2 Bytes as opposed to char(1 Byte) || int(4 Bytes) || long(8 Bytes)
 *  - Data == 46 to 1500 Bytes || you do the math im not haha
 *  - Cyclic Redundency Check (CRC) == 2 Bytes || 16 bits
 *  Totalling 64 to 1518 Bytes including 'Data'
 */
#include <stdio.h>              // Standard library
#include <stdlib.h>             // Standard library
#include <sys/socket.h>         // Sockets and System Calls
#include <netpacket/packet.h>   // For Packets
#include <unistd.h>             // For close();

/* Protocol ID for 'all traffic' is defined as 0x0003 by
 * ETH_P_ALL
 * so I wished to research and define to avoid magical numbers
 */
#define MY_ETH_P_ALL 0x0300

/* Layer 2 - Ethernet Header */
/* Define a structure for Ethernet Headers, that I may see how they work in greater depth */
struct MyEthernetHeader {
    unsigned char dest[6];  // 6 Byte field that contains MAC Address information
    unsigned char src[6];   // 6 Byte field that contains 
    unsigned short proto;   // 2 Bytes for Protocol or length of
};                          // typedef struct is used more in 'modern' C but
                            // I have chosen struct as it allows me the
                            // opportunity to see its call in main()

/* To demystify Network to Host Short(ntohs)
 * visually see the function of swapping bytes
 */
unsigned short manNtohs(unsigned short v) {
    return (v << 8) | (v >> 8);
} 
/* EXPLAINING ENDIANNESS (The Transparent Paper Analogy)
 * Apparently CompSci 101 but I missed this part
 * Imagine the number 0x0800 written on a transparent sheet of glass.
 * The Network (Big Endian) writes it facing the front.
 * You see: [08] [00].
 *
 * My Intel CPU (Little Endian) stands BEHIND the glass.
 * It looks through the glass and reads the bytes in reverse order.
 * It sees: [00] [08].
 *
 * The function 'manNtohs' (Manual Network to Host Short) is essentially
 * flipping the glass over so the CPU can read it correctly.
 */

/* Main Program to view Ethernet Frame */
int main() {
    // It would have been too great a task to reimplement the socket(); call from <sys/socket.h> due to lack of Assembly Language knowledge
    // Therefore have kept in program, alongside recvfrom(); && eth
    int sockRaw = socket(AF_PACKET, SOCK_RAW, MY_ETH_P_ALL); 
    // socket(int domain, int type || socket_type, int protocol);
    // int domain = AF_PACKET 
    //  - The kernal is asking how to interpret the address
    // int type = SOCK_RAW
    //  - Can only be _RAW || _DGRAM
    //  - RAW is for raw packets including the linker level .h which is precisely my intention
    //  - No changes to packet data
    // int protocol = MY_ETH_P_ALL
    //  - I want to tell this program to capture all Ethernet Packets, regardless of protocol
    //  - Used my Macro to see this working and demonstrate its function
    //  Converted into single function call for ease of typing once it has been already laid out
    
    // Error Handling
    if (sockRaw < 0) {
        perror("Socket Error"); // prints the error(perror) instead of printf() as I wish to know what went wrong
        return 1;
    }

    // Allocate Memory
    unsigned char *buffer = (unsigned char *) malloc(65536);    // Theres that magic number again == 64Kb
    // Error Handling for Mem Alloc
    if (buffer == NULL) {
        perror("Memory Allocation Failed");
        return 1;
    }

    printf("Gathering Packets...\n");

    // While Loop so that program does not run once and consider itself complete
    while (1) {
        int dataSize = recvfrom(sockRaw, buffer, 65536, 0, NULL, NULL); 
        // to recieve a message from a socket
        // moreso to ask the kernel for data
        // recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
        // We have specified the sockfd or socket(); as int sockRaw previously
        // We have the buffer already noted and the magic number appears again
        // I want no flags, but will specify the remaining attributes below.

        // Error Handling for recvfrom();
        if (dataSize < 0) {
            perror("Recieved From Error");
            return 1;
        }
        
        // I need to point my struct to the buffer so exactly that
        struct MyEthernetHeader *eth = (struct MyEthernetHeader *)buffer;
        printf("Packet (%d bytes): ", dataSize);
        
        // Destination MAC Address is the first 6 bytes of an ethernet frame
        printf("Destination: |%02x:%02x:%02x:%02x:%02x:%02x| ", 
                eth->dest[0],
                eth->dest[1],
                eth->dest[2],
                eth->dest[3],
                eth->dest[4],
                eth->dest[5]);  // Since the above struct allocates 6 Bytes, I can print them
                                // %x tells the cpu to translate unsigned int to hexadecimal
                                // particularly 'x' = lowercase && 'X' = uppercase
                                // 02 is used to print in standard hexadecimal formatting
        
        printf("Source: |%02x:%02x:%02x:%02x:%02x:%02x| ",
                eth->src[0],
                eth->src[1],
                eth->src[2],
                eth->src[3],
                eth->src[4],
                eth->src[5]);   // Same process as above except for Source address instead of dest

        unsigned short protocol = manNtohs(eth->proto);
        printf("Protocol: |0x%04x|\n", protocol);

    }
    close(sockRaw);
    free(buffer);

    return 0;
}
