// StopWait.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "Global.h"
#include "RdtSender.h"
#include "RdtReceiver.h"
#include "GBNRdtSender.h"
#include "GBNRdtReceiver.h"


int main(int argc, char** argv[])
{
	RdtSender *ps = new GBNRdtSender();
	RdtReceiver * pr = new GBNRdtReceiver();
	pns->init();
	pns->setRtdSender(ps);
	pns->setRtdReceiver(pr);
	pns->setInputFile("C:\\Users\\����ά\\Documents\\Visual Studio 2015\\Projects\\TCP\\Debug\\input.txt");
	pns->setOutputFile("C:\\Users\\����ά\\Documents\\Visual Studio 2015\\Projects\\TCP\\Debug\\output.txt");
	pns->start();

	delete ps;
	delete pr;
	delete pUtils;									//ָ��Ψһ�Ĺ�����ʵ����ֻ��main��������ǰdelete
	delete pns;										//ָ��Ψһ��ģ�����绷����ʵ����ֻ��main��������ǰdelete
	
	return 0;
}

