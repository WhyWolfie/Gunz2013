#include "pch.h"

#include "MMatchAnnounce.h"
#include "MMatchServer_Etc.h"

MMatchAnnounce g_Announcer;

MMatchAnnounce::MMatchAnnounce()
{
	m_nLastAnnounceTime = GetTime();
}

MMatchAnnounce::~MMatchAnnounce()
{
	Destroy();
}

void MMatchAnnounce::Announcement(unsigned long nTime)
{
	if(m_Announces.size() == 0) return;	// not using.
	
	if(m_AnnounceQueue.size() == 0) m_AnnounceQueue = m_Announces;
	
	MAnnounceContent *pAnnounce = m_AnnounceQueue.front();
	
	unsigned long nAnnounceWaitMSec = (unsigned long)(pAnnounce->nWaitSec * 1000);
	unsigned long nNextAnnounceTime = m_nLastAnnounceTime + nAnnounceWaitMSec;
	
	if(nNextAnnounceTime <= nTime)
	{
		const char *pszText = pAnnounce->szText;
		
		if(MStricmp(pszText, "NULL") != 0)
			AnnounceToAll(pszText);
			
		m_nLastAnnounceTime = nTime;
		
		m_AnnounceQueue.pop_front();
	}
}

bool MMatchAnnounce::LoadAnnounceList()
{
	XMLDocument doc;
	doc.LoadFile("announcement.xml");
	
	if(doc.Error() == true) 
	{
		mlog("Error while loading auto text announcer.");
		return false;
	}
	
	mlog("Loading auto announcer text list...");
	
	XMLHandle handle(&doc);
	handle = handle.FirstChildElement("XML");
	
	XMLElement *element;
	
	element = handle.FirstChildElement("ANNOUNCECONFIG").ToElement();
	if(element == NULL) return false;
	
	bool bUse = element->BoolAttribute("use");
	if(bUse == false) return false;
	
	int nAnnounceID = element->IntAttribute("announce_id");
	
	element = handle.FirstChildElement("ANNOUNCEMENT").ToElement();
	if(element == NULL) return false;
	
	XMLElement *subelement = NULL;
	
	while(element != NULL)
	{
		int nCurrID = element->IntAttribute("id");
		
		if(nCurrID == nAnnounceID)
		{
			subelement = element;
			break;
		}
		
		element = element->NextSiblingElement();
	}
	
	if(subelement == NULL) return false;
	
	subelement = subelement->FirstChildElement("ANNOUNCE");
	
	while(subelement != NULL)
	{
		if(subelement->Attribute("text") == NULL)
		{
			subelement = subelement->NextSiblingElement();
			continue;
		}
			
		MAnnounceContent *pNew = new MAnnounceContent;
		strcpy(pNew->szText, subelement->Attribute("text"));
		pNew->nWaitSec = subelement->IntAttribute("waitsec");
		
		m_Announces.push_back(pNew);
		
		subelement = subelement->NextSiblingElement();
	}
	
	mlog("Loaded announcement list.");
	
	return true;
}

void MMatchAnnounce::Destroy()
{
	 m_AnnounceQueue.clear();
	 
	for(deque<MAnnounceContent *>::iterator i = m_Announces.begin(); i != m_Announces.end(); i++)
	{
		delete (*i);
	}
	m_Announces.clear();
}