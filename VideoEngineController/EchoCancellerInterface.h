#ifndef ECHO_CANCELLER_INTERFACE_H
#define ECHO_CANCELLER_INTERFACE_H


class EchoCancellerInterface
{

public:

	virtual int AddFarEndData(short *farEndData, int dataLen);

	virtual int CancelEcho(short *nearEndData, int dataLen);

};


#endif // !ECHO_CANCELLER_INTERFACE_H
