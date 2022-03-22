#ifndef __MCOMMANDID_H__
#define __MCOMMANDID_H__

#define MC_MATCH_NOTIFY						401
#define MC_MATCH_ANNOUNCE					402
#define MC_MATCH_RESPONSE_RESULT			403

#define MC_MATCH_LOGIN							1001
#define MC_MATCH_RESPONSE_LOGIN					1002
#define MC_MATCH_LOGIN_NETMARBLE				1003
#define MC_MATCH_LOGIN_NETMARBLE_JP				1004
#define MC_MATCH_LOGIN_FROM_DBAGENT				1005
#define MC_MATCH_LOGIN_FROM_DBAGENT_FAILED		1008
#define MC_MATCH_FIND_HACKING					1009
#define MC_MATCH_DISCONNMSG						1010
#define MC_MATCH_LOGIN_NHNUSA					1011
#define MC_MATCH_LOGIN_GAMEON_JP				1012
#define MC_MATCH_LOGIN_FHL_PERU					1013

#define MC_MATCH_BRIDGEPEER			1006
#define MC_MATCH_BRIDGEPEER_ACK		1007

#define MC_MATCH_OBJECT_CACHE		1101

#define MC_MATCH_REQUEST_ACCOUNT_CHARLIST	1701
#define MC_MATCH_RESPONSE_ACCOUNT_CHARLIST	1702
#define MC_MATCH_REQUEST_SELECT_CHAR		1703
#define MC_MATCH_RESPONSE_SELECT_CHAR		1704
#define MC_MATCH_REQUEST_CREATE_CHAR		1711
#define MC_MATCH_RESPONSE_CREATE_CHAR		1712
#define MC_MATCH_REQUEST_DELETE_CHAR		1713
#define MC_MATCH_RESPONSE_DELETE_CHAR		1714
#define MC_MATCH_REQUEST_ACCOUNT_CHARINFO	1719
#define MC_MATCH_RESPONSE_ACCOUNT_CHARINFO	1720
#define MC_MATCH_CHECK_GIFTITEM_LIST		1721
#define MC_MATCH_GIFTITEM_LIST				1722
#define MC_MATCH_READ_GIFTITEM_INFO			1723

#define MC_MATCH_REQUEST_SIMPLE_CHARINFO		1801
#define MC_MATCH_RESPONSE_SIMPLE_CHARINFO		1802
#define MC_MATCH_REQUEST_MY_SIMPLE_CHARINFO		1803
#define MC_MATCH_RESPONSE_MY_SIMPLE_CHARINFO	1804

#define MC_MATCH_REQUEST_BUY_ITEM						1811
#define MC_MATCH_RESPONSE_BUY_ITEM						1812
#define MC_MATCH_REQUEST_SELL_ITEM						1813
#define MC_MATCH_RESPONSE_SELL_ITEM						1814
#define MC_MATCH_REQUEST_SHOP_ITEMLIST					1815
#define MC_MATCH_RESPONSE_SHOP_ITEMLIST					1816
#define MC_MATCH_RESPONSE_BLITZSHOP_ITEMLIST			1817

#define MC_MATCH_REQUEST_CHARACTER_ITEMLIST			1821
#define MC_MATCH_RESPONSE_CHARACTER_ITEMLIST		1822
#define MC_MATCH_REQUEST_EQUIP_ITEM					1823
#define MC_MATCH_RESPONSE_EQUIP_ITEM				1824
#define MC_MATCH_REQUEST_TAKEOFF_ITEM				1825
#define MC_MATCH_RESPONSE_TAKEOFF_ITEM				1826
#define MC_MATCH_RESPONSE_GAMBLEITEM_LIST			1827
#define MC_MATCH_STAGE_UPDATE_CHARACTER_EQUIPITEM	1828

#define MC_MATCH_USER_WHISPER	1601
#define MC_MATCH_USER_WHERE		1602
#define MC_MATCH_USER_OPTION	1605

#define MC_MATCH_CHATROOM_CREATE				1651
#define MC_MATCH_CHATROOM_JOIN					1652
#define MC_MATCH_CHATROOM_LEAVE					1653
#define MC_MATCH_CHATROOM_INVITE				1661
#define MC_MATCH_CHATROOM_CHAT					1662
#define MC_MATCH_CHATROOM_SELECT_WRITE			1665

#define MC_MATCH_REQUEST_APPROPRIATE_CHANNEL		1201
#define MC_MATCH_RESPONSE_APPROPRIATE_CHANNEL		1202
#define MC_MATCH_CHANNEL_REQUEST_JOIN				1205
#define MC_MATCH_CHANNEL_REQUEST_INDIVIDUAL_JOIN	1206
#define MC_MATCH_CHANNEL_RESPONSE_JOIN				1207
#define MC_MATCH_CHANNEL_LIST_START					1211
#define MC_MATCH_CHANNEL_LIST_STOP					1212
#define MC_MATCH_CHANNEL_LIST						1213
#define MC_MATCH_CHANNEL_REQUEST_PLAYER_LIST		1221
#define MC_MATCH_CHANNEL_RESPONSE_PLAYER_LIST		1222
#define MC_MATCH_CHANNEL_REQUEST_CHAT				1225
#define MC_MATCH_CHANNEL_CHAT						1226
#define MC_MATCH_CHANNEL_RESPONSE_RULE				1231
#define MC_MATCH_CHANNEL_REQUEST_ALL_PLAYER_LIST	1232
#define MC_MATCH_CHANNEL_RESPONSE_ALL_PLAYER_LIST	1233

#define MC_MATCH_REQUEST_ACCOUNT_ITEMLIST			1831
#define MC_MATCH_RESPONSE_ACCOUNT_ITEMLIST			1832
#define MC_MATCH_REQUEST_BRING_ACCOUNTITEM			1833
#define MC_MATCH_RESPONSE_BRING_ACCOUNTITEM			1834
#define MC_MATCH_REQUEST_BRING_BACK_ACCOUNTITEM		1835
#define MC_MATCH_RESPONSE_BRING_BACK_ACCOUNTITEM	1836
#define MC_MATCH_EXPIRED_RENT_ITEM					1837

#define MC_MATCH_REQUEST_GAMBLE					1841
#define MC_MATCH_RESPONSE_GAMBLE				1842

#define MC_MATCH_REWARD_BATTLE_TIME				1843

#define MC_MATCH_REQUEST_USE_SPENDABLE_NORMAL_ITEM		1850
#define MC_MATCH_REQUEST_USE_SPENDABLE_BUFF_ITEM		1860
#define MC_MATCH_RESPONSE_USE_SPENDABLE_BUFF_ITEM		1861
#define MC_MATCH_SPENDABLE_BUFF_ITEM_STATUS				1865
#define MC_MATCH_REQUEST_USE_SPENDABLE_COMMUNITY_ITEM	1870

#define MC_MATCH_FRIEND_ADD						1901
#define MC_MATCH_FRIEND_REMOVE					1902
#define MC_MATCH_FRIEND_LIST					1903
#define MC_MATCH_RESPONSE_FRIENDLIST			1904
#define MC_MATCH_FRIEND_MSG						1905

#define MC_MATCH_CLAN_REQUEST_CREATE_CLAN			2000
#define MC_MATCH_CLAN_RESPONSE_CREATE_CLAN			2001
#define MC_MATCH_CLAN_ASK_SPONSOR_AGREEMENT			2002
#define MC_MATCH_CLAN_ANSWER_SPONSOR_AGREEMENT		2003
#define MC_MATCH_CLAN_REQUEST_AGREED_CREATE_CLAN	2004
#define MC_MATCH_CLAN_RESPONSE_AGREED_CREATE_CLAN	2005
#define MC_MATCH_CLAN_REQUEST_CLOSE_CLAN			2006
#define MC_MATCH_CLAN_RESPONSE_CLOSE_CLAN			2007
#define MC_MATCH_CLAN_REQUEST_JOIN_CLAN				2008
#define MC_MATCH_CLAN_RESPONSE_JOIN_CLAN			2009
#define MC_MATCH_CLAN_ASK_JOIN_AGREEMENT			2010
#define MC_MATCH_CLAN_ANSWER_JOIN_AGREEMENT			2011
#define MC_MATCH_CLAN_REQUEST_AGREED_JOIN_CLAN		2012
#define MC_MATCH_CLAN_RESPONSE_AGREED_JOIN_CLAN		2013
#define MC_MATCH_CLAN_REQUEST_LEAVE_CLAN			2014
#define MC_MATCH_CLAN_RESPONSE_LEAVE_CLAN			2015
#define MC_MATCH_CLAN_UPDATE_CHAR_CLANINFO			2016
#define MC_MATCH_CLAN_MASTER_REQUEST_CHANGE_GRADE	2017
#define MC_MATCH_CLAN_MASTER_RESPONSE_CHANGE_GRADE	2018
#define MC_MATCH_CLAN_ADMIN_REQUEST_EXPEL_MEMBER	2019
#define MC_MATCH_CLAN_ADMIN_RESPONSE_EXPEL_MEMBER	2020
#define MC_MATCH_CLAN_REQUEST_MSG					2021
#define MC_MATCH_CLAN_MSG							2022
#define MC_MATCH_CLAN_REQUEST_MEMBER_LIST			2023
#define MC_MATCH_CLAN_RESPONSE_MEMBER_LIST			2024
#define MC_MATCH_CLAN_REQUEST_CLAN_INFO				2025
#define MC_MATCH_CLAN_RESPONSE_CLAN_INFO			2026
#define MC_MATCH_CLAN_STANDBY_CLAN_LIST				2027
#define MC_MATCH_CLAN_MEMBER_CONNECTED				2028
#define MC_MATCH_CLAN_REQUEST_EMBLEMURL				2051
#define MC_MATCH_CLAN_RESPONSE_EMBLEMURL			2052
#define MC_MATCH_CLAN_LOCAL_EMBLEMREADY				2055
#define MC_MATCH_CLAN_ACCOUNCE_DELETE				2056

#define MC_MATCH_CALLVOTE							2100
#define MC_MATCH_NOTIFY_CALLVOTE					2101
#define MC_MATCH_NOTIFY_VOTERESULT					2102
#define MC_MATCH_VOTE_YES							2105
#define MC_MATCH_VOTE_NO							2106
#define MC_MATCH_VOTE_RESPONSE						2107
#define MC_MATCH_VOTE_STOP							2108

#define MC_MATCH_VOTE								2105	// 2012 or higher.

#define MC_MATCH_BROADCAST_CLAN_RENEW_VICTORIES			2200
#define MC_MATCH_BROADCAST_CLAN_INTERRUPT_VICTORIES		2201
#define MC_MATCH_BROADCAST_DUEL_RENEW_VICTORIES			2202
#define MC_MATCH_BROADCAST_DUEL_INTERRUPT_VICTORIES		2203

#define MC_MATCH_STAGE_CREATE					1301
#define MC_MATCH_RESPONSE_STAGE_CREATE			1302
#define MC_MATCH_STAGE_JOIN						1303
#define MC_MATCH_REQUEST_STAGE_JOIN				1304
#define MC_MATCH_REQUEST_PRIVATE_STAGE_JOIN		1305
#define MC_MATCH_RESPONSE_STAGE_JOIN			1306
#define MC_MATCH_STAGE_LEAVE					1307
#define MC_MATCH_STAGE_REQUEST_PLAYERLIST		1308
#define MC_MATCH_STAGE_FOLLOW					1309
#define MC_MATCH_RESPONSE_STAGE_FOLLOW			1310
#define MC_MATCH_REQUEST_STAGE_LIST				1311
#define MC_MATCH_STAGE_LIST						1314
#define MC_MATCH_STAGE_CHAT						1321
#define MC_MATCH_STAGE_REQUEST_QUICKJOIN		1322
#define MC_MATCH_STAGE_RESPONSE_QUICKJOIN		1323
#define MC_MATCH_STAGE_GO						1331
#define MC_MATCH_STAGE_REQUIRE_PASSWORD			1332
#define MC_MATCH_STAGE_KICK						1333

#define MC_MATCH_STAGE_REQUEST_ENTERBATTLE		1401
#define MC_MATCH_STAGE_ENTERBATTLE				1402
#define MC_MATCH_STAGE_LEAVEBATTLE_TO_SERVER	1403
#define MC_MATCH_STAGE_LEAVEBATTLE_TO_CLIENT	1404

#define MC_MATCH_REQUEST_STAGESETTING			1411
#define MC_MATCH_RESPONSE_STAGESETTING			1412
#define MC_MATCH_STAGESETTING					1413
#define MC_MATCH_STAGE_MAP						1414
#define MC_MATCH_STAGE_REQUEST_FORCED_ENTRY		1415
#define MC_MATCH_STAGE_RESPONSE_FORCED_ENTRY	1416
#define MC_MATCH_STAGE_RELAY_MAP_ELEMENT_UPDATE	1417
#define MC_MATCH_STAGE_RELAY_MAP_INFO_UPDATE	1418

#define MC_MATCH_STAGE_MASTER					1421
#define MC_MATCH_STAGE_PLAYER_STATE				1422
#define MC_MATCH_STAGE_TEAM						1423
#define MC_MATCH_STAGE_START					1431
#define MC_MATCH_STAGE_LAUNCH					1432
#define MC_MATCH_STAGE_RELAY_LAUNCH				1433

#define MC_MATCH_LOADING_COMPLETE				1441
#define MC_MATCH_STAGE_FINISH_GAME				1442

#define MC_MATCH_REQUEST_GAME_INFO				1451
#define MC_MATCH_RESPONSE_GAME_INFO				1452

#define MC_MATCH_REQUEST_PEERLIST				1461
#define MC_MATCH_RESPONSE_PEERLIST				1462

#define MC_MATCH_REQUEST_PEER_RELAY				1471
#define MC_MATCH_RESPONSE_PEER_RELAY			1472

#define MC_MATCH_GAME_ROUNDSTATE				1501
#define MC_MATCH_ROUND_FINISHINFO				1502
#define MC_MATCH_GAME_KILL						1511
#define MC_MATCH_GAME_DEAD						1512
#define MC_MATCH_GAME_LEVEL_UP					1513
#define MC_MATCH_GAME_LEVEL_DOWN				1514
#define MC_MATCH_GAME_REQUEST_SPAWN				1515
#define MC_MATCH_GAME_RESPONSE_SPAWN			1516
#define MC_MATCH_GAME_TEAMBONUS					1517

#define MC_MATCH_GAME_REQUEST_TIMESYNC			1521
#define MC_MATCH_GAME_RESPONSE_TIMESYNC			1522
#define MC_MATCH_GAME_REPORT_TIMESYNC			1523

#define MC_MATCH_REQUEST_SUICIDE				1531
#define MC_MATCH_RESPONSE_SUICIDE				1532
#define MC_MATCH_RESPONSE_SUICIDE_RESERVE		1533
#define MC_MATCH_NOTIFY_THROW_TRAPITEM			1535
#define MC_MATCH_NOTIFY_ACTIVATED_TRAPITEM		1536
#define MC_MATCH_NOTIFY_ACTIATED_TRAPITEM_LIST	1437
#define MC_MATCH_REQUEST_OBTAIN_WORLDITEM		1541
#define MC_MATCH_OBTAIN_WORLDITEM				1542
#define MC_MATCH_SPAWN_WORLDITEM				1543
#define MC_MATCH_REQUEST_SPAWN_WORLDITEM		1544
#define MC_MATCH_REMOVE_WORLDITEM				1545
#define MC_MATCH_ASSIGN_COMMANDER				1551
#define MC_MATCH_RESET_TEAM_MEMBERS				1552
#define MC_MATCH_SET_OBSERVER					1553

#define MC_MATCH_REQUEST_PROPOSAL				1561
#define MC_MATCH_RESPONSE_PROPOSAL				1562
#define MC_MATCH_ASK_AGREEMENT					1563
#define MC_MATCH_REPLY_AGREEMENT				1564

#define MC_MATCH_DUELTOURNAMENT_REQUEST_JOINGAME				2300
#define MC_MATCH_DUELTOURNAMENT_RESPONSE_JOINGAME				2301
#define MC_MATCH_DUELTOURNAMENT_REQUEST_CANCELGAME				2302
#define MC_MATCH_DUELTOURNAMENT_CHAR_INFO						2303
#define MC_MATCH_DUELTOURNAMENT_PREPARE_MATCH					2304
#define MC_MATCH_DUELTOURNAMENT_LAUNCH_MATCH					2305
#define MC_MATCH_DUELTOURNAMENT_CANCEL_MATCH					2306
#define MC_MATCH_DUELTOURNAMENT_CHAR_INFO_PREVIOUS				2307
#define MC_MATCH_DUELTOURNAMENT_NOT_SERVICE_TIME				2308
#define MC_MATCH_DUELTOURNAMENT_REQUEST_SIDERANKING_INFO		2400
#define MC_MATCH_DUELTOURNAMENT_RESPONSE_SIDERANKING_INFO		2401
#define MC_MATCH_DUELTOURNAMENT_GROUPRANKING_INFO				2402
#define MC_MATCH_DUELTOURNAMENT_GAME_INFO						2500
#define MC_MATCH_DUELTOURNAMENT_GAME_ROUND_RESULT_INFO			2501
#define MC_MATCH_DUELTOURNAMENT_GAME_MATCH_RESULT_INFO			2502
#define MC_MATCH_DUELTOURNAMENT_GAME_PLAYER_STATUS				2503
#define MC_MATCH_DUELTOURNAMENT_GAME_NEXT_MATCH_PLYAERINFO		2504

#define MC_MATCH_ASSIGN_BERSERKER					3001
#define MC_MATCH_DUEL_QUEUEINFO						3100

#define MC_MATCH_REQUEST_CHAR_QUEST_ITEMLIST		21000
#define MC_MATCH_RESPONSE_CHAR_QUEST_ITEMLIST		21001
#define MC_MATCH_REQUEST_BUY_QUEST_ITEM				21002
#define MC_MATCH_RESPONSE_BUY_QUEST_ITEM			21003
#define MC_MATCH_REQUEST_SELL_QUEST_ITEM			21004
#define MC_MATCH_RESPONSE_SELL_QUEST_ITEM			21005
#define MC_MATCH_USER_QUEST_REWARD					21006
#define MC_MATCH_REQUEST_DROP_SACRIFICE_ITEM		21007
#define MC_MATCH_RESPONSE_DROP_SACRIFICE_ITEM		21008
#define MC_MATCH_REQUEST_CALLBACK_SACRIFICE_ITEM	21009
#define MC_MATCH_RESPONSE_CALLBACK_SACRIFICE_ITEM	21010
#define MC_MATCH_REQUEST_SLOT_INFO					21011
#define MC_MATCH_RESPONSE_SLOT_INFO					21012

#define MC_QUEST_NPC_SPAWN					6000
#define MC_QUEST_ENTRUST_NPC_CONTROL		6001
#define MC_QUEST_CHECKSUM_NPCINFO			6002
#define MC_QUEST_REQUEST_NPC_DEAD			6003
#define MC_QUEST_NPC_DEAD					6004
#define MC_QUEST_REFRESH_PLAYER_STATUS		6005
#define MC_QUEST_NPC_ALL_CLEAR				6006
#define MC_MATCH_QUEST_REQUEST_DEAD			6007
#define MC_MATCH_QUEST_PLAYER_DEAD			6008
#define MC_QUEST_OBTAIN_QUESTITEM			6009
#define MC_QUEST_STAGE_MAPSET				6010
#define MC_QUEST_OBTAIN_ZITEM				6011
#define MC_QUEST_PING						6012
#define MC_QUEST_PONG						6013
#define MC_QUEST_NPCLIST					6014

#define MC_QUEST_GAME_INFO					6051
#define MC_QUEST_COMBAT_STATE				6052
#define MC_QUEST_SECTOR_START				6053
#define MC_QUEST_COMPLETED					6054
#define MC_QUEST_FAILED						6055

#define MC_QUEST_REQUEST_MOVETO_PORTAL		6057
#define MC_QUEST_MOVETO_PORTAL				6058
#define MC_QUEST_READYTO_NEWSECTOR			6059

#define MC_GAME_START_FAIL					6060
#define MC_QUEST_STAGE_GAME_INFO			6061
#define MC_QUEST_SECTOR_BONUS				6062

#define MC_QUEST_ROUND_START					6100
#define MC_QUEST_REQUEST_LEVEL					6101
#define MC_QUEST_RESPONSE_LEVEL					6102
#define MC_QUEST_SURVIVAL_RESULT				6103
#define MC_SURVIVAL_RANKING_LIST				6104
#define MC_SURVIVAL_PRIVATE_RANKING				6105

#define MC_REQUEST_SERVER_LIST_INFO				40001
#define MC_RESPONSE_SERVER_LIST_INFO			40002


#define MC_MATCH_REGISTERAGENT				5001
#define MC_MATCH_UNREGISTERAGENT			5002
#define MC_MATCH_AGENT_REQUEST_LIVECHECK	5011
#define MC_MATCH_AGENT_RESPONSE_LIVECHECK	5012
#define MC_AGENT_ERROR						5013

#define MC_AGENT_STAGE_RESERVE				5051
#define MC_AGENT_STAGE_RELEASE				5052
#define MC_AGENT_STAGE_READY				5053
#define MC_AGENT_LOCATETO_CLIENT			5061
#define MC_AGENT_RELAY_PEER					5062
#define MC_AGENT_PEER_READY					5063
#define MC_AGENT_PEER_BINDTCP				5071
#define MC_AGENT_PEER_BINDUDP				5072
#define MC_AGENT_PEER_UNBIND				5073

#define MC_AGENT_TUNNELING_TCP				5081
#define MC_AGENT_TUNNELING_UDP				5082
#define MC_AGENT_ALLOW_TUNNELING_TCP		5083
#define MC_AGENT_ALLOW_TUNNELING_UDP		5084
#define MC_AGENT_DEBUGPING					5101
#define MC_AGENT_DEBUGTEST					5102


#define MC_ADMIN_ANNOUNCE					501
#define MC_ADMIN_REQUEST_SERVER_INFO		505
#define MC_ADMIN_RESPONSE_SERVER_INFO		506
#define MC_ADMIN_SERVER_HALT				511
#define MC_ADMIN_TERMINAL					512
#define MC_ADMIN_REQUEST_UPDATE_ACCOUNT_UGRADE	513
#define MC_ADMIN_RESPONSE_UPDATE_ACCOUNT_UGRADE	514
#define MC_ADMIN_REQUEST_KICK_PLAYER			515
#define MC_ADMIN_RESPONSE_KICK_PLAYER			516
#define MC_ADMIN_REQUEST_MUTE_PLAYER			517
#define MC_ADMIN_RESPONSE_MUTE_PLAYER			518
#define MC_ADMIN_REQUEST_BLOCK_PLAYER			519
#define MC_ADMIN_RESPONSE_BLOCK_PLAYER			520
#define MC_ADMIN_PING_TO_ALL				521
#define MC_ADMIN_REQUEST_SWITCH_LADDER_GAME	522
#define MC_ADMIN_HIDE						531
#define MC_ADMIN_RELOAD_CLIENT_HASH			532
#define MC_ADMIN_RESET_ALL_HACKING_BLOCK	533
#define MC_ADMIN_RELOAD_GAMBLEITEM			534
#define MC_ADMIN_DUMP_GAMBLEITEM_LOG		535
#define MC_ADMIN_ASSASSIN					536


// challenge quest.
#define MC_CHALLENGE_QUEST_NPC_SPAWN					6301
#define MC_CHALLENGE_QUEST_REQUEST_NPC_DEAD				6302
#define MC_CHALLENGE_QUEST_NPC_DEAD						6303
#define MC_CHALLENGE_QUEST_ENTRUST_NPC_CONTROL			6304
#define MC_CHALLENGE_QUEST_ADVANCE_SECTOR				6341
#define MC_CHALLENGE_QUEST_PLAYER_REQUEST_DEAD			6342
#define MC_CHALLENGE_QUEST_PLAYER_DEAD					6343
#define MC_CHALLENGE_QUEST_FAILED						6344
#define MC_CHALLENGE_QUEST_COMPLETED					6345
#define MC_CHALLENGE_QUEST_SECTOR_SUCCESS				6346
#define MC_CHALLENGE_QUEST_PLAYER_REQUEST_NPC_SPAWN		6407
#define MC_CHALLENGE_QUEST_PLAYER_NPC_SPAWN				6408
#define MC_CHALLENGE_QUEST_REQUEST_TOP_RECORD			6347
#define MC_CHALLENGE_QUEST_RESPONSE_TOP_RECORD			6348
#define MC_CHALLENGE_QUEST_RESULT_TOTAL_TIME			6350


// blitzkrieg.
#define MC_BLITZ_REQUEST_MATCH_COLLABORATOR_LIST		6500
#define MC_BLITZ_RESPONSE_MATCH_COLLABORATOR_LIST		6501
#define MC_BLITZ_REQUEST_RESERVE_CHALLENGE				6502
#define MC_BLITZ_WAITING_CHALLENGER_LIST				6503
#define MC_BLITZ_REQUEST_CANCEL_CHALLENGE				6505
#define MC_BLITZ_CANCEL_CHALLENGE						6506
#define MC_BLITZ_GAME_LAUNCH							6507
#define MC_BLITZ_UPDATE_SCORE							6508
#define MC_BLITZ_CHALLENGE_PROHIBITED					6509
#define MC_BLITZ_NPC_SPAWN								6515
#define MC_BLITZ_PLAYER_NPC_SPAWN						6516
#define MC_BLITZ_ROUND_STATE							6517
#define MC_BLITZ_CHOOSE_CLASS							6518
#define MC_BLITZ_AVAILABLE_CLASS						6519
#define MC_BLITZ_REQUEST_NPC_DEAD						6520
#define MC_BLITZ_NPC_DEAD								6521
#define MC_BLITZ_PLAYER_REQUEST_DEAD					6524
#define MC_BLITZ_PLAYER_DEAD							6525
#define MC_BLITZ_PLAYER_REQUEST_SKILL_UPGRADE			6526
#define MC_BLITZ_PLAYER_SKILL_UPGRADE					6527
#define MC_BLITZ_MATCH_FINISH							6530
#define MC_BLITZ_REINFORCE_STATE						6531


// spy mode.
#define MC_SPY_STAGE_BAN_MAP				6450
#define MC_SPY_STAGE_BANNED_MAP_LIST		6451
#define MC_SPY_STAGE_REQUEST_START			6452
#define MC_SPY_GAME_INFO					6453
#define MC_SPY_GAME_RESULT					6454
#define MC_SPY_GAME_SCORE					6455



// cash shop.
#define MC_CASHSHOP_REQUEST_ITEMLIST			1880
#define MC_CASHSHOP_RESPONSE_ITEMLIST			1881
#define MC_CASHSHOP_REQUEST_ITEMINFO			1882
#define MC_CASHSHOP_RESPONSE_ITEMINFO			1883
#define MC_CASHSHOP_REQUEST_CASH_REMAINDER		1885
#define MC_CASHSHOP_RESPONSE_CASH_REMAINDER		1886


#endif