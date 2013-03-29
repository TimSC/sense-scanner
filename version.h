#ifndef VERSION_H
#define VERSION_H

#include <string>
#include <vector>
#include <map>
#include <QtCore/QSettings>

#define VERSION_STR "Dev (Post-V1.0 Beta 3)"
#define VERSION_URL "post-v1beta3"

//#define RELEASE_MODE

class Registration
{
public:
    Registration();
    virtual ~Registration();

    int ReadLicense();
    std::map<std::string, std::string> GetInfo();

    QSettings *settings;

protected:
    std::string masterPubKey;
    std::vector<std::vector<std::string> > verifiedInfo;
};

#endif // VERSION_H
