#include "pch.h"
#include "MServerMessage.h"

#define MXML_SERVERMESSAGE_FILENAME		"serverstring.xml"

MServerMessage g_ServerMessage;

MServerMessage::MServerMessage()
{
}

MServerMessage::~MServerMessage()
{
	m_Messages.clear();
}

bool MServerMessage::Load()
{
	XMLDocument doc;
	
	if(doc.LoadFile(MXML_SERVERMESSAGE_FILENAME) != XML_NO_ERROR)
	{
		mlog("Can't open %s.", MXML_SERVERMESSAGE_FILENAME);
		return false;
	}
	
	mlog("------> Loading server messages...");
	
	XMLHandle handle(&doc);
	XMLElement *element = handle.FirstChildElement("XML").FirstChildElement("STR").ToElement();
	
	while(element != NULL)
	{
		int nMessageID = element->IntAttribute("id");
		
		if(nMessageID != 0)
		{
			const char *pszMessageText = element->GetText();
			
			if(pszMessageText != NULL)
			{
				m_Messages.insert(pair<int, string>(nMessageID, (string)pszMessageText));
			}
		}
		
		element = element->NextSiblingElement();
	}
	
	mlog("------> Loaded %d server messages.", (int)m_Messages.size());
	
	return true;
}

const char *MServerMessage::Get(int nMsgID)
{
	map<int, string>::iterator it = m_Messages.find(nMsgID);
	
	if(it != m_Messages.end())
	{
		return (*it).second.c_str();
	}
	
	return GetDefaultString(nMsgID);
}

const char *MServerMessage::GetDefaultString(int nMsgID)
{
	// defaults.
	
	switch(nMsgID)
	{
		case MSVRSTR_REMAINED_TIME_SEC_STRING_FORMAT	:
			return "%lu seconds remaining.";
			
		case MSVRSTR_REMAINED_TIME_MIN_SEC_STRING_FORMAT	:
			return "%lu min %lu sec remaining.";
	}
	
	return "StringErr";
}
