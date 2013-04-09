#ifndef VERSION_H
#define VERSION_H

#include <string>
#include <vector>
#include <map>
#include <QtCore/QSettings>

#define VERSION_STR "v1.0"
#define VERSION_URL "v1.0"

#define RELEASE_MODE

class Registration
{
public:
    Registration();
    virtual ~Registration();

    int ReadLicense();
    int ReadLicense(QString fiStr);
    int SetLicenseFromFile(QString filename);

    std::map<std::string, std::string> GetInfo();

    QSettings *settings;

protected:
    std::string masterPubKey;
    std::vector<std::vector<std::string> > verifiedInfo;
};

#endif // VERSION_H
