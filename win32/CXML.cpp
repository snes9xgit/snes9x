#include "CXML.h"
#include <windows.h>
#include <stdio.h>
#include "_tfwopen.h"
#ifndef UNICODE
#define _tFromTCHAR(x) CPToWide(x,CP_ACP)
#define _tToTCHAR(x) WideToCP(x,CP_ACP)
#else
#define _tFromTCHAR
#define _tToTCHAR
#endif

CXML::CXML(void)
{
    pXMLDoc = NULL;
    xmlLoaded = false;
    nodeContent = NULL;
    attrValue = NULL;
}

CXML::~CXML(void)
{
    unloadXml();
}

void CXML::unloadXml()
{
    if(pXMLDoc) {
        pXMLDoc->Release();
        pXMLDoc = NULL;
    }
    if(nodeContent) {
        delete [] nodeContent;
        nodeContent = NULL;
    }
    if(attrValue) {
        delete [] attrValue;
        attrValue = NULL;
    }
    xmlLoaded = false;
}

bool CXML::loadXmlFile(TCHAR const *file)
{
    unloadXml();

    if(!file)
        return false;

    TCHAR errorMsg[MAX_PATH + 50];

    HRESULT hr = CoCreateInstance(CLSID_DOMDocument,NULL,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&pXMLDoc));

    if(FAILED(hr)) {
        MessageBox(NULL, TEXT("Error creating XML Parser"), TEXT("XML Error"),
			MB_OK|MB_ICONEXCLAMATION);
        pXMLDoc = NULL;
        return false;
    }


    VARIANT fileName;
	VARIANT_BOOL ret;
	fileName.vt = VT_BSTR;
	fileName.bstrVal = SysAllocString(_tFromTCHAR(file));
    hr = pXMLDoc->load(fileName,&ret);
	SysFreeString(fileName.bstrVal);

    if(FAILED(hr) || hr==S_FALSE) {
        _stprintf(errorMsg,TEXT("Error loading XML file:\n%s"),file);
        MessageBox(NULL, errorMsg, TEXT("XML Error"),
			MB_OK|MB_ICONEXCLAMATION);
        unloadXml();
        return false;
    }

    hr = pXMLDoc->get_documentElement(&pXrootElement);
	if(FAILED(hr) || hr==S_FALSE) {
		_stprintf(errorMsg,TEXT("Error loading root element from file:\n%s"),file);
		MessageBox(NULL, errorMsg, TEXT("XML Error"), MB_OK|MB_ICONEXCLAMATION);
        unloadXml();
		return false;
	}

    xmlLoaded = true;
    return true;
}

IXMLDOMNode *CXML::getNode(TCHAR const *searchNode)
{
    IXMLDOMNode *pXDN;
    BSTR queryString=SysAllocString(_tFromTCHAR(searchNode));
    HRESULT hr = pXMLDoc->selectSingleNode(queryString,&pXDN);
	SysFreeString(queryString);

    return pXDN;
}

TCHAR *CXML::getAttribute(TCHAR const *searchNode, TCHAR const *attrName)
{
    IXMLDOMNode *pXDN = getNode(searchNode);
    
    if(!pXDN)
        return NULL;

    VARIANT attributeValue;
	BSTR attributeName;

    IXMLDOMElement * pXDE = NULL;
    HRESULT hr = pXDN->QueryInterface(IID_PPV_ARGS(&pXDE));
    if(FAILED(hr)) {
        pXDN->Release();
        return NULL;
    }

    attributeName=SysAllocString(_tFromTCHAR(attrName));
	pXDE->getAttribute(attributeName,&attributeValue);
	SysFreeString(attributeName);
	pXDE->Release();

    if(attributeValue.vt!=VT_BSTR)
        return NULL;

    if(attrValue) {
        delete [] attrValue;
        attrValue = NULL;
    }

    attrValue = new TCHAR[lstrlen(_tToTCHAR(attributeValue.bstrVal)) + 1];
    lstrcpy(attrValue,_tToTCHAR(attributeValue.bstrVal));

    SysFreeString(attributeValue.bstrVal);

    return attrValue;
}

TCHAR *CXML::getNodeContent(TCHAR const *searchNode)
{
    IXMLDOMNode *pXDN = getNode(searchNode);
    
    if(!pXDN)
        return NULL;

    BSTR nodeText;

    HRESULT hr = pXDN->get_text(&nodeText);
    pXDN->Release();

	if(hr != S_OK)
		return NULL;

    if(nodeContent) {
        delete [] nodeContent;
        nodeContent = NULL;
    }

    nodeContent = new TCHAR[lstrlen(_tToTCHAR(nodeText)) + 1];
    lstrcpy(nodeContent,_tToTCHAR(nodeText));

	SysFreeString(nodeText);

    return nodeContent;
}
