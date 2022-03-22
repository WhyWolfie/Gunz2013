#ifndef __MMATCHCONSTANT_H__
#define __MMATCHCONSTANT_H__

#include "MUID.h"

// Login.
#define MAX_USERID_LEN			128
#define MAX_USERPASSWORD_LEN	128

// Account.
#define ACCOUNT_CHARLIST_COUNT	4

// Character.
#define MIN_CHARNAME_LEN		4
#define MAX_CHARNAME_LEN		12
#define CHARNAME_LEN			32
#define DB_CHARNAME_LEN			24

// character creation.
#define CHAR_COSTUME_COUNT		6
#define CHAR_HAIR_COUNT			4
#define CHAR_FACE_COUNT			4

// clan.
#define CLANNAME_LEN			16
#define DB_CLANNAME_LEN			24
#define MIN_CLANNAME_LEN		4
#define MAX_CLANNAME_LEN		12

#define CLAN_CREATION_REQUIRED_LEVEL	10
#ifdef _BOUNTY
	#define CLAN_CREATION_REQUIRED_BOUNTY	2000
#else
	#define CLAN_CREATION_REQUIRED_BOUNTY	0
#endif

// quest.
#define MAX_QUEST_ITEM_COUNT		99

#define MAX_QUEST_NPC_INFO_COUNT	14
#define MAX_QUEST_MAP_SECTOR_COUNT	16

// proposal - MMatchProposalMode.
#define MPROPOSAL_NONE				0
#define MPROPOSAL_LADDER_INVITE		1
#define MPROPOSAL_CLAN_INVITE		2
#define MPROPOSAL_BLITZ_INVITE		3
#define MPROPOSAL_END				4

// proposal max replier count.
#define MAX_PROPOSAL_REPLIER_COUNT	3

// Encryption, Decryption key.
#define ENCRYPTIONKEY_LENGTH		32

// Server UID.
#define SERVERUID_LOWID		2
#define SERVERUID_HIGHID	0
#define SERVERUID			MUID(SERVERUID_HIGHID, SERVERUID_LOWID)

#endif