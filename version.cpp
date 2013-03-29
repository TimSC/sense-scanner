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
    this->masterPubKey = "MIIEIDANBgkqhkiG9w0BAQEFAAOCBA0AMIIECAKCBAEAqDa9RdBl9ziUm6/8CLzm5DN8m7S+"
            "g4E/7bbbPXlgesbnM5gT83azb7BrhKxM6aDwNX88Oq/OKgwmi3lK6egyYYQYjZ81o4DIgOvn"
            "PxI6GAmzD9i8Jbj/SJQaxPJJGEHjY3E/2eF3qL7FsvRtDFjoqwb942sHt6gPsHMIKD7CYks6"
            "ya3hz5nGowry8T7CxiEmF4mo3P+OxkeSXoGsZc/qQUYhon0pYM/GukHAZnAkbXOKn0vsQZVS"
            "M47ZQ+93I6dQBcUnYMEQx0QRGZpehl/6QbW6vQzTHQuui99Tj8UOudjoj5stF6B2zkJF2yAa"
            "AlVW6Qe+0lJ1M+cA9ucVIha3WHMxS9FZlYp1fil851k3lry0jchfpltX3ybGltX80LKle+iH"
            "5QN9FZgx13dC7tpoWBzZ7Qzg9Sxw4AHglssQL/eOhm2lb5EiIgXkxKy6hkmYpBiq82GPy/p9"
            "zaz1thvpnDMtH3s3piGnv1RF2JSpuiHcDigoUDJNMQOMlotSNIJTO8tOx0RK9MXi98//5+Th"
            "V01UgVuWuy5sccNMd8asM30WT2PGgptZxqbcwuBm5qfFUUA7XrQgOzgXPtV1OXJpIYEndGLs"
            "UhUadAss7MD+oigIpaCl139d3OGl5tpwoITOOf5YfOCMk/PN1VLXxD0MRjq5Sy/zt6vNK+xD"
            "ESpPqK4fiFy9Tv26OscVU92zOwM+4650UCUkGeKA9MTMP3nkg3Yp8cbIfnrVkBoxk/mvTQbw"
            "9NE7CG9FQ/UDhWhiWQnVN0MVNvwS9ZqkUqCi1uZEK0OJSKmrgagZQzm0QmLVs8q9pSobm2tp"
            "lLomsgVfcl6VxxVxB8oorZ1G8FOtIyVQz4CIAbkVUvfeGgF18KVpi5A3e4Ko2seBFPOhV7zX"
            "IHponzeu/lXKTknj1l2A/RskaVzKaDJRyMTFO21rLIU6dAYL0CUPCN1hqk1XXVjbsRecqigW"
            "hPkfpYKRPfgpC1V5RiTxqcAZTlmNpjz5mxEAuZyyprLtEH41owCvTZ61Yp+S1x6ADafZgZvL"
            "ldZs7Cjia2uIKOZvnhpoPPG2SMc+96iDKbF+87rsSH0Jhi7Ki7Z4ZsgWSUufQc224aFiA3x8"
            "kgGe47PEtbUWkGIv7BOJ+6in4v0Pny/bTGwUiher/z2RAPEP6hpTmts3Oxq+6ekB9sG/TlwE"
            "x8Rv8r5/HTfiPo5Wxu8fYWvp3TuifC11KoqENXMW27elFGund6zl4JxdrLcr3XoPK3CNFz0s"
            "J377T0x/9ZdJBP7nEz4lIkHqJ4Fgu/3HUUDRx0KA+RUrelfaVfqFxpOm8forbVBzvUffiSM7"
            "ZdnP1nMqU5AzVC+RQtfIq/qc69YPOUuFEbWbubu/lQIBEQ==";

    this->settings = NULL;

}

Registration::~Registration()
{


}

int Registration::ReadLicense()
{
    QDomDocument doc("mydocument");
    QString errorMsg;
    /*QFile fi("xmllicense.xml");
    int ret = fi.open(QIODevice::ReadOnly);
    if(ret==false) return 0;

    QString fiStr = fi.readAll();

    assert(this->settings != NULL);
    this->settings->setValue("license", fiStr);*/
    QString fiStr = this->settings->value("license", "").toString();

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
    int infoOk = VerifyLicense(infoStr, infosig.toLocal8Bit().constData(), key.toLocal8Bit().constData());
    int keyOk = VerifyLicense(key.toLocal8Bit().constData(), keysig.toLocal8Bit().constData(), this->masterPubKey);
    if(!infoOk) return -1;
    if(!keyOk) return -2;
    this->verifiedInfo = info;
    return 1;
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
