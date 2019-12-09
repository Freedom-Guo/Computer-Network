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
	return (nextseqnum - base + 8) % 8 >= N;//ֱ�ӷ��ش����жϣ���Ȼ������ϲ��ı���������һ�񵫲�δ���͵�״��
}                                           //�����ٷ�F֮���




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

	//���У�����ȷ������ȷ�����=���ͷ��ѷ��Ͳ��ȴ�ȷ�ϵ����ݰ����
	if (checkSum == ackPkt.checksum) 
	{
		if (!isRepeat(base, ackPkt.acknum))
		{
			int LenthToMove = (ackPkt.acknum - base + 8) % 8;
			cout << "---���ƶ�����ǰ���ݣ�\t";
			for (int i = 0;i < (nextseqnum - base + 8) % 8;i++)
				cout << packetWaitingAck[i].seqnum<<' ';
			cout << endl;
			for (int i = 0;i < 4 - LenthToMove;i++)//�ƶ����ڣ�Ϊ�����ڳ�λ��
				packetWaitingAck[i] = packetWaitingAck[i + LenthToMove];
			base = ackPkt.acknum % 8;
			cout << "---���ƶ����ں����ݣ�\t";
			for (int i = 0;i < (nextseqnum - base + 8) % 8;i++)
				cout << packetWaitingAck[i].seqnum<<' ';
			cout << endl;
			this->waitingState = false;
			pns->stopTimer(SENDER, 0);
			if (base == nextseqnum)
				;
			else
				pns->startTimer(SENDER, Configuration::TIME_OUT, 0);		//�رն�ʱ��
			pUtils->printPacket("���ͷ���ȷ�յ�ȷ��", ackPkt);
		}
		else
		{
			AckCount[ackPkt.acknum]++;
			if (AckCount[ackPkt.acknum] == 3)//Ӧ����4������Ϊ�˷���ĳ�3
			{
				for (int i = 0;i < 4;i++)
					if (packetWaitingAck[i].seqnum == ackPkt.acknum)
					{
						pns->sendToNetworkLayer(RECEIVER, packetWaitingAck[i]);
						cout << "---�������ش�:\t" << ackPkt.acknum << endl;
					}
				AckCount[ackPkt.acknum] = 0;
			}
		}

	}
}

void GBNRdtSender::timeoutHandler(int seqNum) {
	//if (base != nextseqnum) 
	//{
		pns->stopTimer(SENDER, 0);										//���ȹرն�ʱ��
		pns->startTimer(SENDER, Configuration::TIME_OUT, 0);	//�����������ͷ���ʱ��
		pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[0]);
		pUtils->printPacket("���ͷ���ʱ��ʱ�䵽���ط����绹δȷ�ϵı���", this->packetWaitingAck[0]);
	//}
	
}
