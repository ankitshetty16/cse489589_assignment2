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

std::vector<pkt> msgBufferA;
std::vector<pkt> msgQueue;
#define FALSE 0
#define TRUE 1

int base;
int seqnum;
int acknum;
int nextSeqNoA = 1;
int nextSeqNoB = 1;
int WINDOWSIZE = 1;
float RTT = 10000.0;
int timerStarted = 0;

/* To calculate checkSum for packet*/
int checkSum(struct pkt* packet){
  int sum = 0;
  for (int i = 0; i < 20;i++) {
      sum += packet->payload[i];
  }
  sum += packet->acknum;
  sum += packet->seqnum;

  return sum;
 }

/* To calculate checkSum for ACK packet*/
int checkACKSum(struct pkt* packet){
  int sum = 0;
  sum += packet->acknum;
  sum += packet->seqnum;
  
  return sum;
 }

/*To generate ACK packet*/
static pkt* generateACKPacket(int seqnum,int acknum){
    struct pkt* packet = new pkt();
    packet->seqnum = seqnum;
    packet->acknum = seqnum;
    packet->checksum = seqnum + seqnum;
    memset(packet->payload,0,sizeof(packet->payload));

    return packet;
}

/*Create a new packet*/
static pkt* newPacket(int seqnum,int acknum, char* data){ 
    struct pkt* packet = new pkt();
    packet->acknum = acknum;
    packet->seqnum = seqnum;
    strncpy(packet->payload, data, 20);
    packet->checksum = checkSum(packet);

    return packet;
}

/* Sending packet to layer 3*/
void pSend(int flag, struct pkt packet){
  tolayer3(flag,packet);
}

/* Send the next packet in Queue */
void sendNxtPkt(int flag) {
    struct pkt packet = msgQueue.front();
    cout << "Sending packet seq -> " << packet.seqnum << endl;
    pSend(flag, packet);
    msgQueue.erase(msgQueue.begin());
    // if(seqnum == base){
      if(timerStarted == 0) {
      // To start A's Timer
      cout << " ########################################################TIMER STARTED FOR A WITH SIM TIME >>>> " << RTT <<endl;
      starttimer(flag,RTT);
      timerStarted = 1;
    }
}

/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message)
{
  msgBufferA.push_back(*newPacket(seqnum,acknum,message.data));
  msgQueue.push_back(*newPacket(seqnum,acknum,message.data));
  if((nextSeqNoA - base) < WINDOWSIZE && msgQueue.size() > 0){
    cout << "Packet sent: " << seqnum << endl;
    sendNxtPkt(0);
    nextSeqNoA = nextSeqNoA + 1;
  }
  seqnum = seqnum + 1; 
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
  int evalCheckSum = checkACKSum(&packet);
  if(packet.checksum == evalCheckSum){
    // if(packet.seqnum == nextSeqNoA){
      // cout << "IN IF LOOP" << endl;
      acknum = packet.acknum;
      //to remove packet from buffer for which acknowledgment has been received
      if (msgBufferA.size() > 0 && acknum == msgBufferA.begin()->seqnum){
        cout << "REMOVE FROM BUFFER " << endl;
        stoptimer(0);
        timerStarted = 0;
        base += 1;
        cout << "ACK RECIEVED: " << packet.acknum << endl;
        msgBufferA.erase(msgBufferA.begin());
        if((nextSeqNoA - base) < WINDOWSIZE){
          cout << "Next Packet Sent : " << endl;
          if (msgQueue.size() > 0) {
            sendNxtPkt(0);
          };
          if(timerStarted == 0){
            starttimer(0,RTT);
            timerStarted = 1;
            cout << "NEXT -----------------------------------------------TIME RESTARTED-----------------------------------------------" << endl;
          }
        }
      }
  }
  cout << "-----------------------------------------------------------------------------------------------------------------------------------------------------------" << endl;
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
  cout << "TIMER INTERRUPT" << endl;
  int restarted = 0;
  nextSeqNoA = base;
  msgQueue.clear();
  msgQueue = msgBufferA;
  for(vector<pkt>::iterator resendPkt = msgQueue.begin(); resendPkt != msgQueue.end();){
    if((nextSeqNoA - base) < WINDOWSIZE && msgQueue.size() > 0){
      cout << "Resending packet with seq num -> " << resendPkt->seqnum << endl;
      // sendNxtPkt(0);
      pSend(0, *resendPkt);
      msgQueue.erase(msgQueue.begin());
      nextSeqNoA = nextSeqNoA + 1;
      if (restarted == 0){
        cout << "-----------------------------------------------TIME RESTARTED-----------------------------------------------" << endl;
        starttimer(0,RTT);
        timerStarted = 1;
        restarted = 1;
      }
    }else {
       ++resendPkt;
    }
  }
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
  cout << "A Initialised!" << endl;
  base = 1;
  seqnum = 1;
  acknum = 1;
  WINDOWSIZE= getwinsize();
  nextSeqNoA = 1;
  msgBufferA.clear();
  msgQueue.clear();
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
  cout << ">>>>>>>>>>>> RECIEVED AT B >> " <<"SEQNUM >> " << packet.seqnum << endl;
  int cs = checkSum(&packet);
  if (packet.checksum != cs) {
    // cout << "RETURNED FROM HERE ITSELF" << endl;
    return;
  }
  if(packet.seqnum != nextSeqNoB){
    cout << "FIRST LOOP TRIGGERED WHERE VALUES NOT EQUAL" << endl;
    struct pkt *ack = generateACKPacket(packet.seqnum,1);
    tolayer3(1, *ack);
    return;
  }else{
    cout << "SECOND LOOP TRIGGERED WHERE COMMS TO 5 and 3 BOTH" << endl;
    tolayer5(1,packet.payload);
    struct pkt *ack = generateACKPacket(nextSeqNoB,1);
    tolayer3(1,*ack);
    nextSeqNoB += 1;
  }
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
  cout << "B Initialised!" << endl;
  nextSeqNoB = 1;
}
