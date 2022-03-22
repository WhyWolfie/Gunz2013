#include "pch.h"

#include "MMatchServer_OnCommand.h"

#include "MMatchStage.h"
#include "MMatchAnnounce.h"
#include "MMatchClan.h"

#include "MServerSetting.h"
#include "MMatchObject.h"

#include "MMatchDBMgr.h"
#include "MAsyncDBProcess.h"

#include "MMatchQuest.h"

#include "MMatchDuelTournament.h"
#include "MMatchBlitzKrieg.h"

#include "MClientAcceptor.h"

void UpdateServer(unsigned long nNowTime)
{
	MCommandProcess();
		
	for(list<MMatchStage *>::iterator i = g_StageMgr.Begin(); i != g_StageMgr.End(); i++)
	{
		MMatchStage *pCurr = (*i);
		pCurr->Update(nNowTime);
	}
		
	g_Announcer.Announcement(nNowTime);
		
	g_ClanMgr.Update(nNowTime);
		
	g_DTQMgr.ProcessChallengers(nNowTime);
	g_BKQMgr.ProcessGroup(nNowTime);
		
	// d/c invalid acceptors.
	g_ClientAcceptor.KickInvalidClient();
		
	// d/c flooding client.
	for(list<MMatchObject *>::iterator i = g_ObjectMgr.Begin(); i != g_ObjectMgr.End(); i++)
	{
		MMatchObject *pCurr = (*i);
		
		/*
		if(pCurr->IsFlooding() == true)
		{
			pCurr->Disconnect();
		}
		*/
		pCurr->Update(nNowTime);
	}
		
	// db locator info.
	static unsigned long nNextServerStatusUpdateTime = 0;
	if(nNextServerStatusUpdateTime <= nNowTime)
	{
		// Db_UpdateServerStatus(g_ServerConfig.LocatorSetting.nServerID, g_ObjectMgr.Size());
		AsyncDb_UpdateServerStatus(g_ServerConfig.LocatorSetting.nServerID, g_ObjectMgr.Size());
		nNextServerStatusUpdateTime = nNowTime + 20000;	// 20 sec.
	}
		
	// db char ranking.
	static unsigned long nNextCharRankUpdateTime = 0;
	if(nNextCharRankUpdateTime <= nNowTime)
	{
		// Db_UpdateIndividualRanking();
		AsyncDb_UpdateIndividualRanking();
		nNextCharRankUpdateTime = nNowTime + 86400000;	// 1 day.
	}
		
	g_Quest.Run(nNowTime);
	g_DTRankingMgr.Run();
	
	g_BlitzShop.Run();
		
	// send queued data at once.
	SendQueuedData();
}