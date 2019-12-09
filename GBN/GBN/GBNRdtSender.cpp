#include "stdafx.h"
#include "Global.h"
#include "GBNRdtSender.h"


GBNRdtSender::GBNRdtSender():nextseqnum(0),base(0),waitingState(false)
{
}


GBNRdtSender::~GBNRdtSender()
{
}



bool GBNRdtSender::getWaitingState() {
	return (nextseqnum - base + 8) % 8 >= N;;
}




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

void GBNRdtSender::receive(Packet &ackPkt) {
	int checkSum = pUtils->calculateCheckSum(ackPkt);

	//如果校验和正确，并且确认序号=发送方已发送并等待确认的数据包序号
	if (checkSum == ackPkt.checksum) 
	{
		int LenthToMove = (ackPkt.acknum - base + 1 + 8) % 8;
		cout << "\n---》移动窗口前内容：\t";
		for (int i = 0;i < (nextseqnum - base + 8) % 8;i++)
			cout << packetWaitingAck[i].seqnum << ' ';
		for (int i = 0;i < 4 - LenthToMove;i++)//移动窗口，为窗口腾出位置
			packetWaitingAck[i] = packetWaitingAck[i + LenthToMove];
		base = ackPkt.acknum + 1;
		base %= 8;
		cout << "\n---》移动窗口后内容：\t";
		for (int i = 0;i < (nextseqnum - base + 8) % 8;i++)
			cout << packetWaitingAck[i].seqnum << ' ';
		this->waitingState = false;
		pns->stopTimer(SENDER, 0);
		if (base == nextseqnum)
			;
		else
			pns->startTimer(SENDER, Configuration::TIME_OUT, 0);		//关闭定时器
		pUtils->printPacket("发送方正确收到确认", ackPkt);
	}
}

void GBNRdtSender::timeoutHandler(int seqNum) {
	pns->stopTimer(SENDER, seqNum);										//首先关闭定时器
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);	//重新启动发送方定时器
	for (int i = 0;i < (nextseqnum - base + 8) % 8;i++)
	{
		pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[i]);
		pUtils->printPacket("发送方定时器时间到，重发上次发送的报文", this->packetWaitingAck[i]);
	}
}
