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

// SFMT header.
#include "SFMT/SFMT.h"

// TinyXML-2 header.
#include "tinyxml2/tinyxml2.h"
using namespace tinyxml2;

// wxWidgets header.
#include <wx/wx.h>
#include <wx/file.h>
#include <wx/datetime.h>

// Socket wrapper header.
#include "Socket.h"

// TinyThread++ header.
#include "tinythread++/tinythread.h"
using namespace tthread;

// GunZ Header.
#include "Main.h"

#include "MUID.h"
#include "MCommand.h"
#include "MCommandID.h"
#include "MCommandBlob.h"
#include "MPacketCrypter.h"
#include "MPacketSender.h"
#include "MServerFeature.h"

#include "MUtil.h"

#include "MServerMessage.h"

#endif