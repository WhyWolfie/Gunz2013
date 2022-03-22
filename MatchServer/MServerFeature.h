#ifndef __MSERVERFEATURE_H__
#define __MSERVERFEATURE_H__

// display all of received command id to the console.
#ifdef _DEBUG
	#define _SHOW_RECVCMD_ID
#endif

// if this is undefined, the bounty point system will be removed.
#define _BOUNTY

// if this is undefined, you'll be allowed to equip all items without levels.
#define _ITEM_LEVEL_LIMITATION

// 2012 features.
#if _GAME_VERSION >= 2012	
	// set initial equipment to peru version.
	#define _PERU_INIT_EQUIP
#endif

// when login, if the account not exists, create a new account.
#define _AUTO_REGISTER

#endif	// __MSERVERFEATURE_H__