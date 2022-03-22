#include "pch.h"

#include "MMatchConstant.h"
#include "MMatchObject.h"

#include "MMessageID.h"

/*
	TODO : enhance its security.
*/

int CheckProposalResponderValidation(MMatchObject *pResponderObj)
{
	if(pResponderObj == NULL) return MERR_TARGET_USER_NOT_FOUND;
	if(pResponderObj->m_bCharInfoExist == false) return MERR_TARGET_USER_NOT_FOUND;
	
	if(pResponderObj->m_nPlace != MMP_LOBBY) return MERR_LADDER_MEMBER_NOT_READY;
	
	return MSG_OK;
}

void OnLadderProposal(const MUID &uidPlayer, int nProposalMode, int nRequestID, vector<MTD_ReplierNode> &vtResponders)
{
	if(nProposalMode < 0 || nProposalMode >= MPROPOSAL_END) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	// if(pObj->m_uidChannel == MUID(0, 0)) return;
	// if(pObj->m_uidStage != MUID(0, 0)) return;
	
	if(pObj->m_nPlace != MMP_LOBBY) return;
	
	vector<MMatchObject *> vtResponderObjs;
	
	for(vector<MTD_ReplierNode>::iterator i = vtResponders.begin(); i != vtResponders.end(); i++)
	{
		MTD_ReplierNode *pNode = &(*i);
		
		MMatchObject *pCurrObj = g_ObjectMgr.Get(pNode->szName);
		int nValidationResult = CheckProposalResponderValidation(pCurrObj);
		
		if(nValidationResult != MSG_OK)
		{
			MCmdWriter temp;
			temp.WriteInt(nValidationResult);
			temp.WriteInt(nProposalMode);
			temp.WriteInt(nRequestID);
			temp.Finalize(MC_MATCH_RESPONSE_PROPOSAL, MCFT_END);
			SendToClient(&temp, uidPlayer);
			
			return;
		}
		
		if(find(vtResponderObjs.begin(), vtResponderObjs.end(), pCurrObj) == vtResponderObjs.end())
		{
			vtResponderObjs.push_back(pCurrObj);
		}
	}
	
	switch(nProposalMode)
	{
		// TODO : ladder mode too?
		
		case MPROPOSAL_BLITZ_INVITE	:
		{
			if(pObj->m_nBlitzGroupID != 0) return;
		}
		break;
		
		default	:
		{
			return;
		}
		break;
	}
	
	for(vector<MMatchObject *>::iterator i = vtResponderObjs.begin(); i != vtResponderObjs.end(); i++)
	{
		MMatchObject *pResponderObj = (*i);
		
		MCmdWriter Cmd2Responder;
		Cmd2Responder.WriteMUID(uidPlayer);
		Cmd2Responder.StartBlob(sizeof(MTD_ReplierNode));
		for(vector<MTD_ReplierNode>::iterator j = vtResponders.begin(); j != vtResponders.end(); j++)
		{
			Cmd2Responder.WriteData(&(*j), sizeof(MTD_ReplierNode));
		}
		Cmd2Responder.EndBlob();
		Cmd2Responder.WriteInt(nProposalMode);
		Cmd2Responder.WriteInt(nRequestID);
		Cmd2Responder.Finalize(MC_MATCH_ASK_AGREEMENT, MCFT_END);
		SendToClient(&Cmd2Responder, pResponderObj->GetUID());
	}
	
	MCmdWriter Cmd2Proposer;
	Cmd2Proposer.WriteInt(MSG_OK);
	Cmd2Proposer.WriteInt(nProposalMode);
	Cmd2Proposer.WriteInt(nRequestID);
	Cmd2Proposer.Finalize(MC_MATCH_RESPONSE_PROPOSAL, MCFT_END);
	SendToClient(&Cmd2Proposer, uidPlayer);
}

// for 2011 or under.
void OnLadderProposalAgreement(const MUID &uidProposer, const MUID &uidResponder, const char *pszResponderName, int nProposalMode, int nRequestID, bool bAnswer)
{
	MCmdWriter Cmd;
	Cmd.WriteMUID(uidProposer);
	Cmd.WriteMUID(uidResponder);
	Cmd.WriteString(pszResponderName);
	Cmd.WriteInt(nProposalMode);
	Cmd.WriteInt(nRequestID);
	Cmd.WriteBool(bAnswer);
	Cmd.Finalize(MC_MATCH_REPLY_AGREEMENT, MCFT_END);
	SendToClient(&Cmd, uidProposer);
}

// for 2012 or above.
void OnLadderProposalAgreement(const MUID &uidProposer, const MUID &uidResponder, const char *pszResponderName, vector<MTD_ReplierNode> &vtResponders, int nProposalMode, int nRequestID, bool bAnswer, bool bBusy)
{
	MCmdWriter Cmd;
	Cmd.WriteMUID(uidProposer);
	Cmd.WriteMUID(uidResponder);
	Cmd.WriteString(pszResponderName);
	Cmd.StartBlob(sizeof(MTD_ReplierNode));
	for(vector<MTD_ReplierNode>::iterator i = vtResponders.begin(); i != vtResponders.end(); i++)
	{
		Cmd.WriteData(&(*i), sizeof(MTD_ReplierNode));
	}
	Cmd.EndBlob();
	Cmd.WriteInt(nProposalMode);
	Cmd.WriteInt(nRequestID);
	Cmd.WriteBool(bAnswer);
	Cmd.WriteBool(bBusy);
	Cmd.Finalize(MC_MATCH_REPLY_AGREEMENT, MCFT_END);
	SendToClient(&Cmd, uidProposer);
}