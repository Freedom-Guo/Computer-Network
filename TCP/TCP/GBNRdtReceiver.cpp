#include "stdafx.h"
#include "Global.h"
#include "GBNRdtReceiver.h"

int flag[3] = { 0 };
GBNRdtReceiver::GBNRdtReceiver():expectSequenceNumberRcvd(0)
{
	lastAckPkt.acknum = -1; //初始状态下，上次发送的确认包的确认序号为-1，使得当第一个接受的数据包出错时该确认报文的确认号为-1
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1;	//忽略该字段
	for(int i = 0; i < Configuration::PAYLOAD_SIZE;i++){
		lastAckPkt.payload[i] = '.';
	}
	lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
}


GBNRdtReceiver::~GBNRdtReceiver()
{
}

bool isOK(int base, int seq)
{
	if (seq == (base + 1) % 8 ||
		seq == (base + 2) % 8 ||
		seq == (base + 3) % 8 )
		return true;
	else
		return false;
}

void GBNRdtReceiver::receive(Packet &packet) {
	//检查校验和是否正确
	pUtils->printPacket("接收方收到发送方的报文", packet);
	int checkSum = pUtils->calculateCheckSum(packet);
	if (checkSum == packet.checksum) 
	{
		
		if (this->expectSequenceNumberRcvd == packet.seqnum)
		{
			pUtils->printPacket("接收方正确收到发送方的报文", packet);
			Message msg;
			memcpy(msg.data, packet.payload, sizeof(packet.payload));
			pns->delivertoAppLayer(RECEIVER, msg);
			int i = 0;
			int count = 1;//本次输出几个
			while (i < 3 && flag[i])//输出连续缓冲区内容
			{
				memcpy(msg.data, buf[i].payload, sizeof(buf[i].payload));
				pns->delivertoAppLayer(RECEIVER, msg);
				flag[i] = 0;
				i++;
				count++;
			}
			i--;
			if (i == -1)//缓冲区无内容
			{
				lastAckPkt.acknum = (packet.seqnum + 1) % 8;
				lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
				this->expectSequenceNumberRcvd = packet.seqnum + 1;
				expectSequenceNumberRcvd %= 8;
			}
			else
			{
				lastAckPkt.acknum = (buf[i].seqnum + 1) % 8;
				lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
				this->expectSequenceNumberRcvd = buf[i].seqnum + 1;
				expectSequenceNumberRcvd %= 8;
			}
			pUtils->printPacket("缓存区已发送，接收方发送新的确认报文", lastAckPkt);
			pns->sendToNetworkLayer(SENDER, lastAckPkt);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送上次的确认报文
			//for (i = 0;i < 3 && flag[i] == 0;i++)//此处有误，比如要J，但是来了M，即* * M，然后J来之后，i=2，即向左移两格
			//	;                                //实际只应移一格
			int j;
			for (j = 0;j < 3-count ;j++)
			{
				flag[j] = flag[j +count];
				buf[j].acknum = buf[j + count].acknum;
				buf[j].seqnum = buf[j + count].seqnum;
				buf[j].checksum = buf[j + count].checksum;
				memcpy(buf[j].payload, buf[j + count].payload, sizeof(buf[j + count].payload));
			}
			for ( ;j < 3;j++)
				flag[j] = 0;
			
			
		}
		else if(isOK(expectSequenceNumberRcvd,packet.seqnum))
		{
			int buf_num = (packet.seqnum - expectSequenceNumberRcvd+8)%8 - 1;
			buf[buf_num].acknum = packet.acknum;
			buf[buf_num].checksum = packet.checksum;
			buf[buf_num].seqnum = packet.seqnum;
			memcpy(buf[buf_num].payload, packet.payload, sizeof(packet.payload));
			flag[buf_num] = 1;
			pUtils->printPacket("已缓存，接收方重新发送上次的确认报文", lastAckPkt);
			pns->sendToNetworkLayer(SENDER, lastAckPkt);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文到对方
		}
		else 
		{
			pUtils->printPacket("即非所需也非缓存内容，接收方重新发送上次的确认报文", lastAckPkt);
			pns->sendToNetworkLayer(SENDER, lastAckPkt);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送上次的确认报文
		}
	}
	else
	{
		pUtils->printPacket("校验和错误，接收方重新发送上次的确认报文", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送上次的确认报文
	}
}