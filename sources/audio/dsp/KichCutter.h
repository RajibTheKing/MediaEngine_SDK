#ifndef KICHCUTTER_H

class KichCutter
{
public:
	KichCutter();
	~KichCutter();
	void Despike(short *sBuffer);

private:
	short m_sZeroSampleCount;
	short m_sSpikeSampleCount;
	short m_sThreshold;
	short m_sTotalSpikeSampleCount;
	bool m_bRemoveContinentalShelves;
	int m_nDataBufSize;
	int m_nDataBufIOSize;
	int m_nDataBufCopySize;


	short *m_sSilentBuf;
	short *m_sSilentFillBuf;
	short *m_sTempBuf;
	short *m_sDataBuf;
};

#endif // !KICHCUTTER_H
