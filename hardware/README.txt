
Flutter Code Plan
==

9 bit address


packet format:
[length:8/256][sender:9/512][recipient:9/512][sender's index:5/32][packetcode:6/64][data bytes][system][RSSI]

Sender's index is


Main Arduino Code
==
setup()
loop()


Flutter.h
===

packetWait(timeout) //wait for packet, return packet index or -1. -1 indicates error. Optional lastPacketError() returns timeout or packet index. returned packet index is saved contents of bad CRC packet.



CC1200.h
===

packetStatus = packet info bitpacked

ReadData()
load data from FIFO

WriteData()
Write data to FIFO and send

send packet


Round Robin packet buffer stores packet information (bitpacked data with: packetnum, status, sendtime ms). Smaller buffer stores recieved packets. Send temp buffer

packets to send get queued up to TX_QUEUE number of packets.
"process radio" runs and will first recieve any packets to the RX buffer, then


Set Registers
Read Registers