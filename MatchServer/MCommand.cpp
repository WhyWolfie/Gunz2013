#include "pch.h"
#include "MCommand.h"

// Packet writer.
MCommandWriter::MCommandWriter(int nDefBuffSize)
{
	m_pData = NULL;
	m_nDataSize = 0;
	m_bIsUseReady = false;
	m_nDataIndex = MPACKET_HEADER_LENGTH;
	m_nBaseDataIndex = 0;
	m_nBlobIndex = 0;
	m_nBlobSize = 0;

	CreateBuffer(nDefBuffSize);
}

MCommandWriter::~MCommandWriter()
{
	RemoveBuffer();
}

void MCommandWriter::CreateBuffer(int nSize)
{
	if(m_pData == NULL &&
	        nSize > 0)
	{
		m_pData = new unsigned char[nSize];
		m_nDataSize = nSize;
	}
}

void MCommandWriter::RemoveBuffer()
{
	if(m_pData != NULL)
	{
		delete[] m_pData;
		m_pData = NULL;
		m_nDataSize = 0;
	}
}

void MCommandWriter::ResizeBuffer(int nBaseSize)
{
	if(m_nDataIndex + nBaseSize > m_nDataSize &&	// buffer size check before doing resize.
	        m_pData != NULL)
	{
		const int nBytesToCopy = m_nDataIndex;
		const int nNewDataSize = m_nDataSize * 2;

		unsigned char *pNewData = new unsigned char[nNewDataSize];
		for(int i = 0; i < nBytesToCopy; i++)
			pNewData[i] = m_pData[i];
			
		delete[] m_pData;

		m_pData = pNewData;
		m_nDataSize = nNewDataSize;
	}
}

void MCommandWriter::SetData(const unsigned char *p, const int n)
{
	m_bIsUseReady = true;
	m_nDataIndex = 0;
	m_nBaseDataIndex = 0;
	
	WriteData(p, n);
}

// ---------------------------------------------------------------------------
void MCommandWriter::WriteBool(bool v)
{
	ResizeBuffer((int)sizeof(bool));
	if(v == false)
		m_pData[m_nDataIndex] = 0;
	else
		m_pData[m_nDataIndex] = 1;
	m_nDataIndex += (int)sizeof(bool);
}

void MCommandWriter::WriteChar(char v)
{
	Write((char)v);
}

void MCommandWriter::WriteUChar(unsigned char v)
{
	Write((unsigned char)v);
}

void MCommandWriter::WriteShort(short v)
{
	Write((short)v);
}

void MCommandWriter::WriteUShort(unsigned short v)
{
	Write((unsigned short)v);
}

void MCommandWriter::WriteInt(int v)
{
	Write((int)v);
}

void MCommandWriter::WriteUInt(unsigned int v)
{
	Write((unsigned int)v);
}

void MCommandWriter::WriteLong(long v)
{
	Write((long)v);
}

void MCommandWriter::WriteULong(unsigned long v)
{
	Write((unsigned long)v);
}

void MCommandWriter::WriteLLong(long long v)
{
	Write((long long)v);
}

void MCommandWriter::WriteULLong(unsigned long long v)
{
	Write((unsigned long long)v);
}

void MCommandWriter::WriteFloat(float v)
{
	Write((float)v);
}

void MCommandWriter::WriteDouble(double v)
{
	Write((double)v);
}

void MCommandWriter::WriteString(const char *v, int n)
{
	int nStrLen = (int)strlen(v);

	if(n == -1)
	{
		ResizeBuffer(nStrLen + 2);	// Resize by string length.

		// General string.
		if(v != NULL)
		{
			WriteShort((short)(nStrLen + 2));
			for(int i = 0; i < nStrLen; i++)
				m_pData[m_nDataIndex + i] = (const unsigned char)v[i];
			m_pData[m_nDataIndex + nStrLen] = '\0';
			m_nDataIndex += nStrLen + 2;
		}
		else WriteShort(2);
	}
	else
	{
		ResizeBuffer(n);	// Resize by total length.

		// Fixed-length string.
		if(v != NULL)
		{
			for(int i = 0; i < n; i++)
			{
				if(i < nStrLen)
					m_pData[m_nDataIndex + i] = (const unsigned char)v[i];
				else
					m_pData[m_nDataIndex + i] = '\0';
			}
			m_nDataIndex += n;
		}
		else
		{
			for(int i = 0; i < n; i++)
				WriteUChar(0);
		}
	}
}

void MCommandWriter::WriteMUID(const MUID &v)
{
	WriteULong(v.ulHighID);
	WriteULong(v.ulLowID);
}

void MCommandWriter::WriteSkip(int n)
{
	for(int i = 0; i < n; i++)
		WriteUChar(0);
}

void MCommandWriter::WriteData(const void *v, int n)
{
	ResizeBuffer(n);
	memcpy(&m_pData[m_nDataIndex], v, n);
	m_nDataIndex += n;
}

void MCommandWriter::WriteBlob(const void *v, int n)
{
	WriteInt(n);
	WriteData(v, n);
}

void MCommandWriter::WriteVec(float x, float y, float z)
{
	WriteFloat(x);
	WriteFloat(y);
	WriteFloat(z);
}

void MCommandWriter::WriteSVec(short x, short y, short z)
{
	WriteShort(x);
	WriteShort(y);
	WriteShort(z);
}

void MCommandWriter::StartBlob(int nSize)
{
	const int nHeaderSize = (nSize == -1) ? (int)sizeof(int) : ((int)sizeof(int) * 3);
	ResizeBuffer(nHeaderSize);

	m_nBlobIndex = m_nDataIndex;
	m_nBlobSize = nSize;

	m_nDataIndex += nHeaderSize;
}

void MCommandWriter::EndBlob()
{
	if(m_nBlobSize == -1)
	{
		const int nBlobTotal = ((m_nDataIndex - m_nBlobIndex) - (int)sizeof(int));
		*((int *)&m_pData[m_nBlobIndex]) = nBlobTotal;		
	}
	else
	{
		const int nBlobSize = m_nBlobSize;
		const int nBlobCount = ((nBlobSize != 0) ? ((m_nDataIndex - m_nBlobIndex - ((int)sizeof(int) * 3)) / nBlobSize) : 0);
		const int nBlobTotal = (nBlobCount * nBlobSize) + ((int)sizeof(int) * 2);

		// write results to the header.
		*((int *)&m_pData[m_nBlobIndex]) = nBlobTotal;		// 1 : total.
		*((int *)&m_pData[m_nBlobIndex + 4]) = nBlobSize;	// 2 : size.
		*((int *)&m_pData[m_nBlobIndex + 8]) = nBlobCount;	// 3 : count.
	}
}

void MCommandWriter::Finalize(unsigned short nCmdID, unsigned long nType)
{
	short nSize = m_nDataIndex - m_nBaseDataIndex,
	      nCommandSize = m_nDataIndex - m_nBaseDataIndex - 6;
		  
	if((nType & MCFT_RAW) == 0)
		*((unsigned short *)&m_pData[m_nBaseDataIndex]) = 0x65;	// 0x65 (101) = encrypted.
	else
		*((unsigned short *)&m_pData[m_nBaseDataIndex]) = 0x64;	// 0x64 (100) = decrypted.

	*((short *)&m_pData[m_nBaseDataIndex + 2]) = nSize;
	*((short *)&m_pData[m_nBaseDataIndex + 6]) = nCommandSize;
	*((unsigned short *)&m_pData[m_nBaseDataIndex + 8]) = nCmdID;
	#if _GAME_VERSION < 2012
	*((unsigned char *)&m_pData[m_nBaseDataIndex + 10]) = 0;	// MCommand serial number.
	#endif
	
	if((nType & MCFT_END) != 0)
		m_bIsUseReady = true;
	else
	{
		ResizeBuffer(MPACKET_HEADER_LENGTH);

		m_nBaseDataIndex = m_nDataIndex;
		m_nDataIndex += MPACKET_HEADER_LENGTH;
	}
}

void MCommandWriter::Finalize()
{
	m_nDataIndex -= MPACKET_HEADER_LENGTH;

	if(m_nDataIndex == m_nBaseDataIndex && m_nDataIndex > 0)
		m_bIsUseReady = true;
	else
		m_bIsUseReady = false;
}

void MCommandWriter::ResetIndex()
{
	m_bIsUseReady = false;
	m_nDataIndex = MPACKET_HEADER_LENGTH;
	m_nBaseDataIndex = 0;
}

template<class T> 
void MCommandWriter::Write(T v)
{
	ResizeBuffer((int)sizeof(T));
	*((T *)&m_pData[m_nDataIndex]) = v;
	m_nDataIndex += (int)sizeof(T);
}


// Packet reader.
MCommandReader::MCommandReader(const unsigned char *pData, const int nDataSize, const MUID &uidOwner, const MUID &uidAcceptor)
{
	m_nDataIndex = MPACKET_HEADER_LENGTH;
	m_nBaseDataIndex = 0;
	
	m_pData = NULL;
	m_nDataSize = 0;
	SetData(pData, nDataSize);
	
	SetOwnerUID(uidOwner);
	SetAcceptorUID(uidAcceptor);
}

MCommandReader::~MCommandReader()
{
	DeleteData();
}

void MCommandReader::SetData(const unsigned char *p, const int n)
{
	DeleteData();
	
	if(p != NULL && n > 0)
	{
		m_pData = new unsigned char[n];
		memcpy(m_pData, p, n);
		
		m_nDataSize = n;
	}
}

bool MCommandReader::ToNext()
{
	if(m_nBaseDataIndex + MPACKET_HEADER_LENGTH > m_nDataSize)
		return false;

	unsigned short nHeader = *((unsigned short *)&m_pData[m_nBaseDataIndex]);
	
	// encrypted or decrypted.
	if(nHeader != 0x64 && nHeader != 0x65)
		return false;

	short nSize = *((short *)&m_pData[m_nBaseDataIndex + 2]);
	short nCommandSize = *((short *)&m_pData[m_nBaseDataIndex + 6]);

	if(nSize != nCommandSize + 6)
		return false;

	if(m_nBaseDataIndex + (int)nSize + MPACKET_HEADER_LENGTH > m_nDataSize)
		return false;

	m_nBaseDataIndex += (int)nSize;	// update curr index to next index.
	m_nDataIndex = m_nBaseDataIndex + MPACKET_HEADER_LENGTH;
	
	nHeader = *((unsigned short *)&m_pData[m_nBaseDataIndex]);
	
	if(nHeader != 0x64 && nHeader != 0x65)
		return false;
		
	nSize = *((short *)&m_pData[m_nBaseDataIndex + 2]);
	nCommandSize = *((short *)&m_pData[m_nBaseDataIndex + 6]);
	
	if(nSize != nCommandSize + 6)
		return false;

	return true;
}

unsigned short MCommandReader::GetCommandID() const
{
	if(m_nBaseDataIndex + MPACKET_HEADER_LENGTH > m_nDataSize)
		return 0;
	return *((unsigned short *)&m_pData[m_nBaseDataIndex + 8]);
}

bool MCommandReader::ReadBool(bool *v)
{
	if(m_nDataIndex + (int)sizeof(bool) > m_nDataSize)
	{
		*v = false;
		return false;
	}
	
	if(m_pData[m_nDataIndex] == 0)
		*v = false;
	else
		*v = true;
	m_nDataIndex += (int)sizeof(bool);
	
	return true;
}

bool MCommandReader::ReadChar(char *v)
{
	return Read((char *)v);
}

bool MCommandReader::ReadUChar(unsigned char *v)
{
	return Read((unsigned char *)v);
}

bool MCommandReader::ReadShort(short *v)
{
	return Read((short *)v);
}

bool MCommandReader::ReadUShort(unsigned short *v)
{
	return Read((unsigned short *)v);
}

bool MCommandReader::ReadInt(int *v)
{
	return Read((int *)v);
}

bool MCommandReader::ReadUInt(unsigned int *v)
{
	return Read((unsigned int *)v);
}

bool MCommandReader::ReadLong(long *v)
{
	return Read((long *)v);
}

bool MCommandReader::ReadULong(unsigned long *v)
{
	return Read((unsigned long *)v);
}

bool MCommandReader::ReadLLong(long long *v)
{
	return Read((long long *)v);
}

bool MCommandReader::ReadULLong(unsigned long long *v)
{
	return Read((unsigned long long *)v);
}

bool MCommandReader::ReadFloat(float *v)
{
	return Read((float *)v);
}

bool MCommandReader::ReadDouble(double *v)
{
	return Read((double *)v);
}

bool MCommandReader::ReadString(char *v, int nBuffSize, int nLen)
{
	for(int i = 0; i < nBuffSize; i++)
		v[i] = '\0';

	bool bIsReadLength = true;
	if(nLen == -1)
	{
		short nReadLength;
		ReadShort(&nReadLength);
		nLen = (int)nReadLength - 2;
	}
	else
		bIsReadLength = false;

	if(nLen >= 0)
	{
		int nTotalIndex = m_nDataIndex + nLen;
		if(bIsReadLength == true) nTotalIndex = nTotalIndex + 2;

		if(nTotalIndex <= m_nDataSize)
		{
			for(int i = 0; i < nLen && i < nBuffSize - 1; i++)
				v[i] = (const char)m_pData[m_nDataIndex + i];
			m_nDataIndex = nTotalIndex;
		}
		else return false;
	}
	
	return true;
}

bool MCommandReader::ReadMUID(MUID *v)
{
	v->ulHighID = 0;
	v->ulLowID = 0;
	
	if(ReadULong(&v->ulHighID) == false) return false;
	if(ReadULong(&v->ulLowID) == false) return false;
	
	return true;
}

bool MCommandReader::ReadBlob(void *v, int n)
{
	int nBlobHeader;
	if(ReadInt(&nBlobHeader) == false) return false;
	
	if(nBlobHeader != n) return false;
	
	return ReadData(v, n);
}

bool MCommandReader::ReadBlobArray(int *v, int *s)
{
	*v = 0;
	if(s != NULL) *s = 0;

	int nBlobHeader = 0, nBlobSize = 0, nBlobCount = 0;

	if(ReadInt(&nBlobHeader) == false) return false;	// Blob size.
	if(nBlobHeader < (int)(sizeof(int) * 2)) return false;	// too less blob size, invalid or empty.
		
	if(ReadInt(&nBlobSize) == false) return false;
	if(ReadInt(&nBlobCount) == false) return false;

	if(nBlobHeader <= 0 || nBlobSize <= 0 || nBlobCount <= 0)
		return false;
		
	nBlobHeader -= (int)(sizeof(int) * 2);
	if(nBlobHeader != nBlobSize * nBlobCount)
		return false;
		
	nBlobHeader /= nBlobCount;
	if(nBlobHeader != nBlobSize)
		return false;

	*v = nBlobCount;
	if(s != NULL) *s = nBlobSize;
	
	return true;
}

bool MCommandReader::ReadSkip(int n)
{
	if(m_nDataIndex + n > m_nDataSize)
		return false;
		
	m_nDataIndex += n;
	
	return true;
}

bool MCommandReader::ReadStringSkip(int n)
{
	for(int i = 0; i < n; i++)
	{
		short nStrLen;
		if(ReadShort(&nStrLen) == false) return false;

		if(m_nDataIndex + (int)nStrLen <= m_nDataSize)
			m_nDataIndex += (int)nStrLen;
		else return false;
	}
	
	return true;
}

bool MCommandReader::ReadData(void *v, int n)
{
	if(m_nDataIndex + n > m_nDataSize) return false;
	
	memcpy(v, &m_pData[m_nDataIndex], n);
	m_nDataIndex += n;
	
	return true;
}

bool MCommandReader::ReadVec(float *px, float *py, float *pz)
{
	if(ReadFloat(px) == false) return false;
	if(ReadFloat(py) == false) return false;
	if(ReadFloat(pz) == false) return false;
	
	return true;
}

bool MCommandReader::ReadSVec(short *px, short *py, short *pz)
{
	if(ReadShort(px) == false) return false;
	if(ReadShort(py) == false) return false;
	if(ReadShort(pz) == false) return false;
	
	return true;
}

void MCommandReader::DeleteData()
{
	if(m_pData != NULL)
	{
		delete[] m_pData;
		m_pData = NULL;
	}
	
	m_nDataSize = 0;
}

template<class T> 
bool MCommandReader::Read(T *v)
{
	if(m_nDataIndex + (int)sizeof(T) > m_nDataSize)
	{
		*v = 0;
		return false;
	}
	
	*v = *((T *)&m_pData[m_nDataIndex]);
	m_nDataIndex += (int)sizeof(T);
	
	return true;
}


// result writer.
void CreateResultCommand(MCommandWriter *pCmd, unsigned short nCmdID, int nResult)
{
	pCmd->WriteInt(nResult);
	pCmd->Finalize(nCmdID, MCFT_NONE);
}

// reader maker.
MCommandReader *CreateReaderCommand(const unsigned char *pData, const int nSize, const MUID &uidOwner, const MUID &uidAcceptor)
{
	return new MCommandReader(pData, nSize, uidOwner, uidAcceptor);
}

void DestroyReaderCommand(MCommandReader *pCmd)
{
	pCmd->DeleteData();
	delete pCmd;
}

// reader command builder.
bool BuildReaderCommand(const unsigned char *pData, const int nSize, const MUID &uidOwner, vector<MCommandReader *> *pOut, const MUID &uidAcceptor)	// this function is must given decrypted data.
{
	int nIndex = 0;
	
	while(nIndex + MPACKET_HEADER_LENGTH <= nSize && nIndex != nSize)
	{
		unsigned short a = *((unsigned short *)&pData[nIndex]);
		
		// 100 (decrypted), 101 (encrypted).
		if(a != 0x64 && a != 0x65)
		{	// invalid command.
			for(vector<MCommandReader *>::iterator i = pOut->begin(); i != pOut->end(); i++)
			{
				DestroyReaderCommand(*i);
			}
			pOut->clear();
			
			return false;
		}
		
		short nPacketSize = *((short *)&pData[nIndex + 2]);
		
		pOut->push_back(CreateReaderCommand(&pData[nIndex], (int)nPacketSize, uidOwner, uidAcceptor));
		
		nIndex += (int)nPacketSize;
	}
	
	return true;
}
