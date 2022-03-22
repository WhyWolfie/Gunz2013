#include "pch.h"
#include "MMatchChannel.h"

char MMatchChannel::m_szDefaultRule[32];
int MMatchChannel::m_nDefaultRuleID;

bool MMatchChannel::Enter(MMatchObject *pObj)
{
	// check already exists.
	if(m_Users.find(pObj->GetUID()) != m_Users.end())
		return false;	// error, already entered.
		
	m_Users.insert(pair<MUID, MMatchObject *>(pObj->GetUID(), pObj));
	return true;
}

bool MMatchChannel::Leave(MMatchObject *pObj)
{
	// remove player.
	m_Users.erase(pObj->GetUID());
	return true;
}

void MMatchChannel::RetrievePlayerList(MChannelPlayerList *pOut, int nPage)
{
	// for lobby list.
	
	int nTotal = 0;
	nPage *= MAX_CHANNEL_PLAYERLIST_COUNT;
	
	unsigned int nChecksum = 0;
	
	for(map<MUID, MMatchObject *>::iterator it = m_Users.begin(); 
		it != m_Users.end(); it++)
	{
		MMatchObject *pCurr = (*it).second;
		
		if(nPage <= 0)
		{
			pOut->pObject[nTotal] = pCurr;
			nChecksum += pCurr->MakeChecksum();
			
			nTotal++;
			if(nTotal >= MAX_CHANNEL_PLAYERLIST_COUNT) break;
		}
		else
			nPage--;
	}
	
	pOut->nTotalCount = nTotal;
	pOut->nChecksum = nChecksum;
}

// ------------------------------------------------------------

MMatchChannelManager g_ChannelMgr;

MMatchChannelManager::MMatchChannelManager()
{
}

MMatchChannelManager::~MMatchChannelManager()
{
	for(list<MMatchChannel *>::iterator i = m_Channels.begin(); i != m_Channels.end(); i++)
	{
		delete (*i);
	}
	m_Channels.clear();
}

bool MMatchChannelManager::LoadPreset()
{
	XMLDocument doc;
	doc.LoadFile(MXML_FILENAME_CHANNEL);
	
	if(doc.Error() == true) 
	{
		mlog("%s file loading failure.", MXML_FILENAME_CHANNEL);
		return false;
	}
	
	mlog("Loading channel list preset...");
	
	XMLHandle handle(&doc);
	XMLElement *element;
	
	handle = handle.FirstChildElement("XML");
	
	// default rule.
	element = handle.FirstChildElement("DEFAULTRULENAME").ToElement();
	if(element != NULL && element->GetText() != NULL)
		MMatchChannel::SetDefaultRule(element->GetText());
	else
	{
		MMatchChannel::SetDefaultRule(MCHANNEL_DEFAULT_RULENAME);
		mlog("channel.xml : has no default rule name.");
	}
	
	// default rule (id).
	element = handle.FirstChildElement("DEFAULTRULEID").ToElement();
	if(element != NULL && element->GetText() != NULL)
		MMatchChannel::SetDefaultRuleID(ToInt(element->GetText()));
	else
	{
		MMatchChannel::SetDefaultRuleID(MCHANNEL_DEFAULT_RULEID);
		mlog("channel.xml : has no default rule ID.");
	}
	
	element = handle.FirstChildElement("CHANNEL").ToElement();
	
	while(element != NULL)
	{
		if(element->Attribute("name") == NULL)
		{
			element = element->NextSiblingElement();
			continue;
		}
		
		MMatchChannel *pNewChannel = new MMatchChannel;
		
		pNewChannel->SetName(element->Attribute("name"));
		pNewChannel->SetType(ConvertChannelTypeNameToID(element->Attribute("type") == NULL ? "" : element->Attribute("type")));
		pNewChannel->SetRule(element->Attribute("rule") == NULL ? MMatchChannel::GetDefaultRule() : element->Attribute("rule"));
		pNewChannel->SetRuleID(element->IntAttribute("ruleid"));
		pNewChannel->SetMaxPlayers(element->IntAttribute("maxplayer"));
		
		if(Add(pNewChannel) == false)
		{
			delete pNewChannel;
		}
		
		element = element->NextSiblingElement();
	}
	
	mlog("Channel preset is successfully loaded.");
	
	return true;
}

/*
	MMatchChannelManager::Add() function : 
		channel added = true as ret, non-zero uid as pOutUID.
		channel not added but found same channel = false as ret, non-zero uid as pOutUID.
		channel not added and not found same channel = false as ret, zero uid as pOutUID.
*/

bool MMatchChannelManager::Add(MMatchChannel *pChannel, MUID *pOutUID)
{
	if(pOutUID != NULL) (*pOutUID) = MUID(0, 0);
	
	if(IsValidChannelType(pChannel->GetType()) == false) return false;
	
	for(list<MMatchChannel *>::iterator it = m_Channels.begin(); 
		it != m_Channels.end(); it++)
	{
		// if((*it) == pChannel)
		// 	return false;
		
		MMatchChannel *pCurr = (*it);
		
		if(pCurr->GetType() == pChannel->GetType() && MStricmp(pCurr->GetName(), pChannel->GetName()) == 0)
		{
			(*pOutUID) = pCurr->GetUID();
			return false;
		}
	}
	
	MUID uidChannel = MUID::Assign();
	
	pChannel->SetUID(uidChannel);
	m_Channels.push_back(pChannel);	// add.
	
	if(pOutUID != NULL)
	{
		(*pOutUID) = uidChannel;
	}
	
	return true;
}

bool MMatchChannelManager::Add(const char *pszName, int nType, MUID *pOutUID)	// private channel.
{
	if(pOutUID != NULL) (*pOutUID) = MUID(0, 0);
	
	if(nType != MCHANNEL_TYPE_USER && nType != MCHANNEL_TYPE_CLAN)
		return false;
		
	MMatchChannel *pNew = new MMatchChannel;
	
	pNew->SetName(pszName);
	pNew->SetType(nType);
	pNew->SetRule(MMatchChannel::GetDefaultRule());
	pNew->SetRuleID(MMatchChannel::GetDefaultRuleID());
	pNew->SetMaxPlayers(DEFAULT_CHANNEL_MAXPLAYERS);
	
	if(Add(pNew, pOutUID) == false)
	{
		delete pNew;
		return false;
	}
	
	return true;
}

MMatchChannel *MMatchChannelManager::Get(const MUID &uidChannel)
{
	for(list<MMatchChannel *>::iterator it = m_Channels.begin(); 
		it != m_Channels.end(); it++)
	{
		MMatchChannel *pCurr = (*it);
		
		if(pCurr->GetUID() == uidChannel)
			return pCurr;
	}
	
	return NULL;
}

bool MMatchChannelManager::Remove(const MUID &uidChannel)
{
	for(list<MMatchChannel *>::iterator it = m_Channels.begin(); 
		it != m_Channels.end(); it++)
	{
		MMatchChannel *pCurr = (*it);
		
		if(pCurr->GetUID() == uidChannel)
		{
			delete pCurr;
			m_Channels.erase(it);
			return true;
		}
	}
	
	return false;
}

bool MMatchChannelManager::Remove(MMatchChannel *pChannel)
{
	return Remove(pChannel->GetUID());
}

bool MMatchChannelManager::Enum(unsigned long nChannelFlag, vector<MMatchChannel *> *pOut)
{
	for(list<MMatchChannel *>::iterator it = m_Channels.begin(); 
		it != m_Channels.end(); it++)
	{
		MMatchChannel *pCurr = (*it);
		
		unsigned long nCurrFlag = MCHANNEL_ENUMTYPE_PRESET;
		
		switch(pCurr->GetType())
		{
			case MCHANNEL_TYPE_PRESET			: nCurrFlag = MCHANNEL_ENUMTYPE_PRESET;			 break;
			case MCHANNEL_TYPE_USER				: nCurrFlag = MCHANNEL_ENUMTYPE_USER; 			 break;
			case MCHANNEL_TYPE_CLAN				: nCurrFlag = MCHANNEL_ENUMTYPE_CLAN; 			 break;
			case MCHANNEL_TYPE_DUELTOURNAMENT	: nCurrFlag = MCHANNEL_ENUMTYPE_DUELTOURNAMENT;	 break;
			case MCHANNEL_TYPE_CLASSIC			: nCurrFlag = MCHANNEL_ENUMTYPE_CLASSIC;		 break;
			case MCHANNEL_TYPE_BLITZKRIEG		: nCurrFlag = MCHANNEL_ENUMTYPE_BLITZKRIEG;		 break;
		}
		
		if((nChannelFlag & nCurrFlag) != 0)
			pOut->push_back(pCurr);
	}
	
	return true;
}

int MMatchChannelManager::ConvertChannelTypeNameToID(const char *pszTypeName)
{
	// for channel.xml.
	
	if(MStricmp(pszTypeName, "General") == 0)					return MCHANNEL_TYPE_PRESET;
	// else if(MStricmp(pszTypeName, "Individual") == 0)			return MCHANNEL_TYPE_USER;
	// else if(MStricmp(pszTypeName, "Clan") == 0)					return MCHANNEL_TYPE_CLAN;
	else if(MStricmp(pszTypeName, "Duel Tournament") == 0)		return MCHANNEL_TYPE_DUELTOURNAMENT;
	else if(MStricmp(pszTypeName, "Classic") == 0)				return MCHANNEL_TYPE_CLASSIC;
	else if(MStricmp(pszTypeName, "BlitzKrieg") == 0)			return MCHANNEL_TYPE_BLITZKRIEG;
	
	return MCHANNEL_TYPE_PRESET;
}