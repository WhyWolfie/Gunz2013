#ifndef __MMATCHEXP_H__
#define __MMATCHEXP_H__

#define MAX_CHARLEVEL	99

class MMatchFormula
{
private:
	bool ParseFormulaTable(XMLDocument *doc, const char *pszId, float *pOut);
	
public:
	bool Load();

	int GetExpPercent(int nLevel, unsigned long nExp);
	int GetLevelFromExp(unsigned long nExp);
	bool GetKillExp(int nLevel, unsigned long *pOutXP, unsigned long *pOutBP);
	
private:
	unsigned long m_nNeedExp[MAX_CHARLEVEL + 1];
	unsigned long m_nGettingExp[MAX_CHARLEVEL + 1];
	unsigned long m_nGettingBounty[MAX_CHARLEVEL + 1];
};

extern MMatchFormula g_FormulaMgr;

#endif