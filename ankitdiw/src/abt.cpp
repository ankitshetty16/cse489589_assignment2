   
#include "../include/simulator.h"

/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose
   This code should be used for PA2, unidirectional data transfer 
   protocols (from A to B). Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

#include <iostream>
#include <stdio.h>
#include <queue>
#include <string.h>

using namespace std;
#define Time_out 20.0
#define FALSE 0
#define TRUE 1

std::queue<std::string> msgBuffer;

struct Sender
{
    int nextSeqNo;
    int nextAckNo;
    int isReady;
}aInfo;

struct Receiver
{
    int currAckNo;
}bInfo;

int compute_checksum(struct pkt *packet)
{   
    cout<<"Entering checksum"<<endl;
    int checksum = 0;
    checksum += packet->seqnum;
    checksum += packet->acknum;
    for(int i=0;i<20;++i)
    {
        checksum += packet->payload[i]; 
    }
    cout<<"Leaving checksum"<<endl;
    return ~checksum;
}

/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message)
{   
    msgBuffer.push(message.data);
    if(aInfo.isReady == FALSE)
    {
        cout<<"Waiting for acknowledgement of the correct packet, hence adding to message buffer"<<endl;
        return;
    }
    else
    {
        cout<<"Sending the current message"<<endl;
        pkt packet;
        packet.seqnum = aInfo.nextSeqNo;
        packet.acknum = aInfo.nextAckNo;
        memcpy(packet.payload,message.data,sizeof(message.data));
        packet.checksum = compute_checksum(&packet);
        cout<<"Sending from A:\n"<<"msg:"<<message.data<<"\n"<<"Seq Number:"<<aInfo.nextSeqNo<<"\nAck number:"<<aInfo.nextAckNo<<endl;
        tolayer3(0,packet);
        aInfo.isReady = FALSE;
        starttimer(0,Time_out);
    }
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
    cout<<"Ack packet received from B with ack number:"<<packet.acknum<<endl;
    if((packet.checksum != compute_checksum(&packet)) || (packet.acknum != aInfo.nextAckNo))
    {
        cout<<"Packet is corrupted/has wrong ackno, waiting ack no is: "<<aInfo.nextAckNo<<endl;
        return;
    }

    cout<<"Right acknowledgement received without corruption and right ack no:"<<aInfo.nextAckNo<<endl;
    stoptimer(0); 
    msgBuffer.pop();
    aInfo.isReady = TRUE;
    aInfo.nextSeqNo = (aInfo.nextSeqNo + 1) % 2;
    aInfo.nextAckNo = (aInfo.nextAckNo + 1) % 2;
    if(msgBuffer.size() > 0)
    {   
        cout<<"Messages has been queued in Buffer, sending that now"<<endl;
        pkt packet;
        packet.seqnum = aInfo.nextSeqNo;
        packet.acknum = aInfo.nextAckNo;
        string msg = msgBuffer.front();
        strncpy(packet.payload,msg.c_str(),20);
        packet.checksum = compute_checksum(&packet);
        cout<<"Sending from A:\n"<<"msg:"<<msg<<"\n"<<"Seq Number:"<<aInfo.nextSeqNo<<"\nAck number:"<<aInfo.nextAckNo<<endl;
        tolayer3(0,packet);
        aInfo.isReady = FALSE;
        starttimer(0,Time_out);
    }
    cout<<"Leaving A_input"<<endl;
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
    cout<<"Timer has expired for the current packet with Seq number:"<<aInfo.nextSeqNo<<" Hence resending the packet"<<endl;
    pkt packet;
    packet.seqnum = aInfo.nextSeqNo;
    packet.acknum = aInfo.nextAckNo;
    string msg = msgBuffer.front();
    strncpy(packet.payload,msg.c_str(),20);
    packet.checksum = compute_checksum(&packet);
    cout<<"Sending from A:\n"<<"msg:"<<packet.payload<<"\n"<<"Seq Number:"<<aInfo.nextSeqNo<<"\nAck number:"<<aInfo.nextAckNo<<endl;
    tolayer3(0,packet);
    aInfo.isReady = FALSE;
    starttimer(0,Time_out);
    cout<<"Leaving timerinterrupt"<<endl;
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
    aInfo.nextSeqNo = 0;
    aInfo.nextAckNo = 0;
    aInfo.isReady = TRUE;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
    //3 cases:
    //case 1: perfect pkt, not received before, transmit ack and call tolayer5()
    //case 2: perfect pkt, but received before, transmit ack but don't deliver to upper layer
    if (packet.checksum == compute_checksum(&packet)) 
    {
        cout << "Received packet with payload " << packet.payload << ", seq=" << packet.seqnum << " ack=" << packet.acknum << endl;
        pkt sendPacket;
        if (packet.seqnum == bInfo.currAckNo) 
        {
            tolayer5(1, packet.payload);
            sendPacket.acknum = packet.seqnum;
            sendPacket.checksum = compute_checksum(&sendPacket);
            tolayer3(1, sendPacket);
            bInfo.currAckNo = (bInfo.currAckNo + 1) % 2;
            cout << "Send the ack packet: " << "ack=" << sendPacket.acknum << " next expected seq number:" << bInfo.currAckNo << endl;
        } 
        else {
            sendPacket.acknum = packet.seqnum;
            sendPacket.checksum = compute_checksum(&sendPacket);
            cout << "Response to sender: " << "ack=" << sendPacket.acknum << " next expected seq=" << bInfo.currAckNo << endl;
            tolayer3(1, sendPacket);
        }
    } 
    else 
    {
        cout << "Packet received is corrupted!!!!!" << endl;
    }
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
    bInfo.currAckNo = 0;
}