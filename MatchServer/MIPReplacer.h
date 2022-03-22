#ifndef __MIPREPLACER_H__
#define __MIPREPLACER_H__

struct MIPRepNode
{
	unsigned long nIPFrom;
	unsigned long nIPTo;
};

class MIPReplacer
{
public:
	MIPReplacer();
	~MIPReplacer();
	
	void Add(unsigned long nIPFrom, unsigned long nIPTo);
	void Clear();
	bool Load();
	
	unsigned long Replace(unsigned long nIP);
	
private:
	vector<MIPRepNode *> m_vtIPs;
};

extern MIPReplacer g_IPReplacer;

#endif