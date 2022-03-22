#include "pch.h"

void BuildAnnounceCommand(const char *pszMsg, MCommandWriter *pOut)
{
	pOut->WriteUInt(0);
	pOut->WriteString(pszMsg);
	pOut->Finalize(MC_MATCH_ANNOUNCE, MCFT_END | MCFT_RAW);
}

void AnnounceToClient(const char *pszMsg, const MUID &uidPlayer)
{
	MCmdWriter Cmd;
	BuildAnnounceCommand(pszMsg, &Cmd);
	SendToClient(&Cmd, uidPlayer);
}

void AnnounceToStage(const char *pszMsg, const MUID &uidStage)
{
	MCmdWriter Cmd;
	BuildAnnounceCommand(pszMsg, &Cmd);
	SendToStage(&Cmd, uidStage);
}

void AnnounceToAll(const char *pszMsg)
{
	MCmdWriter Cmd;
	BuildAnnounceCommand(pszMsg, &Cmd);
	SendToAll(&Cmd);
}