#ifndef __MCOMMAND_H__
#define __MCOMMAND_H__

#if _GAME_VERSION >= 2012
#define MPACKET_HEADER_LENGTH	10
#else
#define MPACKET_HEADER_LENGTH	11
#endif

// MCommandFinalizeType flags.
#define MCFT_NONE	0
#define MCFT_END	1
#define MCFT_RAW	2


class MCommandWriter
{
public:
	MCommandWriter(int nDefBuffSize = 0x400);	// 0x400 (1024).
	~MCommandWriter();

	void CreateBuffer(int nSize);
	void RemoveBuffer();
	void ResizeBuffer(int nBaseSize = 8);
	
	void SetData(const unsigned char *p, const int n);

	const unsigned char *GetData() const { return m_pData; }
	int GetDataIndex() const { return m_nDataIndex; }
	bool IsReadyToUse() const { return m_bIsUseReady; }

	void WriteBool(bool v);
	void WriteChar(char v);
	void WriteUChar(unsigned char v);
	void WriteShort(short v);
	void WriteUShort(unsigned short v);
	void WriteInt(int v);
	void WriteUInt(unsigned int v);
	void WriteLong(long v);
	void WriteULong(unsigned long v);
	void WriteLLong(long long v);
	void WriteULLong(unsigned long long v);
	void WriteFloat(float v);
	void WriteDouble(double v);
	void WriteString(const char *v, int n = -1);
	void WriteMUID(const MUID &v);
	void WriteSkip(int n);
	
	void WriteData(const void *v, int n);
	void WriteBlob(const void *v, int n);

	void WriteVec(float x, float y, float z);	// float vector.
	void WriteSVec(short x, short y, short z);	// short vector.

	void StartBlob(int nSize = -1);
	void EndBlob();

	void Finalize(unsigned short nCmdID, unsigned long nType = MCFT_NONE);
	void Finalize();

	void ResetIndex();
	
private:
	template<class T> void Write(T v);
	
private:
	unsigned char *m_pData;

	int m_nDataIndex;
	int m_nBaseDataIndex;

	bool m_bIsUseReady;
	int m_nDataSize;

	// Blob-related.
	int m_nBlobIndex;	// header index.
	int m_nBlobSize;	// size of individual data.
};

class MCommandReader
{
public:
	MCommandReader(const unsigned char *pData = NULL, const int nDataSize = 0, const MUID &uidOwner = MUID(0, 0), const MUID &uidAcceptor = MUID(0, 0));
	~MCommandReader();

	void SetData(const unsigned char *p, const int n);
	void SetOwnerUID(const MUID &uid) { m_uidOwner = uid; }
	
	const unsigned char *GetData() const { return m_pData; }
	int GetDataSize() const	{ return m_nDataSize; }
	const MUID &GetOwnerUID() const { return m_uidOwner; }
	
	void SetAcceptorUID(const MUID &uid)	{ m_uidAcceptor = uid; }
	const MUID &GetAcceptorUID()	{ return m_uidAcceptor; }
	
	bool ToNext();
	void ResetIndex() { m_nDataIndex = m_nBaseDataIndex + MPACKET_HEADER_LENGTH; }

	unsigned short GetCommandID() const;

	bool ReadBool(bool *v);
	bool ReadChar(char *v);
	bool ReadUChar(unsigned char *v);
	bool ReadShort(short *v);
	bool ReadUShort(unsigned short *v);
	bool ReadInt(int *v);
	bool ReadUInt(unsigned int *v);
	bool ReadLong(long *v);
	bool ReadULong(unsigned long *v);
	bool ReadLLong(long long *v);
	bool ReadULLong(unsigned long long *v);
	bool ReadFloat(float *v);
	bool ReadDouble(double *v);
	bool ReadString(char *v, int nBuffSize, int nLen = -1);
	bool ReadMUID(MUID *v);
	bool ReadBlob(void *v, int n);
	bool ReadBlobArray(int *v, int *s = NULL);

	bool ReadSkip(int n);
	bool ReadStringSkip(int n);
	
	bool ReadData(void *v, int n);

	bool ReadVec(float *px, float *py, float *pz);	// float vector.
	bool ReadSVec(short *px, short *py, short *pz);	// short vector.
	
	void DeleteData();
	
private:
	template<class T> bool Read(T *v);
	
private:
	unsigned char *m_pData;
	int m_nDataSize;

	int m_nDataIndex;
	int m_nBaseDataIndex;

	MUID m_uidOwner;
	MUID m_uidAcceptor;
};

typedef MCommandWriter MCmdWriter;
typedef MCommandReader MCmdReader;

// for store vector command parameter.
struct FloatVector
{
	float x;
	float y;
	float z;
};

struct ShortVector
{
	short x;
	short y;
	short z;
};

void CreateResultCommand(MCommandWriter *pCmd, unsigned short nCmdID, int nResult = 0);

MCommandReader *CreateReaderCommand(const unsigned char *pData, const int nSize, const MUID &uidOwner, const MUID &uidAcceptor);
void DestroyReaderCommand(MCommandReader *pCmd);
bool BuildReaderCommand(const unsigned char *pData, const int nSize, const MUID &uidOwner, vector<MCommandReader *> *pOut, const MUID &uidAcceptor = MUID(0, 0));

#endif
