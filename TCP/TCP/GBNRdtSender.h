#ifndef STOP_WAIT_RDT_SENDER_H
#define STOP_WAIT_RDT_SENDER_H
#include "RdtSender.h"
class GBNRdtSender :public RdtSender
{
private:
	int nextseqnum;	// ��һ��������� 
	int base;       //�����
	bool waitingState;				// �Ƿ��ڵȴ�Ack��״̬
	const int N = 4;
	Packet packetWaitingAck[4];		//�ѷ��Ͳ��ȴ�Ack�����ݰ�
	int AckCount[8];

public:

	bool getWaitingState();
	bool send(Message &message);						//����Ӧ�ò�������Message����NetworkServiceSimulator����,������ͷ��ɹ��ؽ�Message���͵�����㣬����true;�����Ϊ���ͷ����ڵȴ���ȷȷ��״̬���ܾ�����Message���򷵻�false
	void receive(Packet &ackPkt);						//����ȷ��Ack������NetworkServiceSimulator����	
	void timeoutHandler(int seqNum);					//Timeout handler������NetworkServiceSimulator����

public:
	GBNRdtSender();
	virtual ~GBNRdtSender();
};

#endif

