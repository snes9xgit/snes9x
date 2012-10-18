#ifndef CXML_H
#define CXML_H

#include "msxml2.h"
#include <tchar.h>

class CXML
{
private:
    IXMLDOMDocument * pXMLDoc;
    IXMLDOMElement * pXrootElement;
    bool xmlLoaded;
    TCHAR *nodeContent;
    TCHAR *attrValue;

    IXMLDOMNode *getNode(TCHAR const *searchNode);

public:
    CXML(void);
    ~CXML(void);

    bool loadXmlFile(TCHAR const *xmlFile);
    void unloadXml();

    TCHAR *getAttribute(TCHAR const *searchNode, TCHAR const *attrName);
    TCHAR *getNodeContent(TCHAR const *searchNode);
};

#endif