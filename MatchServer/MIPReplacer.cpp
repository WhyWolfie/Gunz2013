#include "pch.h"
#include "MIPReplacer.h"

MIPReplacer g_IPReplacer;

MIPReplacer::MIPReplacer()
{
}

MIPReplacer::~MIPReplacer()
{
	Clear();
}

void MIPReplacer::Add(unsigned long nIPFrom, unsigned long nIPTo)
{
	MIPRepNode *pNew = new MIPRepNode;
	pNew->nIPFrom = nIPFrom;
	pNew->nIPTo = nIPTo;
	m_vtIPs.push_back(pNew);
}

void MIPReplacer::Clear()
{
	for(vector<MIPRepNode *>::iterator i = m_vtIPs.begin(); i != m_vtIPs.end(); i++)
	{
		delete (*i);
	}
	m_vtIPs.clear();
}

bool MIPReplacer::Load()
{
	XMLDocument doc;
	doc.LoadFile("ipreplacer.xml");
	
	if(doc.Error() == true) 
	{
		mlog("Error at loading IP replacement list.");
		return false;
	}
	
	mlog("IP replacement contents loading...");
	
	XMLHandle handle(&doc);
	XMLElement *element = handle.FirstChildElement("XML").FirstChildElement("IP").ToElement();
	
	while(element != NULL)
	{
		if(element->Attribute("from") == NULL || element->Attribute("to") == NULL)
		{
			element = element->NextSiblingElement();
			continue;
		}
			
		unsigned long nIPFrom = Socket::InetAddr(element->Attribute("from"));
		unsigned long nIPTo = Socket::InetAddr(element->Attribute("to"));
		
		Add(nIPFrom, nIPTo);
		
		element = element->NextSiblingElement();
	}
	
	mlog("IP replacements have loaded.");
	
	return true;
}

// found = replaced ip, not found = base ip.
unsigned long MIPReplacer::Replace(unsigned long nIP)
{
	for(vector<MIPRepNode *>::iterator i = m_vtIPs.begin(); i != m_vtIPs.end(); i++)
	{
		MIPRepNode *pNode = (*i);
		if(pNode->nIPFrom == nIP) return pNode->nIPTo;
	}
	return nIP;
}