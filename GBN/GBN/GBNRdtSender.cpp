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

		pUtils->printPacket("���ͷ����ͱ���", this->packetWaitingAck[wndbase]);
		pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[wndbase]);

		if(base==nextseqnum)
			pns->startTimer(SENDER, Configuration::TIME_OUT, 0);
		nextseqnum++;
		nextseqnum %= 8;
		this->waitingState = false;//����δ��
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

	//���У�����ȷ������ȷ�����=���ͷ��ѷ��Ͳ��ȴ�ȷ�ϵ����ݰ����
	if (checkSum == ackPkt.checksum) 
	{
		int LenthToMove = (ackPkt.acknum - base + 1 + 8) % 8;
		cout << "\n---���ƶ�����ǰ���ݣ�\t";
		for (int i = 0;i < (nextseqnum - base + 8) % 8;i++)
			cout << packetWaitingAck[i].seqnum << ' ';
		for (int i = 0;i < 4 - LenthToMove;i++)//�ƶ����ڣ�Ϊ�����ڳ�λ��
			packetWaitingAck[i] = packetWaitingAck[i + LenthToMove];
		base = ackPkt.acknum + 1;
		base %= 8;
		cout << "\n---���ƶ����ں����ݣ�\t";
		for (int i = 0;i < (nextseqnum - base + 8) % 8;i++)
			cout << packetWaitingAck[i].seqnum << ' ';
		this->waitingState = false;
		pns->stopTimer(SENDER, 0);
		if (base == nextseqnum)
			;
		else
			pns->startTimer(SENDER, Configuration::TIME_OUT, 0);		//�رն�ʱ��
		pUtils->printPacket("���ͷ���ȷ�յ�ȷ��", ackPkt);
	}
}

void GBNRdtSender::timeoutHandler(int seqNum) {
	pns->stopTimer(SENDER, seqNum);										//���ȹرն�ʱ��
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);	//�����������ͷ���ʱ��
	for (int i = 0;i < (nextseqnum - base + 8) % 8;i++)
	{
		pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[i]);
		pUtils->printPacket("���ͷ���ʱ��ʱ�䵽���ط��ϴη��͵ı���", this->packetWaitingAck[i]);
	}
}
