#include "pch.h"

#include "MServerSetting.h"
#include "MServerMessage.h"

#include "MMatchChannel.h"
#include "MMatchItem.h"
#include "MMatchMap.h"
#include "MMatchExp.h"

#include "MMatchAnnounce.h"
#include "MIPReplacer.h"

#include "MMatchQuest.h"
#include "MMatchChallengeQuest.h"

#include "MMatchGambleItem.h"

#include "MMatchDuelTournament.h"
#include "MMatchBlitzKrieg.h"

#include "MMatchSpy.h"

#include "MMatchCashShop.h"

MConfig g_ServerConfig;

bool LoadServerConfig()
{
	XMLDocument doc;
	doc.LoadFile(MCONFIG_FILENAME);

	if(doc.Error() == true)
	{
		MLog("Error loading %s ...", MCONFIG_FILENAME);
		return false;
	}

	MLog("------ Parse %s file started. ------", MCONFIG_FILENAME);

	XMLHandle handle(&doc);
	XMLElement *element;

	element = handle.FirstChildElement("XML").FirstChildElement("SERVER").FirstChildElement("PORT").ToElement();
	if(element != NULL && element->GetText() != NULL)
		g_ServerConfig.ServerSetting.nPort = (unsigned short)ToInt(element->GetText());
	else
	{
		g_ServerConfig.ServerSetting.nPort = MCONFIG_DEFAULT_SERVER_TCPPORT;
		MLog("server tcp port not found. defaulted to %d.", MCONFIG_DEFAULT_SERVER_TCPPORT);
	}

	element = handle.FirstChildElement("XML").FirstChildElement("SERVER").FirstChildElement("UDPPORT").ToElement();
	if(element != NULL && element->GetText() != NULL)
		g_ServerConfig.ServerSetting.nUdpPort = (unsigned short)ToInt(element->GetText());
	else
	{
		g_ServerConfig.ServerSetting.nUdpPort = MCONFIG_DEFAULT_SERVER_UDPPORT;
		MLog("server udp port not found. defaulted to %d.", MCONFIG_DEFAULT_SERVER_UDPPORT);
	}

	element = handle.FirstChildElement("XML").FirstChildElement("SERVER").FirstChildElement("MAXPLAYERS").ToElement();
	if(element != NULL && element->GetText() != NULL)
		g_ServerConfig.ServerSetting.nMaxPlayers = (unsigned short)ToInt(element->GetText());
	else
	{
		g_ServerConfig.ServerSetting.nMaxPlayers = MCONFIG_DEFAULT_SERVER_MAXPLAYERS;
		MLog("max players is not set. defaulted to %d.", MCONFIG_DEFAULT_SERVER_MAXPLAYERS);
	}

	element = handle.FirstChildElement("XML").FirstChildElement("SERVER").FirstChildElement("NAME").ToElement();
	if(element != NULL && element->GetText() != NULL)
		strcpy(g_ServerConfig.ServerSetting.szName, element->GetText());
	else
	{
		strcpy(g_ServerConfig.ServerSetting.szName, MCONFIG_DEFAULT_SERVER_NAME);
		MLog("unknown server name. defaulting %s.", MCONFIG_DEFAULT_SERVER_NAME);
	}

	element = handle.FirstChildElement("XML").FirstChildElement("SERVER").FirstChildElement("SURVIVAL").ToElement();
	if(element != NULL && element->GetText() != NULL)
		g_ServerConfig.ServerSetting.bSurvival =
		    (MStricmp(element->GetText(), "1") == 0 || MStricmp(element->GetText(), "true") == 0 || MStricmp(element->GetText(), "yes") == 0) ?
		    true : false;
	else
	{
		g_ServerConfig.ServerSetting.bSurvival = MCONFIG_DEFAULT_SERVER_SURVIVAL;
		MLog("survival not set. defaulting %s.", MCONFIG_DEFAULT_SERVER_SURVIVAL == false ? "False" : "True");
	}

	element = handle.FirstChildElement("XML").FirstChildElement("SERVER").FirstChildElement("DUELTOURNAMENT").ToElement();
	if(element != NULL && element->GetText() != NULL)
		g_ServerConfig.ServerSetting.bDuelTournament =
		    (MStricmp(element->GetText(), "1") == 0 || MStricmp(element->GetText(), "true") == 0 || MStricmp(element->GetText(), "yes") == 0) ?
		    true : false;
	else
	{
		g_ServerConfig.ServerSetting.bDuelTournament = MCONFIG_DEFAULT_SERVER_DUELTOURNAMENT;
		MLog("duel tournament not set. defaulting %s.", MCONFIG_DEFAULT_SERVER_DUELTOURNAMENT == false ? "False" : "True");
	}

	element = handle.FirstChildElement("XML").FirstChildElement("SERVER").FirstChildElement("SERVERMODE").ToElement();
	if(element != NULL && element->GetText() != NULL)
	{
		if(MStricmp(element->GetText(), MSM_NORMAL_STR) == 0)
			g_ServerConfig.ServerSetting.nServerMode = MSM_NORMAL;
			
		else if(MStricmp(element->GetText(), MSM_CLAN_STR) == 0)
			g_ServerConfig.ServerSetting.nServerMode = MSM_CLAN;
			
		else if(MStricmp(element->GetText(), MSM_QUEST_STR) == 0)
			g_ServerConfig.ServerSetting.nServerMode = MSM_QUEST;
			
		else if(MStricmp(element->GetText(), MSM_BOTH_STR) == 0)
			g_ServerConfig.ServerSetting.nServerMode = MSM_BOTH;
			
		else
		{
			// error config.
			g_ServerConfig.ServerSetting.nServerMode = MCONFIG_DEFAULT_SERVER_MODE;
			MLog("invalid server mode. defaulting to %d...", MCONFIG_DEFAULT_SERVER_MODE);
		}
	}
	else
	{
		g_ServerConfig.ServerSetting.nServerMode = MCONFIG_DEFAULT_SERVER_MODE;
		MLog("no server mode. default $d.", MCONFIG_DEFAULT_SERVER_MODE);
	}
	
	// ----------------------------------------
	
	element = handle.FirstChildElement("XML").FirstChildElement("DATABASE").FirstChildElement("USERNAME").ToElement();
	if(element != NULL && element->GetText() != NULL)
		strcpy(g_ServerConfig.DatabaseSetting.szUsername, element->GetText());
	else
	{
		strcpy(g_ServerConfig.DatabaseSetting.szUsername, MCONFIG_DEFAULT_DATABASE_USERNAME);
		MLog("DB username is not set. defaulting %s...", MCONFIG_DEFAULT_DATABASE_USERNAME);
	}
	
	element = handle.FirstChildElement("XML").FirstChildElement("DATABASE").FirstChildElement("PASSWORD").ToElement();
	if(element != NULL && element->GetText() != NULL)
		strcpy(g_ServerConfig.DatabaseSetting.szPassword, element->GetText());
	else
	{
		strcpy(g_ServerConfig.DatabaseSetting.szPassword, MCONFIG_DEFAULT_DATABASE_PASSWORD);
		MLog("DB password is not set. defaulting %s...", MCONFIG_DEFAULT_DATABASE_PASSWORD);
	}
	
	element = handle.FirstChildElement("XML").FirstChildElement("DATABASE").FirstChildElement("DSN").ToElement();
	if(element != NULL && element->GetText() != NULL)
		strcpy(g_ServerConfig.DatabaseSetting.szDSN, element->GetText());
	else
	{
		strcpy(g_ServerConfig.DatabaseSetting.szDSN, MCONFIG_DEFAULT_DATABASE_DSN);
		MLog("DB dsn not set. defaulting %s...", MCONFIG_DEFAULT_DATABASE_DSN);
	}
	
	// ----------------------------------------
	
	element = handle.FirstChildElement("XML").FirstChildElement("AGENT").FirstChildElement("TRUSTED_IP").ToElement();
	if(element != NULL && element->GetText() != NULL)
		strcpy(g_ServerConfig.AgentSetting.szTrustedIP, element->GetText());
	else
	{
		strcpy(g_ServerConfig.AgentSetting.szTrustedIP, MCONFIG_DEFAULT_AGENT_TRUSTED_IP);
		MLog("Agent's trusted IP setting is empty. defaulted to %s.", MCONFIG_DEFAULT_AGENT_TRUSTED_IP);
	}
	
	// ----------------------------------------
	
	element = handle.FirstChildElement("XML").FirstChildElement("LOCATOR").FirstChildElement("SERVERID").ToElement();
	if(element != NULL && element->GetText() != NULL)
		g_ServerConfig.LocatorSetting.nServerID = ToInt(element->GetText());
	else
	{
		g_ServerConfig.LocatorSetting.nServerID = MCONFIG_DEFAULT_LOCATOR_SERVERID;
		MLog("locator server id is not set. defaulting to %d.", MCONFIG_DEFAULT_LOCATOR_SERVERID);
	}
	
	element = handle.FirstChildElement("XML").FirstChildElement("LOCATOR").FirstChildElement("IP").ToElement();
	if(element != NULL && element->GetText() != NULL)
		strcpy(g_ServerConfig.LocatorSetting.szIP, element->GetText());
	else
	{
		strcpy(g_ServerConfig.LocatorSetting.szIP, MCONFIG_DEFAULT_LOCATOR_SERVERIP);
		MLog("Locator IP not setted. defaulting to %s.", MCONFIG_DEFAULT_LOCATOR_SERVERIP);
	}
	
	element = handle.FirstChildElement("XML").FirstChildElement("LOCATOR").FirstChildElement("TYPE").ToElement();
	if(element != NULL && element->GetText() != NULL)
	{
		if(MStricmp(element->GetText(), MLOCATOR_SERVERTYPE_DEBUG_STR) == 0)
			g_ServerConfig.LocatorSetting.nType = MLOCATOR_SERVERTYPE_DEBUG;
			
		else if(MStricmp(element->GetText(), MLOCATOR_SERVERTYPE_MATCH_STR) == 0)
			g_ServerConfig.LocatorSetting.nType = MLOCATOR_SERVERTYPE_MATCH;
			
		else if(MStricmp(element->GetText(), MLOCATOR_SERVERTYPE_CLAN_STR) == 0)
			g_ServerConfig.LocatorSetting.nType = MLOCATOR_SERVERTYPE_CLAN;
			
		else if(MStricmp(element->GetText(), MLOCATOR_SERVERTYPE_QUEST_STR) == 0)
			g_ServerConfig.LocatorSetting.nType = MLOCATOR_SERVERTYPE_QUEST;
			
		else if(MStricmp(element->GetText(), MLOCATOR_SERVERTYPE_EVENT_STR) == 0)
			g_ServerConfig.LocatorSetting.nType = MLOCATOR_SERVERTYPE_EVENT;
			
		else if(MStricmp(element->GetText(), MLOCATOR_SERVERTYPE_UNKNOWN_STR) == 0)
			g_ServerConfig.LocatorSetting.nType = MLOCATOR_SERVERTYPE_UNKNOWN;
			
		else
		{
			g_ServerConfig.LocatorSetting.nType = MCONFIG_DEFAULT_LOCATOR_SERVERTYPE;
			MLog("locator type is invalid. defaulted to %d.", MCONFIG_DEFAULT_LOCATOR_SERVERTYPE);
		}
	}
	else
	{
		g_ServerConfig.LocatorSetting.nType = MCONFIG_DEFAULT_LOCATOR_SERVERTYPE;
		MLog("Setting of Locator type is not found. defaulting to %d.", MCONFIG_DEFAULT_LOCATOR_SERVERTYPE);
	}
	
	// ----------------------------------------
	
	element = handle.FirstChildElement("XML").FirstChildElement("OTHER").FirstChildElement("IPBAN").ToElement();
	if(element != NULL && element->GetText() != NULL)
		g_ServerConfig.OtherSetting.bIpBan = 
		    (MStricmp(element->GetText(), "1") == 0 || MStricmp(element->GetText(), "true") == 0 || MStricmp(element->GetText(), "yes") == 0) ?
		    true : false;
	else
	{
		g_ServerConfig.OtherSetting.bIpBan = MCONFIG_DEFAULT_OTHER_IPBAN;
		MLog("ip ban config not found. defaulted to %s.", MCONFIG_DEFAULT_OTHER_IPBAN == false ? "False" : "True");
	}
	
	element = handle.FirstChildElement("XML").FirstChildElement("OTHER").FirstChildElement("IPACCOUNT").ToElement();
	if(element != NULL && element->GetText() != NULL)
		g_ServerConfig.OtherSetting.bIpAccount = 
		    (MStricmp(element->GetText(), "1") == 0 || MStricmp(element->GetText(), "true") == 0 || MStricmp(element->GetText(), "yes") == 0) ?
		    true : false;
	else
	{
		g_ServerConfig.OtherSetting.bIpAccount = MCONFIG_DEFAULT_OTHER_IPACCOUNT;
		MLog("ip account config not found. defaulted to %s.", MCONFIG_DEFAULT_OTHER_IPACCOUNT == false ? "False" : "True");
	}
	
	// ----------------------------------------
	
// #ifdef _DEBUG
	MLog("  >>>>>> %s (%d port) : %d players allowed.", 
	      g_ServerConfig.ServerSetting.szName, 
			g_ServerConfig.ServerSetting.nPort, 
			  g_ServerConfig.ServerSetting.nMaxPlayers);
	MLog("  >>>>>> [ODBC connection] username %s - password %s - dsn %s", 
	      g_ServerConfig.DatabaseSetting.szUsername, 
		    g_ServerConfig.DatabaseSetting.szPassword, 
			  g_ServerConfig.DatabaseSetting.szDSN);
// #endif
	
	MLog("------ Ended. ------");
	
	return true;
}

bool LoadServerXML()
{
	// serverconf.xml
	if(LoadServerConfig() == false)
		return false;
		
	if(g_ServerMessage.Load() == false)
	{
		// nothing.
	}
		
	// channel.xml
	if(g_ChannelMgr.LoadPreset() == false)
		return false;
	
	if(g_ItemMgr.LoadZItem(MXML_FILENAME_ZITEM) == false) return false;
	if(g_ItemMgr.LoadZItem(MXML_FILENAME_ZITEM_LOCALE) == false) return false;
	
	if(g_ItemMgr.LoadZQuestItem() == false) return false;
	
	if(g_ShopMgr.Load() == false)
		return false;
	
	if(g_MapMgr.Load() == false)
		return false;
		
	if(g_FormulaMgr.Load() == false)
		return false;
		
	if(g_Announcer.LoadAnnounceList() == false)
	{
		// do nothing.
	}
	
	if(g_IPReplacer.Load() == false)
	{
		// do nothing.
	}
	
	// quest & survival.
	if(g_Quest.LoadNPCInfo() == false)
		return false;
		
	if(g_Quest.LoadScenario(MXML_QUEST_SCENARIO_FILENAME) == false) return false;
	if(g_Quest.LoadScenario(MXML_SURVIVAL_SCENARIO_FILENAME) == false) return false;
	
	if(g_Quest.LoadNPCDropItem() == false) return false;
		
	// challenge quest.
	if(g_ChallengeQuest.LoadNPCInfo() == false) return false;
	if(g_ChallengeQuest.LoadScenario() == false) return false;
	
	if(g_GItemMgr.Load() == false)
	{
		// do nothing.
	}
	
	if(g_DTRankingMgr.LoadTime() == false) return false;
	
	if(g_BlitzShop.LoadXML() == false) return false;
	if(g_BlitzShop.LoadRestockTime() == false) return false;
	
	#ifdef _DEBUG
	{
		// for debug : generate shop.xml with all items.
		
		XMLDocument shop;
		
		XMLElement *main = shop.NewElement("XML");
		shop.InsertEndChild(main);
		
		for(vector<MMatchItem *>::iterator i = g_ItemMgr.Begin(); i != g_ItemMgr.End(); i++)
		{
			MMatchItem *pCurr = (*i);
			
			XMLElement *sub = shop.NewElement("SELL");
			sub->SetAttribute("itemid", pCurr->GetID());
			
			main->InsertEndChild(sub);
		}
		
		shop.SaveFile("shop_all.xml");
	}
	#endif
	
	if(LoadSpyModeXML() == false) return false;
	if(LoadSpyMapXML() == false) return false;
	
	if(g_CashShop.Load() == false) return false;
	
	return true;
}
