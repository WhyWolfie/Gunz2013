#include "pch.h"

#include "MMatchExp.h"

MMatchFormula g_FormulaMgr;

bool MMatchFormula::ParseFormulaTable(XMLDocument *doc, const char *pszId, float *pOut)
{
	XMLHandle handle(doc);
	handle = handle.FirstChildElement("XML");
	
	XMLElement *element = handle.FirstChildElement("FORMULA_TABLE").ToElement();
	
	while(element != NULL)
	{
		if(element->Attribute("id") == NULL)
		{
			element = element->NextSiblingElement();
			continue;
		}
		
		if(MStricmp(element->Attribute("id"), pszId) != 0)
		{
			element = element->NextSiblingElement();
			continue;
		}
		
		XMLElement *subelement = element->FirstChildElement("LM");
		
		while(subelement != NULL)
		{
			int nLower = subelement->IntAttribute("lower");
			int nUpper = subelement->IntAttribute("upper");
		
			float fLM;
			
			if(subelement->QueryFloatText(&fLM) != XML_SUCCESS)
				return false;
				
			for(int i = nLower; i <= nUpper; i++)
			{
				pOut[i] = fLM;
			}
				
			subelement = subelement->NextSiblingElement();
		}
		
		return true;
	}
	
	return false;
}

bool MMatchFormula::Load()
{
	XMLDocument doc;
	doc.LoadFile("formula.xml");
	
	if(doc.Error() == true) 
	{
		mlog("Can't load EXP table.");
		return false;
	}
	
	mlog("Loading formula EXP calculator...");
	
	float fNeedExpLM[MAX_CHARLEVEL + 1];
	float fGettingExpLM[MAX_CHARLEVEL + 1];
	float fGettingBountyLM[MAX_CHARLEVEL + 1];
	
	if(ParseFormulaTable(&doc, "NeedExpLM", fNeedExpLM) == false) return false;
	if(ParseFormulaTable(&doc, "GettingExpLM", fGettingExpLM) == false) return false;
	if(ParseFormulaTable(&doc, "GettingBountyLM", fGettingBountyLM) == false) return false;
	
	// calc need exp.
	for(int lv = 1; lv <= MAX_CHARLEVEL; lv++)
	{
		unsigned long n = (unsigned long)(((float)lv * (float)lv * fNeedExpLM[lv] * 100.0f) + 0.5f) * 2;
		m_nNeedExp[lv] = m_nNeedExp[lv - 1] + n;
	}
	
	// calc getting exp.
	for(int lv = 1; lv <= MAX_CHARLEVEL; lv++)
	{
		m_nGettingExp[lv] = (unsigned long)(((float)lv * fGettingExpLM[lv] * 20.0f) + 0.5f) + 
						(unsigned long)(((float)(lv - 1) * fGettingExpLM[lv] * 10) + 0.5f);
	}
	
	// calc getting bp.
	for(int lv = 1; lv <= MAX_CHARLEVEL; lv++)
	{
		#ifdef _BOUNTY
		m_nGettingBounty[lv] = (unsigned long)(((float)lv * fGettingBountyLM[lv] * 1.2f) + 0.5f);
		#else
		m_nGettingBounty[lv] = 0;
		#endif
	}
	
	mlog("Formula EXP calculator loaded.");
	
	return true;
}

int MMatchFormula::GetExpPercent(int nLevel, unsigned long nExp)
{
	if(nLevel < 1 || nLevel > MAX_CHARLEVEL) return 0;
	
	unsigned long nCurrLevelExp = m_nNeedExp[nLevel - 1];
	unsigned long nNextLevelExp = m_nNeedExp[nLevel];
	
	int nPercent = (int)(((float)(nExp - nCurrLevelExp) / (float)(nNextLevelExp - nCurrLevelExp)) * 100.0f);
	
	if(nPercent < 0) nPercent = 0;
	else if(nPercent > 100) nPercent = 100;
	
	return nPercent;
}

int MMatchFormula::GetLevelFromExp(unsigned long nExp)
{
	for(int lv = 1; lv < MAX_CHARLEVEL; lv++)
	{
		if(nExp < m_nNeedExp[lv]) return lv;
	}
	return MAX_CHARLEVEL;
}

bool MMatchFormula::GetKillExp(int nLevel, unsigned long *pOutXP, unsigned long *pOutBP)
{
	*pOutXP = *pOutBP = 0;
	
	if(nLevel < 1 || nLevel > MAX_CHARLEVEL) return false;
	
	*pOutXP = m_nGettingExp[nLevel];
	*pOutBP = m_nGettingBounty[nLevel];
	
	return true;
}
