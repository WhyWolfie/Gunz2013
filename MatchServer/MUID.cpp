#include "pch.h"
#include "MUID.h"

MUID::MUID(unsigned long h, unsigned long l)
{
	ulHighID = h;
	ulLowID = l;
}

void MUID::SetZero()
{
	ulHighID = 0;
	ulLowID = 0;
}

bool MUID::IsValid() const
{
	if(ulHighID != 0 || ulLowID != 0)
		return true;
		
	return false;
}

bool MUID::IsInvalid() const
{
	if(ulHighID == 0 && ulLowID == 0)
		return true;
		
	return false;
}

void MUID::Increment()
{
	if(ulLowID < ULONG_MAX)
	{
		ulLowID++;
	}
	else
	{
		if(ulHighID < ULONG_MAX)
		{
			ulLowID = 0;
			ulHighID++;
		}
		else
		{
			ulLowID = 1;
			ulHighID = 0;
		}
	}
}

bool MUID::operator==(const MUID &uid) const
{
	if(ulHighID == uid.ulHighID && ulLowID == uid.ulLowID)
		return true;
		
	return false;
}

bool MUID::operator!=(const MUID &uid) const
{
	if(ulHighID != uid.ulHighID || ulLowID != uid.ulLowID)
		return true;
		
	return false;
}

bool MUID::operator<(const MUID &uid) const
{
	if(ulHighID < uid.ulHighID) return true;
	
	if(ulHighID == uid.ulHighID)
		if(ulLowID < uid.ulLowID) return true;
		
	return false;
}

bool MUID::operator<=(const MUID &uid) const
{
	if(ulHighID < uid.ulHighID) return true;
	
	if(ulHighID == uid.ulHighID)
		if(ulLowID <= uid.ulLowID) return true;
		
	return false;
}

bool MUID::operator>(const MUID &uid) const
{
	if(ulHighID > uid.ulHighID) return true;
	
	if(ulHighID == uid.ulHighID)
		if(ulLowID > uid.ulLowID) return true;
		
	return false;
}

bool MUID::operator>=(const MUID &uid) const
{
	if(ulHighID > uid.ulHighID) return true;
	
	if(ulHighID == uid.ulHighID)
		if(ulLowID >= uid.ulLowID) return true;
		
	return false;
}

MUID &MUID::operator=(const MUID &uid)
{
	ulHighID = uid.ulHighID;
	ulLowID = uid.ulLowID;
	
	return *this;
}

const MUID &MUID::Assign()
{
	static MUID uid = MUID(0, 10);
	uid.Increment();
	
	return uid;
}
