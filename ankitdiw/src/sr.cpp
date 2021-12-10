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
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <cstring>
#include <vector>
#include <queue>
using namespace std;

int next_seqnum;
int next_acknum;
int base;
int recv_seqnum;

//struct for Timer, creating instance
struct pkt_timer{
  struct pkt content;
  float start_time;
};

vector <pkt_timer> window_A;
vector <pkt> window_B;
queue <pkt> msg_queue;
int WIN_size;
int B_base;
float RTT= 100.00;

//Checksum
int checkSum(struct pkt* packet){
  int sum = 0;
  for (int i = 0; i < 20;i++) {
      sum += packet->payload[i];
  }
  sum += packet->acknum;
  sum += packet->seqnum;
  cout << "CHECKSUM DATA -> FINAL SUM -> " << sum << endl;

  return ~sum;
}

//Check acksum

int checkACKSum(struct pkt* packet){
  int sum = 0;
  sum += packet->acknum;
  sum += packet->seqnum;

  return ~sum;
 }

void find_pkt_window(vector<pkt_timer> *p, pkt packet){

  for(vector<pkt_timer>::iterator i= p->begin();i != p->end();++i){
      if(i->content.seqnum == packet.seqnum){
        window_A.erase(i);
        break;
      }
  }

} 

void sendNxtpkt(int flag, struct pkt packet) {
  tolayer3(flag, packet);
  cout << " send tolayer3" << endl;
      // To start A's Timer
  cout << "TIMER STARTED FOR A WITH SIM TIME >>>> " << get_sim_time() <<endl;
  starttimer(flag,RTT);
  // starttimer(flag,get_sim_time());
    
}

// Creating a Packet
static pkt* newPacket(int seqnum,int acknum, char* data){ 
    struct pkt* packet = new pkt();
    packet->acknum = acknum;
    packet->seqnum = seqnum;
    strncpy(packet->payload, data, 20);
    packet->checksum = checkSum(packet);
    return packet;
}

/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message)
{
  struct pkt next_pkt;
  cout << "A output started>>>" << endl;
  next_pkt=*newPacket(next_seqnum,0, message.data);
  struct pkt_timer buffer_pkt;
  if(next_seqnum - base < WIN_size){
    cout << " Sending packet seqnum >> " << next_seqnum << endl;
    sendNxtpkt(0, next_pkt);
    cout <<"##A_output## >>> After send pkt" <<endl;
    buffer_pkt.content= next_pkt;
    buffer_pkt.start_time = get_sim_time();
    window_A.push_back(buffer_pkt);
    cout<<"##A_output## >>> After Pushback!"<<endl;
    next_seqnum++;
  }
  else{
    cout<< "##A_output## >>>  Wait and load it in waiting que " <<endl;
    msg_queue.push(next_pkt);
    next_seqnum++;
  }
  cout << "A output ended >>>> " << endl;
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
  cout<<"##A_input## >>> INSIDE A_INPUT "<<endl;
  int evalAckkSum = checkSum(&packet);
  if(packet.checksum == evalAckkSum){
    find_pkt_window(&window_A, packet);
    recv_seqnum = max(recv_seqnum,packet.seqnum);
    if (packet.seqnum == base){
      stoptimer(0);
      int min_seqnum=10000;
      int max_seqnum=0;
      if (window_A.size()==0){
        base = recv_seqnum +1;
        max_seqnum= recv_seqnum;
      }else {
          for(vector<pkt_timer>::iterator i = window_A.begin();i != window_A.end();++i){
            min_seqnum = min(min_seqnum,i->content.seqnum);
            max_seqnum = max(max_seqnum,i->content.seqnum);
          }
          base = min_seqnum;
      }
      while (msg_queue.size()>0 && max_seqnum - base < WIN_size){
        cout << "##A_input## >>>Inside while condition to send the msg queue....." << endl;
        struct pkt buffer_A;
        buffer_A = msg_queue.front();
        msg_queue.pop();
        sendNxtpkt(0, buffer_A);
        struct pkt_timer buffer_pkt;
        buffer_pkt.content = buffer_A;
        buffer_pkt.start_time = get_sim_time();
        window_A.push_back(buffer_pkt);
        max_seqnum = buffer_A.seqnum;

      }

    }
  }

}

/* called when A's timer goes off */
void A_timerinterrupt(){
cout << "A interrupt started" <<  endl;
int restarted = 0;
int curr_timer=0;
struct pkt_timer resendPkt;
resendPkt = window_A[0];
if ((base - next_acknum)<WIN_size && window_A.size()>0){
  stoptimer(0);
  curr_timer = get_sim_time();
  starttimer(0, curr_timer);
  resendPkt.start_time = curr_timer;
  tolayer3(0, resendPkt.content); 
  window_A.push_back(resendPkt);
  window_A.erase(window_A.begin());
}
cout << "A interrupt ended" <<  endl;
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
  WIN_size = getwinsize();
  base = 1;
  next_seqnum = 1;
  recv_seqnum =0;
  struct pkt temp;
  for(int i = 0;i <= WIN_size;++i){ 
    window_B.push_back(temp);
  }
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
  cout<<"##B_input## >>> ENTERED "<<endl;

  int eval_checksum= checkSum(&packet);
  if(packet.checksum == eval_checksum){
  struct pkt buffer_B;
  struct pkt temp;
  struct msg message;
  temp.acknum=0;
  buffer_B= *newPacket(packet.seqnum,0, packet.payload);
  sendNxtpkt(1, buffer_B);
  if(packet.seqnum==B_base){
    packet.acknum=1;
    window_B[0]=packet;
    while(window_B.size()>0 && window_B[0].acknum==1){
      strcpy(message.data,window_B[0].payload);
      window_B.erase(window_B.begin());
      window_B.push_back(temp);
      tolayer5(1,message.data);
      cout<<"##B_input## >>> MESSAGE SENT TO LAYER 5, SUCCESS!!!!"<<endl;
      B_base++;
    }
  }
  else if(packet.seqnum>B_base){
    if((packet.seqnum - B_base) <  + WIN_size){
      packet.acknum = 1;
      window_B[packet.seqnum - B_base] = packet;
    }
  }
  
  }
  
 cout << "B INPUT ended" << endl;

}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
  B_base = 1;
}