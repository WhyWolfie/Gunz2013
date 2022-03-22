#ifndef __MMATCHOBJECT_CONSTANT_H__
#define __MMATCHOBJECT_CONSTANT_H__

// character sex.
#define MMS_MALE	0
#define MMS_FEMALE	1
#define MMS_END		2

// character item slots.
#define MMCIP_HEAD			0
#define MMCIP_CHEST			1
#define MMCIP_HANDS			2
#define MMCIP_LEGS			3
#define MMCIP_FEET			4
#define MMCIP_FINGERL		5
#define MMCIP_FINGERR		6
#define MMCIP_MELEE			7
#define MMCIP_PRIMARY		8
#define MMCIP_SECONDARY		9
#define MMCIP_CUSTOM1		10
#define MMCIP_CUSTOM2		11
#define MMCIP_AVATAR		12
#define MMCIP_COMMUNITY1	13
#define MMCIP_COMMUNITY2	14
#define MMCIP_LONGBUFF1		15
#define MMCIP_LONGBUFF2		16
#define MMCIP_END			17

// account ugradeid.
#define MMUG_FREE			0
#define MMUG_REGULAR		1
#define MMUG_STAR			2
#define MMUG_CRIMINAL		100
#define MMUG_WARNING_1		101
#define MMUG_WARNING_2		102
#define MMUG_WARNING_3		103
#define MMUG_CHAT_LIMITED	104
#define MMUG_PENALTY		105
#define MMUG_EVENTMASTER	252
#define MMUG_BLOCKED		253
#define MMUG_DEVELOPER		254
#define MMUG_ADMIN			255

// account pgradeid.
#define MMPG_FREE		0
#define MMPG_PREMIUM_IP	1

// place.
#define MMP_OUTSIDE	0
#define MMP_LOBBY	1
#define MMP_STAGE	2
#define MMP_BATTLE	3

// object stage state.
#define MOSS_WAIT		0
#define MOSS_READY		1
#define MOSS_SHOP		2
#define MOSS_EQUIPMENT	3
#define MOSS_END		4

// enter battle param.
#define MCEP_NORMAL	0
#define MCEP_FORCED	1

// clan grade.
#define MCG_NONE	0
#define MCG_MASTER	1
#define MCG_ADMIN	2
#define MCG_MEMBER	9

// game team.
#define MMT_ALL			0
#define MMT_SPECTATOR	1
#define MMT_RED			2
#define MMT_BLUE		3
#define MMT_END			4

// option flag.
#define MBITFLAG_USEROPTION_REJECT_WHISPER	1
#define MBITFLAG_USEROPTION_REJECT_INVITE	2

#endif