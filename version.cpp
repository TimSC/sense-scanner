#include "version.h"

#include <QtCore/QFile>
#include <QtXml/QtXml>
#include <iostream>
#include <vector>
#include <string>
using namespace std;
#include <crypto++/rsa.h>
#include <crypto++/osrng.h>
#include <crypto++/base64.h>
#include <crypto++/files.h>
using namespace CryptoPP;

void myReplace(std::string& str, const std::string& oldStr, const std::string& newStr)
{
    //From http://stackoverflow.com/a/1494435
    size_t pos = 0;
    while((pos = str.find(oldStr, pos)) != std::string::npos)
    {
        str.replace(pos, oldStr.length(), newStr);
        pos += newStr.length();
    }
}

string SerialiseKeyPairs(vector<vector<std::string> > &info)
{
    string out;
    for(unsigned int pairNum = 0;pairNum < info.size();pairNum++)
    {
        out.append("<data k=\"");
        myReplace(info[pairNum][0], "\"", "&quot;");
        out.append(info[pairNum][0]);
        out.append("\" v=\"");
        myReplace(info[pairNum][1], "\"", "&quot;");
        out.append(info[pairNum][1]);
        out.append("\" />");
    }

    return out;
}

int VerifyLicense(string signedTxt, string sigIn, string pubKeyEnc)
{
    //Read public key
    CryptoPP::ByteQueue bytes;
    StringSource file(pubKeyEnc, true, new Base64Decoder);
    file.TransferTo(bytes);
    bytes.MessageEnd();
    RSA::PublicKey pubKey;
    pubKey.Load(bytes);

    RSASSA_PKCS1v15_SHA_Verifier verifier(pubKey);

    //Read signed message
    CryptoPP::ByteQueue sig;
    StringSource sigFile(sigIn, true, new Base64Decoder);
    string sigStr;
    StringSink sigStrSink(sigStr);
    sigFile.TransferTo(sigStrSink);

    string combined(signedTxt);
    combined.append(sigStr);

    //Verify signature
    try
    {
        StringSource(combined, true,
            new SignatureVerificationFilter(
                verifier, NULL,
                SignatureVerificationFilter::THROW_EXCEPTION
           )
        );
    }
    catch(SignatureVerificationFilter::SignatureVerificationFailed &err)
    {
        cout << err.what() << endl;
        return 0;
    }
    return 1;
}

//******************************************************

Registration::Registration()
{
    this->masterPubKey = "MIIEIDANBgkqhkiG9w0BAQEFAAOCBA0AMIIECAKCBAEA1+aISbx0wxYfvSsbCW0kZwEOWbot"
            "V9MNkm10h+agygBMGcwDUCH/iURgf5RiM9/crgmUQ9ol/uJXnDUCgyXcZvO1sz3+Y3NaYkvy"
            "b+IWGAmYAq/VU2Dui3lSo8whm2h1vAtnwrOgECt2pPs1jg5qKu1gzl6AtplviLJdRg07FFNU"
            "pFjCjlCgVL12Oj7OWiJtxWa6qggULTGNh2F28Ge9PvPFtg/W5vzDW9NoKhIyE3JmIFx2p0qK"
            "Wke3z3YJziUtd16el7tO2QgIgVJ7IHJGJCr6wRBD8ouHqkdguxYNP7Y+JoL9HIXJ/s/bgTVJ"
            "/6hj342AAQmEhYfFxA9lJ+zKC/ohe4iekQ0cNrQSs79KSO73lYoY1qvEilmaBWzNBp9iq8/S"
            "PHJjTtzM0GTWUdrty848AJtQRtvq+4eHHqYWufTlSthB0dQrtmvEYcr8URNVkZP7DV9ZWsUo"
            "5lR79t/rK3kvNoSP0xr2sYhAnn3OkfHN/ei+0PEutyX/9y8s6Sl+O1ujm0shJJP/fd46o9Tx"
            "jKkPvv7CBhHQVvHV5IJneNqpKv4ojrbXvp4XH0a6NXef9j/c66qLP1pbngtmbgn40BDVsD31"
            "Oqm2T4sVl8cyoJOrd5MpxdvUv484+cjkCmaligKdbjp5C1FL/fjTr0OCsedexsZz3iZbfWHg"
            "G9uSuoyICs6pQtt6FoDlxVQWyTHINcXFleWp77jCt0G4jtUw1UYLQIaAEgTL375cG67CQbuv"
            "GB+soF3LAGS7U+N+mBYj3p6+nNpIqoi1W2r29lsveoOjLBbyaSz4rx22mlup7ZzPrrb7czBD"
            "jvBaDMQxRarJkpNT+6X5idsNzg8jNShT6PUU41YgAOaswbjMQArb9jItnpOn+Jvj+J0e7hW3"
            "njDxBiTgXHJ2ssGaLVCnPHEx1fb0JGpGsifyz27vI2m8VGP4aTRzuwBkIrA5Yh7SXfRlXZuz"
            "m60RhQOEkqEn3cZJK84Q199e+qx/hpwn7uq4+/C+NxXtQLsHn6dSIMLkSDOCHFzfJsqkJh9o"
            "1fjSBrQkK/Dkd1qKOx2bXXLO2ENhl9DW+DeEHIhSSO4fZ2wNA2NAJx3Pqk6Q9Z9hJkJwgzOa"
            "pu5mPrD2KsZqpXWOp+wNBZpA8gbArIfYMbg9bl3zlXjttj9MiU/79JzMaCkd++p20T8tnrHu"
            "ihmi49wz2l8RlqyToSPXLCGGNGbhOG9d/cjQD3ASZl//O/zSVTCCKAIkYm6EAJR5vPgviL6A"
            "bSu+l70o9HH42wvpTh9qppwVwn/AT62DkZo299rwStU55Xju8T+5Z9YbUh/Fdvj4aCtR8WJR"
            "egJb+2hWqWPoNmPjjYZn3OpFY5lmMDNsL+Cmb7b0twIBEQ==";

    this->settings = NULL;

}

Registration::~Registration()
{


}

int Registration::ReadLicense()
{
    assert(this->settings != NULL);
    QString fiStr = this->settings->value("license", "").toString();
    return this->ReadLicense(fiStr);
}

int Registration::ReadLicense(QString fiStr)
{
    QDomDocument doc("mydocument");
    QString errorMsg;

    if (!doc.setContent(fiStr, &errorMsg))
    {
        cout << "Xml Error: "<< errorMsg.toLocal8Bit().constData() << endl;
        return 0;
    }

    QDomElement rootElem = doc.documentElement();

    //Load licensee information
    QDomNodeList dataNodes = rootElem.elementsByTagName("info");
    vector<vector<string> > info;
    for(unsigned int i=0;i<dataNodes.size();i++)
    {
        QDomNode node = dataNodes.item(i);
        if(!node.isElement()) continue;
        QDomElement el = node.toElement();

        QDomElement par = el.firstChildElement();
        while(!par.isNull())
        {
            vector<string> pair;
            string k = par.attribute("k").toLocal8Bit().constData();
            string val = par.attribute("v").toLocal8Bit().constData();
            pair.push_back(k);
            pair.push_back(val);
            par = par.nextSiblingElement();
            info.push_back(pair);
        }
    }

    string infoStr = SerialiseKeyPairs(info);

    //Read keys etc
    QString infosig;
    QDomNodeList infoSigLi = rootElem.elementsByTagName("infosig");
    for(unsigned int i=0;i<infoSigLi.size();i++)
    {
        QDomNode node = infoSigLi.item(i);
        if(!node.isElement()) continue;
        QDomElement el = node.toElement();
        infosig = el.text();
    }

    QString key;
    QDomNodeList keyLi = rootElem.elementsByTagName("key");
    for(unsigned int i=0;i<keyLi.size();i++)
    {
        QDomNode node = keyLi.item(i);
        if(!node.isElement()) continue;
        QDomElement el = node.toElement();
        key = el.text();
    }

    QString keysig;
    QDomNodeList keysigLi = rootElem.elementsByTagName("keysig");
    for(unsigned int i=0;i<keysigLi.size();i++)
    {
        QDomNode node = keysigLi.item(i);
        if(!node.isElement()) continue;
        QDomElement el = node.toElement();
        keysig = el.text();
    }

    //Check licensee information
    int infoOk = 0, keyOk = 0;
    try
    {
        infoOk = VerifyLicense(infoStr, infosig.toLocal8Bit().constData(), key.toLocal8Bit().constData());
        keyOk = VerifyLicense(key.toLocal8Bit().constData(), keysig.toLocal8Bit().constData(), this->masterPubKey);
    }
    catch(Exception err)
    {
        return -3;
    }

    if(!infoOk) return -1;
    if(!keyOk) return -2;
    this->verifiedInfo = info;
    return 1;
}

int Registration::SetLicenseFromFile(QString filename)
{
    QFile fi(filename);
    int ret = fi.open(QIODevice::ReadOnly);
    if(ret==false) return 0;
    QString fiStr = fi.readAll();

    int ret2 = this->ReadLicense(fiStr);

    if(1) //ret2 > 0
        this->settings->setValue("license", fiStr);

    return ret2;
}

std::map<string, string> Registration::GetInfo()
{
    std::map<string, string> out;
    for(unsigned int i=0;i<this->verifiedInfo.size();i++)
    {
        std::vector<std::string> &pair = this->verifiedInfo[i];
        out[pair[0]] = pair[1];
    }
    return out;
}
