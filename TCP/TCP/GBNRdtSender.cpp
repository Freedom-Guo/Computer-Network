#include "stdafx.h"
#include "Global.h"
#include "GBNRdtSender.h"


GBNRdtSender::GBNRdtSender():nextseqnum(0),base(0),waitingState(false)
{
	for (int i = 0;i < 8;i++)
		AckCount[i] = 0;
}


GBNRdtSender::~GBNRdtSender()
{
}



bool GBNRdtSender::getWaitingState() {
	return (nextseqnum - base + 8) % 8 >= N;//直接返回窗口判断，不然会出现上层文本往后移了一格但并未发送的状况
}                                           //就是少发F之类的




bool GBNRdtSender::send(Message &message) {
	if ((nextseqnum - base + 8) % 8 < N)
	{
		int wndbase = (nextseqnum - base + 8) % 8;
		this->packetWaitingAck[wndbase].acknum = -1;
		this->packetWaitingAck[wndbase].seqnum = this->nextseqnum;
		this->packetWaitingAck[wndbase].checksum = 0;
		memcpy(this->packetWaitingAck[wndbase].payload, message.data, sizeof(message.data));
		this->packetWaitingAck[wndbase].checksum = pUtils->calculateCheckSum(this->packetWaitingAck[wndbase]);

		pUtils->printPacket("发送方发送报文", this->packetWaitingAck[wndbase]);
		pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[wndbase]);

		if(base==nextseqnum)
			pns->startTimer(SENDER, Configuration::TIME_OUT, 0);
		nextseqnum++;
		nextseqnum %= 8;
		this->waitingState = false;//窗口未满
		return true;
	}
	else
	{
		this->waitingState = true;
		return false;
	}
}
bool isRepeat(int base, int seq)
{
	if (seq == (base + 1) % 8 ||
		seq == (base + 2) % 8 ||
		seq == (base + 3) % 8 ||
		seq == (base + 4) % 8)
		return false;
	else
		return true;
}
void GBNRdtSender::receive(Packet &ackPkt) {
	int checkSum = pUtils->calculateCheckSum(ackPkt);

	//如果校验和正确，并且确认序号=发送方已发送并等待确认的数据包序号
	if (checkSum == ackPkt.checksum) 
	{
		if (!isRepeat(base, ackPkt.acknum))
		{
			int LenthToMove = (ackPkt.acknum - base + 8) % 8;
			cout << "---》移动窗口前内容：\t";
			for (int i = 0;i < (nextseqnum - base + 8) % 8;i++)
				cout << packetWaitingAck[i].seqnum<<' ';
			cout << endl;
			for (int i = 0;i < 4 - LenthToMove;i++)//移动窗口，为窗口腾出位置
				packetWaitingAck[i] = packetWaitingAck[i + LenthToMove];
			base = ackPkt.acknum % 8;
			cout << "---》移动窗口后内容：\t";
			for (int i = 0;i < (nextseqnum - base + 8) % 8;i++)
				cout << packetWaitingAck[i].seqnum<<' ';
			cout << endl;
			this->waitingState = false;
			pns->stopTimer(SENDER, 0);
			if (base == nextseqnum)
				;
			else
				pns->startTimer(SENDER, Configuration::TIME_OUT, 0);		//关闭定时器
			pUtils->printPacket("发送方正确收到确认", ackPkt);
		}
		else
		{
			AckCount[ackPkt.acknum]++;
			if (AckCount[ackPkt.acknum] == 3)//应该是4，但是为了方便改成3
			{
				for (int i = 0;i < 4;i++)
					if (packetWaitingAck[i].seqnum == ackPkt.acknum)
					{
						pns->sendToNetworkLayer(RECEIVER, packetWaitingAck[i]);
						cout << "---》快速重传:\t" << ackPkt.acknum << endl;
					}
				AckCount[ackPkt.acknum] = 0;
			}
		}

	}
}

void GBNRdtSender::timeoutHandler(int seqNum) {
	//if (base != nextseqnum) 
	//{
		pns->stopTimer(SENDER, 0);										//首先关闭定时器
		pns->startTimer(SENDER, Configuration::TIME_OUT, 0);	//重新启动发送方定时器
		pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[0]);
		pUtils->printPacket("发送方定时器时间到，重发最早还未确认的报文", this->packetWaitingAck[0]);
	//}
	
}
