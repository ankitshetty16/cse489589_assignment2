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

#include<vector>
#include<iostream>
#include<stdio.h>
#include<string.h>

using namespace std;

std::vector<msg> msgBufferA;
std::vector<pkt> pktBufferB;
pkt currentPacket;
#define FALSE 0
#define TRUE 1

int nextSeqNoA = 0;
int currentWaitingAckNo = 0;
int isReadyToSend = 0;
int nextSeqNoB = 0;
int currentAckNoB = 0;

void send_Pkt(int seqNo,int ackNo,char* data,int flag)
{
    //pkt* newPkt = new pkt();
    cout<<"Entering send_Pkt"<<endl;
    pkt newPkt;
    newPkt.seqnum = seqNo;
    newPkt.acknum = ackNo;
    strcpy(newPkt.payload,data);
    newPkt.checksum = 0;
    cout<<"Entering tolayer3"<<endl;
    tolayer3(flag,newPkt);
    cout<<"Leaving tolayer3"<<endl;
    cout<<"Sent packet with seq no:"<<seqNo<<" data:";
    for(int i=0;i<20;++i)
    cout<<data[i];
    cout<<"\n";
    starttimer(0,22.0);
    cout<<"Leaving send_pkt"<<endl;
    //delete(newPkt);
}

/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message)
{
    cout<<"Received a msg from layer 5 to be sent"<<endl;
    msgBufferA.push_back(message);
    if(isReadyToSend)
    {   
        cout<<"Sending a existing msg from the buffer"<<endl;
        send_Pkt(nextSeqNoA,nextSeqNoA,msgBufferA.front().data,0);
        currentWaitingAckNo = nextSeqNoA;
        nextSeqNoA = (nextSeqNoA + 1) % 2;
        isReadyToSend = FALSE;
        //starttimer(0,100.0);
    }
    else
    {
        cout<<"Buffer the msg as we are waiting for acknowledgement of the existing packet"<<endl;
    }

}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
    cout<<"Received a new packet from B to A with ack_no"<<packet.acknum<<endl;
    if(packet.acknum == currentWaitingAckNo)
    {
        cout<<"Received the ack for the right packet!!"<<endl;
        //tolayer5(0,packet.payload);
        cout<<"Size of vector before erase"<<msgBufferA.size()<<endl;
        msgBufferA.erase(msgBufferA.begin());
        cout<<"Size of vector after erase"<<msgBufferA.size()<<endl;
        stoptimer(0);
        isReadyToSend = TRUE;
        if(msgBufferA.size() > 0)
        {
            cout<<"Sending a existing msg from the buffer with data:"<<endl;
            for(int i =0;i<20;++i)
            cout<<msgBufferA.front().data[i];
            cout<<endl;
            send_Pkt(nextSeqNoA,nextSeqNoA,msgBufferA.front().data,0);
            currentWaitingAckNo = nextSeqNoA;
            nextSeqNoA = (nextSeqNoA + 1) % 2;
            isReadyToSend = FALSE;
            //starttimer(0,100.0);
        }
    }
    else
    {
        cout<<"Received the wrong acknowlegdement msg, hence resending the current pkt"<<endl;
        send_Pkt(currentWaitingAckNo,currentWaitingAckNo,msgBufferA.front().data,0);
        //currentWaitingAckNo = nextSeqNoA;
        nextSeqNoA = (currentWaitingAckNo + 1) % 2;
        isReadyToSend = FALSE;
        //starttimer(0,100.0);
    }
}

/* called when A's timer goes off */
void A_timerinterrupt()
{   
    stoptimer(0);
    cout<<"Entering Timer Interrupt"<<endl;
    cout<<"Timer expired for the packet with seq number:"<<currentWaitingAckNo<<endl;
    cout<<"Resending the packet back to B"<<endl;
    send_Pkt(currentWaitingAckNo,currentWaitingAckNo,msgBufferA.front().data,0);
    cout <<"nextSeqno="<<nextSeqNoA<<endl;
    nextSeqNoA = (currentWaitingAckNo + 1) % 2;
    cout <<"nextSeqno="<<nextSeqNoA<<endl;
    //currentWaitingAckNo = newPkt.seqnum;
    isReadyToSend = FALSE;
    //starttimer(0,100.0);
    cout<<"Leaving Timer Interrupt"<<endl;
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
  nextSeqNoA = 0;
  currentWaitingAckNo = 0;
  msgBufferA.clear();
  isReadyToSend = TRUE;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
    cout<<"Packet recieved at B"<<endl;
    pkt ackPkt;
    memset(&ackPkt,0,sizeof(pkt));
    if(packet.seqnum != nextSeqNoB)
    {
        ackPkt.seqnum = nextSeqNoB;
        ackPkt.acknum = nextSeqNoB;
        ackPkt.checksum = 0;
        tolayer3(1,ackPkt);
    }
    else
    {
        ackPkt.seqnum = nextSeqNoB;
        ackPkt.acknum = packet.seqnum;
        tolayer5(1,packet.payload);
        tolayer3(1,ackPkt);
        currentAckNoB = packet.seqnum;
        nextSeqNoB = (nextSeqNoB + 1) % 2;

    }

}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
  //NextSeqNoB = 0;
  currentAckNoB = 0;
}
