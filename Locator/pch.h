#ifndef __PCH_H__
#define __PCH_H__

// Standard C Headers.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

// Standard C++ Header.
#include <vector>
#include <list>
#include <deque>
#include <map>
#include <algorithm>
using namespace std;

// TinyXML-2 header.
#include "../MatchServer/tinyxml2/tinyxml2.h"
using namespace tinyxml2;

// wxWidgets header.
#include <wx/wx.h>

// Socket wrapper header.
#include "../MatchServer/Socket.h"

// TinyThread++ header.
#include "../MatchServer/tinythread++/tinythread.h"
using namespace tthread;

// GunZ Header.
#include "../MatchServer/MUID.h"
#include "../MatchServer/MCommand.h"
#include "../MatchServer/MCommandID.h"
// #include "../MatchServer/MCommandBlob.h"
#include "MLocatorCommandBlob.h"
#include "../MatchServer/MPacketCrypter.h"

#include "../MatchServer/MUtil.h"

#endif