#include "pch.h"
#include "MMatchSpy.h"

#include "MMatchObject.h"
#include "MMatchStage.h"

#include "MMatchServer_OnCommand.h"

// from mmatchserver_stage.
void CreateStageSettingCommand(MMatchStage *pStage, MCommandWriter *pOut);

void OnSpyActivateMap(const MUID &uidPlayer, int nMapID, bool bDeActivate)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_uidStage == MUID(0, 0)) return;
	
	MMatchStage *pStage = g_StageMgr.Get(pObj->m_uidStage);
	if(pStage == NULL) return;
	
	if(pStage->IsMaster(uidPlayer) == false) return;
	if(pStage->IsGameLaunched() == true) return;
	
	if(pStage->GetGameType() != (int)MMGT_SPY) return;
	
	if(bDeActivate == true)
	{
		pStage->DeActivateSpyMap(nMapID);
	}
	else
	{
		pStage->ActivateSpyMap(nMapID);
	}
	
	// pStage->SendSpyMapInfo();
	
	MCmdWriter Cmd;
	Cmd.WriteInt(nMapID);
	Cmd.WriteBool(bDeActivate);
	Cmd.Finalize(MC_SPY_STAGE_BAN_MAP, MCFT_END);
	SendToStage(&Cmd, pObj->m_uidStage);
}

void OnSpyStageStart(const MUID &uidPlayer, vector<int> &vtMapList)
{
	if(vtMapList.size() == 0) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_uidStage == MUID(0, 0)) return;
	
	MMatchStage *pStage = g_StageMgr.Get(pObj->m_uidStage);
	if(pStage == NULL) return;
	
	if(pStage->IsMaster(uidPlayer) == false) return;
	if(pStage->IsGameLaunched() == true) return;
	
	if(pStage->GetGameType() != (int)MMGT_SPY) return;
	
	int nRandIndex = (int)(RandNum() % (unsigned int)vtMapList.size());
	int nMapID = vtMapList[nRandIndex];
	
	MMatchSpyMap *pSpyMap = g_SpyMapSetting.GetMap(nMapID);
	if(pSpyMap == NULL) return;
	
	pStage->SetMap(nMapID);
	pStage->SetTimeLimit(pSpyMap->nLimitTime * 1000);
	
	MCmdWriter CmdStageSetting;
	CreateStageSettingCommand(pStage, &CmdStageSetting);
	SendToStage(&CmdStageSetting, pObj->m_uidStage);
	
	OnStageStart(uidPlayer, 0);
}
