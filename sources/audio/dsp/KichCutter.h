#ifndef KICHCUTTER_H
#define KICHCUTTER_H
namespace MediaSDK
{
	class CKichCutter
	{
	public:
		CKichCutter();
		~CKichCutter();
		int Despike(short *sBuffer, int nFlags);

	private:
		short m_sZeroSampleCount;
		short m_sSpikeSampleCount;
		short m_sThreshold;
		short m_sTotalSpikeSampleCount;
		bool m_bRemoveContinentalShelves;
		int m_nFramesInBuffer;
		int m_nDataBufSize;
		int m_nDataBufIOSize;
		int m_nDataBufCopySize;


		short *m_sSilentBuf;
		short *m_sSilentFillBuf;
		short *m_sTempBuf;
		short *m_sDataBuf;
		int *m_nFlagBuf;
	};
}

#endif // !KICHCUTTER_H
