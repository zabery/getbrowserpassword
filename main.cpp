#include "browserinfo.h"

int main()
{
	CBrowserInfo *pBrowser = new CBrowserInfo();
	pBrowser->ReadChromeLoginData();
	delete pBrowser;
	system("pause");
	return 0;
}