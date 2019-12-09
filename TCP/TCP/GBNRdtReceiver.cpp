#include "stdafx.h"
#include "Global.h"
#include "GBNRdtReceiver.h"

int flag[3] = { 0 };
GBNRdtReceiver::GBNRdtReceiver():expectSequenceNumberRcvd(0)
{
	lastAckPkt.acknum = -1; //��ʼ״̬�£��ϴη��͵�ȷ�ϰ���ȷ�����Ϊ-1��ʹ�õ���һ�����ܵ����ݰ�����ʱ��ȷ�ϱ��ĵ�ȷ�Ϻ�Ϊ-1
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1;	//���Ը��ֶ�
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
	//���У����Ƿ���ȷ
	pUtils->printPacket("���շ��յ����ͷ��ı���", packet);
	int checkSum = pUtils->calculateCheckSum(packet);
	if (checkSum == packet.checksum) 
	{
		
		if (this->expectSequenceNumberRcvd == packet.seqnum)
		{
			pUtils->printPacket("���շ���ȷ�յ����ͷ��ı���", packet);
			Message msg;
			memcpy(msg.data, packet.payload, sizeof(packet.payload));
			pns->delivertoAppLayer(RECEIVER, msg);
			int i = 0;
			int count = 1;//�����������
			while (i < 3 && flag[i])//�����������������
			{
				memcpy(msg.data, buf[i].payload, sizeof(buf[i].payload));
				pns->delivertoAppLayer(RECEIVER, msg);
				flag[i] = 0;
				i++;
				count++;
			}
			i--;
			if (i == -1)//������������
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
			pUtils->printPacket("�������ѷ��ͣ����շ������µ�ȷ�ϱ���", lastAckPkt);
			pns->sendToNetworkLayer(SENDER, lastAckPkt);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢���ϴε�ȷ�ϱ���
			//for (i = 0;i < 3 && flag[i] == 0;i++)//�˴����󣬱���ҪJ����������M����* * M��Ȼ��J��֮��i=2��������������
			//	;                                //ʵ��ֻӦ��һ��
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
			pUtils->printPacket("�ѻ��棬���շ����·����ϴε�ȷ�ϱ���", lastAckPkt);
			pns->sendToNetworkLayer(SENDER, lastAckPkt);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢��ȷ�ϱ��ĵ��Է�
		}
		else 
		{
			pUtils->printPacket("��������Ҳ�ǻ������ݣ����շ����·����ϴε�ȷ�ϱ���", lastAckPkt);
			pns->sendToNetworkLayer(SENDER, lastAckPkt);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢���ϴε�ȷ�ϱ���
		}
	}
	else
	{
		pUtils->printPacket("У��ʹ��󣬽��շ����·����ϴε�ȷ�ϱ���", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢���ϴε�ȷ�ϱ���
	}
}