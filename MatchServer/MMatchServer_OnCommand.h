#ifndef __MMATCHSERVER_ONCOMMAND_H__
#define __MMATCHSERVER_ONCOMMAND_H__

// login.
bool OnMatchLogin(const MUID &uidPlayer, const char *pszUserID, const char *pszPassword, int nCommandVersion);
void OnAccountCharList(const MUID &uidPlayer);
void OnAccountCharInfo(const MUID &uidPlayer, int nCharNum);
void OnCreateCharacter(const MUID &uidPlayer, int nCharIndex, const char *pszName, int nSex, int nHair, int nFace, int nCostume);
void OnDeleteCharacter(const MUID &uidPlayer, int nCharIndex, const char *pszCharName);
void OnSelectCharacter(const MUID &uidPlayer, int nCharIndex);

// channel.
void OnAppropriateChannel(const MUID &uidPlayer);
void OnChannelJoin(const MUID &uidPlayer, const MUID &uidChannel);
void OnPrivateChannelJoin(const MUID &uidPlayer, int nChannelType, const char *pszChannelName);
void OnChannelPlayerList(const MUID &uidPlayer, const MUID &uidChannel, int nPage, bool bChecksum = true);
void OnChannelChat(const MUID &uidPlayer, const MUID &uidChannel, const char *pszChat);
void OnChannelListStart(const MUID &uidPlayer, int nChannelType);
void OnChannelListStop(const MUID &uidPlayer);

// character.
void OnUserOption(const MUID &uidPlayer, unsigned long nFlag);
void OnUserWhisper(const MUID &uidSender, const char *pszTarget, const char *pszMessage);
void OnCharacterItemList(const MUID &uidPlayer);
void OnMySimpleCharInfo(const MUID &uidPlayer);
void OnUserWhere(const MUID &uidPlayer, const char *pszTargetName);

// item.
void OnShopItemList(const MUID &uidPlayer);
void OnBuyItem(const MUID &uidPlayer, int nItemID, int nItemCount);	// 2011 ver.
void OnBuyItem(const MUID &uidPlayer, int nCurrencyType, int nItemID, int nItemCount, const char *pszGiftName, const char *pszGiftMsg, int nRentHourPeriod);	// 2012 ver.
void OnSellItem(const MUID &uidPlayer, const MUID &uidItem, int nItemCount);
void OnEquipItem(const MUID &uidPlayer, const MUID &uidItem, int nSlot);
void OnTakeoffItem(const MUID &uidPlayer, int nSlot);
void OnOpenGambleItem(const MUID &uidPlayer, const MUID &uidItem);
void OnAccountItemList(const MUID &uidPlayer);
void OnTakeOutItemFromStorage(const MUID &uidPlayer, int nAIID, int nItemID, int nItemCount);
void OnDepositItemToStorage(const MUID &uidPlayer, const MUID &uidItem, int nItemCount);
void OnCashShopItemList(const MUID &uidPlayer, int nSex, int nSortType);
void OnCashShopItemInfo(const MUID &uidPlayer, int nSex, int nSortType, int nStartIndex, int nCount);
void OnSyncCashRemainder(const MUID &uidPlayer);
void OnCheckGiftItem(const MUID &uidPlayer);
void OnReadGiftItem(const MUID &uidPlayer, vector<int> &vtGiftID);

// stage and game.
void OnStageList(const MUID &uidPlayer, const MUID &uidChannel, int nStageCursor, bool bChecksum = true);
void OnCreateStage(const MUID &uidPlayer, const char *pszStageName, const char *pszPassword, bool bPrivate = false);
void OnPrivateStageJoin(const MUID &uidPlayer, const MUID &uidStage, const char *pszPassword = NULL);
void OnStageJoin(const MUID &uidPlayer, const MUID &uidStage);
void OnStageLeave(const MUID &uidPlayer);
// void OnStageChat(const MUID &uidPlayer, const MUID &uidStage, const char *pszChat);
void OnStageChat(const MUID &uidPlayer, const char *pszChat);
void OnStageSetting(const MUID &uidPlayer, const MUID &uidStage);
void OnChangeStageSetting(const MUID &uidPlayer, const MUID &uidStage, const MSTAGE_SETTING_NODE *pNode);
void OnStageMap(const MUID &uidPlayer, const MUID &uidStage, const char *pszMapName);
void OnRelayMapElementUpdate(const MUID &uidPlayer, const MUID &uidStage, int nRelayMapType, int nRelayMapRepeat);
void OnRelayMapInfoUpdate(const MUID &uidPlayer, const MUID &uidStage, int nRelayMapType, int nRelayMapRepeat, const MTD_RelayMap *pRelayMapList, int nCount);
void OnStageTeam(const MUID &uidPlayer, const MUID &uidStage, int nTeam);
void OnStageState(const MUID &uidPlayer, const MUID &uidStage, int nState);
// void OnStageStart(const MUID &uidPlayer, const MUID &uidStage, int nCountdown = 0);
void OnStageStart(const MUID &uidPlayer, int nCountdown = 0);
void OnLoadingComplete(const MUID &uidPlayer, int nPercent);
void OnTimeSync(const MUID &uidPlayer, unsigned long nLocalTimeStamp);
void OnReportTimeSync(const MUID &uidPlayer, unsigned long nLocalTimeStamp, unsigned long nDataChecksum);
void OnStageEnterBattle(const MUID &uidPlayer, const MUID &uidStage);
void OnPeerList(const MUID &uidPlayer, const MUID &uidStage);
void OnStageLeaveBattle(const MUID &uidPlayer, bool bFinishLeave);
// void OnForceEntry(const MUID &uidPlayer, const MUID &uidStage);
void OnForceEntry(const MUID &uidPlayer);
void OnGameInfo(const MUID &uidPlayer, const MUID &uidStage);
void OnGameSpawn(const MUID &uidPlayer, const FloatVector *pPos, const FloatVector *pDir);
void OnSpawnWorldItem(const MUID &uidPlayer, int nItemID, const FloatVector *pPos, float fDropTime);
void OnObtainWorldItem(const MUID &uidPlayer, int nItemUID);
void OnGameKill(const MUID &uidAttacker, const MUID &uidVictim);
void OnGameSuicide(const MUID &uidPlayer);
void OnCallvote(const MUID &uidPlayer, const char *pszDiscuss, const char *pszArg);
void OnVoteYes(const MUID &uidPlayer);
void OnVoteNo(const MUID &uidPlayer);
void OnVote(const MUID &uidPlayer, bool bAgreed);
void OnStageFollow(const MUID &uidPlayer, const char *pszTargetName);
void OnStageQuickJoin(const MUID &uidPlayer, const MTD_QuickJoinParam *pParam);
void OnStageGo(const MUID &uidPlayer, int nStageNo);
void OnUseNormalSpendableItem(const MUID &uidPlayer, const MUID &uidItem);
void OnStageKick(const MUID &uidPlayer, const char *pszTargetName);

// clan.
void OnCreateClan(const MUID &uidPlayer, const char *pszClanName);
void OnJoinClan(const MUID &uidPlayer, const char *pszClanName, const char *pszJoinerName);
void OnAnswerJoinClanAgreement(const MUID &uidPlayer, const MUID &uidClanAdmin, const char *pszJoinerName, bool bAnswer);
void OnJoinClanAgreed(const MUID &uidPlayer, const char *pszClanName, const char *pszJoinerName);
void OnChangeClanMemberGrade(const MUID &uidPlayer, const char *pszMemberName, int nGrade);
void OnExpelClanMember(const MUID &uidPlayer, const char *pszMemberName);
void OnLeaveClan(const MUID &uidPlayer);
void OnCloseClan(const MUID &uidPlayer, const char *pszClanName);
void OnClanMemberList(const MUID &uidPlayer);
void OnClanMsg(const MUID &uidPlayer, const char *pszMsg);
void OnClanInfo(const MUID &uidPlayer, const char *pszClanName);
void OnClanEmblemURL(const MUID &uidPlayer, vector<int> *pClanIDs);

// friend.
void OnFriendList(const MUID &uidPlayer);
void OnFriendAdd(const MUID &uidPlayer, const char *pszCharName);
void OnFriendRemove(const MUID &uidPlayer, const char *pszCharName);

// chat room.
void OnChatRoomCreate(const MUID &uidPlayer, const char *pszChatRoomName);
void OnChatRoomJoin(const MUID &uidPlayer, const char *pszChatRoomName);
void OnChatRoomLeave(const MUID &uidPlayer, const char *pszChatRoomName);
void OnChatRoomSelect(const MUID &uidPlayer, const char *pszChatRoomName);
void OnChatRoomInvite(const MUID &uidPlayer, const char *pszTargetName);
void OnChatRoomChat(const MUID &uidPlayer, const char *pszMsg);

// quest.
void OnCharQuestItemList(const MUID &uidPlayer);
void OnBuyQuestItem(const MUID &uidPlayer, int nItemID, int nCount);
void OnSellQuestItem(const MUID &uidPlayer, int nItemID, int nCount);
void OnQuestSlotInfo(const MUID &uidPlayer);
void OnQuestLevel(const MUID &uidPlayer);
void OnQuestDropSacrificeItem(const MUID &uidPlayer, int nSlotIndex, int nItemID);
void OnQuestCallbackSacrificeItem(const MUID &uidPlayer, int nSlotIndex, int nItemID);
void OnQuestPlayerDead(const MUID &uidPlayer);
void OnQuestNPCDead(const MUID &uidPlayer, const MUID &uidKiller, const MUID &uidNPC, const ShortVector *pPos);
void OnMoveToPortal(const MUID &uidPlayer, int nCurrSectorIndex);
void OnReadyToNewSector(const MUID &uidPlayer);

// duel tournament.
void OnDuelTournamentRankingInfo(const MUID &uidPlayer);
void OnDuelTournamentJoin(const MUID &uidPlayer, int nTournamentType);
void OnDuelTournamentCancel(const MUID &uidPlayer, int nTournamentType);
void OnDuelTournamentGamePlayerStatus(const MUID &uidPlayer, float fDamagedPoint, float fHP, float fAP);

// challenge quest.
void OnChallengeQuestBestRecord(const MUID &uidPlayer);
void OnChallengeQuestNPCDead(const MUID &uidPlayer, const MUID &uidKiller, const MUID &uidNPC, const ShortVector *pPos);
void OnChallengeQuestPlayerDead(const MUID &uidPlayer);
void OnChallengeQuestNPCSpawn(const MUID &uidPlayer, const MUID &uidOwnerNPC, const char *pszActorName, const FloatVector *pPos, const FloatVector *pDir);

// blitzkrieg.
void OnBlitzMatchCollaboratorList(const MUID &uidPlayer);
void OnBlitzReserveChallenge(const MUID &uidPlayer, vector<MTD_BlitzChallengerNode> *pChallengers);
void OnBlitzCancelChallenge(const MUID &uidPlayer);
void OnBlitzChooseClass(const MUID &uidPlayer, unsigned int nClass);
void OnBlitzNPCDead(const MUID &uidPlayer, const MUID &uidKiller, const MUID &uidNPC);
void OnBlitzPlayerDead(const MUID &uidAttacker, const MUID &uidVictim, vector<MTD_BlitzAssisterInfoNode> *pAssisters = NULL);
void OnBlitzSkillUpgrade(const MUID &uidPlayer, int nSkillType);

// ladder.
void OnLadderProposal(const MUID &uidPlayer, int nProposalMode, int nRequestID, vector<MTD_ReplierNode> &vtResponders);
void OnLadderProposalAgreement(const MUID &uidProposer, const MUID &uidResponder, const char *pszResponderName, int nProposalMode, int nRequestID, bool bAnswer);
void OnLadderProposalAgreement(const MUID &uidProposer, const MUID &uidResponder, const char *pszResponderName, vector<MTD_ReplierNode> &vtResponders, int nProposalMode, int nRequestID, bool bAnswer, bool bBusy);

// spy.
void OnSpyActivateMap(const MUID &uidPlayer, int nMapID, bool bDeActivate);
void OnSpyStageStart(const MUID &uidPlayer, vector<int> &vtMapList);

// agent.
void OnPeerRelay(const MUID &uidPlayer, const MUID &uidPeer);
void OnAgentRegister(const MUID &uidAgent, const char *pszIP, int nPort, int nUDPPort);
void OnAgentUnregister(const MUID &uidAgent);
void OnAgentStageReady(const MUID &uidAgent, const MUID &uidStage);
void OnAgentPeerReady(const MUID &uidAgent, const MUID &uidPlayer, const MUID &uidPeer);
void OnLiveCheck(const MUID &uidAgent, unsigned long nTimestamp, unsigned int nStageCount, unsigned int nUserCount);

// admin.
void OnAdminAnnounce(const MUID &uidAdmin, const char *pszMsg, unsigned int nMsgType);
void OnAdminHalt(const MUID &uidAdmin);
void OnAdminKick(const MUID &uidAdmin, const char *pszTargetName);
void OnAdminHide(const MUID &uidAdmin);
void OnAdminAssassin(const MUID &uidAdmin);

// main.
void MCommandProcess();
bool MCommandAdd(vector<MCommandReader *> *pCmdList);

#endif
