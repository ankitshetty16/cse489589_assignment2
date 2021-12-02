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

// /* function to send packet to respective destination */
// void send_Pkt(int seqNo,int ackNo,char* data,int flag)
// {
//     struct pkt* newPkt = new pkt();
//     cout<<"Entering send_Pkt"<<endl;
//     // pkt newPkt;
//     newPkt->seqnum = seqNo;
//     newPkt->acknum = ackNo;
//     strcpy(newPkt->payload,data);
//     newPkt->checksum = checkSum(newPkt);
//     cout<<"Entering tolayer3"<<endl;
//     tolayer3(flag,*newPkt);
//     cout<<"Leaving tolayer3"<<endl;
//     cout<<"Sent packet with seq no:"<<seqNo<<" data:";
//     for(int i=0;i<20;++i)
//     cout<<data[i];
//     cout<<"\n";
//     // starttimer(0,100.0);
//     cout<<"Leaving send_pkt"<<endl;
//     //delete(newPkt);
// }

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
#include<vector>
#include<iostream>
#include<stdio.h>
#include<string.h>

using namespace std;

std::vector<pkt> msgBufferA;
std::vector<pkt> pktBufferB;
pkt currentPacket;
#define FALSE 0
#define TRUE 1

int base;
int seqnum;
int acknum;
int nextSeqNoA = 1;
int nextSeqNoB = 1;
int WINDOWSIZE = 1;

// int currentWaitingAckNo = 0;
// int isReadyToSend = 0;
// int nextSeqNoB = 0;
// int currentAckNoB = 0;

/**/
int checkSum(struct pkt* packet){
  int sum = 0;
  for (int i = 0; i < 20;i++) {
      sum += packet->payload[i];
  }
  sum += packet->acknum;
  sum += packet->seqnum;
  cout << "CHECKSUM DATA -> FINAL SUM -> " << sum << endl;

  return sum;
 }

/**/
int checkACKSum(struct pkt* packet){
  int sum = 0;
  sum += packet->acknum;
  sum += packet->seqnum;
  
  return sum;
 }

/**/
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

/* Sending to layer 3*/
void pSend(int flag, struct pkt packet){
  cout << "PACKET DETAILS: " << "seqno > " << packet.seqnum << endl;
  cout << "PACKET DETAILS: " << "BASE > " << base << endl;
  cout << "PACKET DETAILS: " << "ackno > " << packet.acknum << endl;
  tolayer3(flag,packet);  
}

/* Send the next packet in Queue */
void sendNxtPkt(int flag) {
    struct pkt packet = msgBufferA.back();
    pSend(flag, packet);
    if(seqnum == base){
      // To start A's Timer
      cout << " ########################################################TIMER STARTED FOR A WITH SIM TIME >>>> " << get_sim_time() <<endl;
      starttimer(flag,get_sim_time());
    }
    // msgBufferA.pop_back();
}

/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message)
{
  cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>A_output TRIGGERED" << endl;
  cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> message >> " << message.data << endl;
  cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> seqnum >> " << seqnum << endl;
  cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> acknum >> " << acknum << endl;
  cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> WINDOW SIZE >> " << WINDOWSIZE << endl;
  cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Difference >> " << (seqnum - acknum) << endl;
  nextSeqNoA = nextSeqNoA + 1;
  // msgBufferA.insert(msgBufferA.begin(),*newPacket(seqnum,acknum,message.data));
  msgBufferA.push_back(*newPacket(seqnum,acknum,message.data));
  cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> BUFFER SIZE >> " << msgBufferA.size()  << endl;
  // && msgBufferA.size() > 0
  if((base - acknum) < WINDOWSIZE && msgBufferA.size() > 0){
    // send_Pkt(seqnum,seqnum,msgBufferA.back().data,0);
    cout << "************************* ************************* SENDING PACKET ************************* *************************" << endl;
    sendNxtPkt(0);
  }
  seqnum = seqnum + 1;
  cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>A_output ENDED" << endl;
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
  int evalCheckSum = checkACKSum(&packet);
  cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>A_input TRIGGERED" << endl;
  cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> RECEIVED ACK >> " << packet.acknum << endl;
  cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> RECEIVED SEQNUM >> " << packet.seqnum << endl;
  cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> RECEIVED nextSeqNoA >> " << nextSeqNoA << endl;
  cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> RECEIVED CHECKSUM >> " << packet.checksum << endl;
  cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> evalCheckSum CHECKSUM >> " << evalCheckSum << endl;
  cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> (seqnum - acknum) >> " << (seqnum - acknum) << endl;
  cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> msgBufferA.size() >> " << msgBufferA.size() << endl;
  if(packet.checksum == evalCheckSum){
    // if(packet.seqnum == nextSeqNoA){
      cout << "IN IF LOOP" << endl;
      stoptimer(0);
      nextSeqNoA += 1;
      base += 1;
      acknum = packet.acknum;

      //to remove packet from buffer for which acknowledgment has been received
      if (msgBufferA.size() > 0 && acknum == msgBufferA.begin()->seqnum){
        cout << "$$$$$$$$$$$$$$$$$$ I GOT IN $$$$$$$$$$$$$$$$$$" << endl;
        msgBufferA.erase(msgBufferA.begin());
      }

      if((base - acknum) < WINDOWSIZE && msgBufferA.size() > 0){
        cout << "TRIGGERED FROM A INPUT>>>>>>>>>>!@#$@%^&^*%^$#!@#*&(^&%$#@$%^&*%$#@!$%^$#@!%^&#@%$^%&^#@%$^%&^$#@%$^%&^#@" << endl;
        sendNxtPkt(0);
      }
      cout << "*************************************************************************** msgBufferA.size() >> " << msgBufferA.size() << endl;
      cout << ">>>>>>******>>>>>>>>>>>>******>>>>>> ACK HAS BEEN UPDATED TO >>>>>>******>>>>>> " << acknum << endl;
  }
  cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>A_input ENDED" << endl;
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
  cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>A_timerinterrupt TRIGGERED" << endl;
  // for(int i = 0; i < msgBufferA.size();i++){
  int restarted = 0;
  for(vector<pkt>::iterator resendPkt = msgBufferA.begin(); resendPkt != msgBufferA.end(); ++resendPkt){
    if((base - acknum) < WINDOWSIZE && msgBufferA.size() > 0){
      // send_Pkt(seqnum,seqnum,msgBufferA.back().data,0);
      cout << "************************* &&&&&&&&&&&&&&&&&&& RE--SENDING PACKET &&&&&&&&&&&&&&&&&&& *************************" << endl;
      pSend(0, *resendPkt);
      if (restarted == 0){
        stoptimer(0);
        starttimer(0,get_sim_time());
        restarted = 1;
      }
    } 
  }
  // ackpkttimlist.clear();
  // vector<pkt_tim>::iterator poke;
  // int i = 0;
  // for (poke = pkttimlist.begin(); poke != pkttimlist.end(); ++poke){
  //     struct pkt_tim* pkt_time = inSendingPacket(poke->seqnum);
  //     pkt_time->call_time = get_sim_time() + RTT + 2*i;
  //     i++;
  //     struct pkt* pkt_2 = timtoPacket(pkt_time);
  //     sendpkt(pkt_2);
  // }
  // starttimer(0,RTT);
  cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>A_timerinterrupt ENDED" << endl;

}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
  cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>A_INIT TRIGGERED" << endl;
  cout << ">>>>>>>>>>>>>>>>>>>>>>Testing init here" << endl;
  base = 1;
  seqnum = 1;
  acknum = 1;
  WINDOWSIZE= getwinsize();
  nextSeqNoA = 1;
  // currentWaitingAckNo = 0;
  msgBufferA.clear();
  // isReadyToSend = TRUE;

  cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>A_INIT ENDED" << endl;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
  cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>B_INPUT TRIGGERED" << endl;
  cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> " <<"nextSeqNoB >> " << nextSeqNoB << endl;
  cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> " <<"ACKNUM >> " << packet.acknum << endl;
  cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> " <<"SEQNUM >> " << packet.seqnum << endl;
  cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> " << "packet checksum >> "<< packet.checksum << endl;
  int cs = checkSum(&packet);
  cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> " << "CHECKSUM result >> "<< cs << endl;
  if (packet.checksum != cs) {
    cout << "RETURNED FROM HERE ITSELF" << endl;
    return;
  }
  if(packet.seqnum != nextSeqNoB){
    cout << "FIRST LOOP TRIGGERED WHERE VALUES NOT EQUAL" << endl;
    struct pkt *ack = generateACKPacket(packet.seqnum,1);
    tolayer3(1, *ack);
    // cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> " << "*ACK result >> "<< *ack << endl;
    return;
  }else{
    cout << "SECOND LOOP TRIGGERED WHERE COMMS TO 5 and 3 BOTH" << endl;
    tolayer5(1,packet.payload);
    struct pkt *ack = generateACKPacket(nextSeqNoB,1);
    tolayer3(1,*ack);
    // cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> " << "*ACK result >> "<< *ack << endl;
    nextSeqNoB += 1;
  }
  cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>B_INPUT ENDED" << endl;
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
  cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>B_INIT TRIGGERED" << endl;
  nextSeqNoB = 1;
  cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>B_INIT ENDED" << endl;
}
