#ifndef __MUID_H__
#define __MUID_H__

class MUID
{
public:
	unsigned long ulHighID;
	unsigned long ulLowID;

public:
	MUID(unsigned long h = 0, unsigned long l = 0);

	void SetZero();

	bool IsValid() const;
	bool IsInvalid() const;

	void Increment();

	bool operator==(const MUID &uid) const;
	bool operator!=(const MUID &uid) const;
	
	bool operator<(const MUID &uid) const;
	bool operator<=(const MUID &uid) const;
	bool operator>(const MUID &uid) const;
	bool operator>=(const MUID &uid) const;
	
	MUID &operator=(const MUID &uid);
	
public:
	static const MUID &Assign();
};

#endif	// __MUID_H__
