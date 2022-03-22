#ifndef __MMATCHANNOUNCE_H__
#define __MMATCHANNOUNCE_H__

struct MAnnounceContent
{
	char szText[256];
	int nWaitSec;
};

class MMatchAnnounce
{
public:
	MMatchAnnounce();
	~MMatchAnnounce();
	
	void Announcement(unsigned long nTime);
	bool LoadAnnounceList();
	
	void Destroy();

private:
	deque<MAnnounceContent *> m_Announces;
	
	deque<MAnnounceContent *> m_AnnounceQueue;
	unsigned long m_nLastAnnounceTime;
};

extern MMatchAnnounce g_Announcer;

#endif