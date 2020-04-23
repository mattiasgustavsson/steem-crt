// DLLExports.cpp

#define INITGUID
//#include <initguid.h>

#include "windows.h"
#include "Common/ComTry.h"
#include "7Zip/Archive/Zip/ZipHandler.h"
#include "Windows/PropVariant.h"
#include "7Zip/ICoder.h"
#include "7Zip/IPassword.h"
#include "ArchiveAccess.h"

///////////////////////////////////////////////////////////////////////////////
// Original 7-Zip has a number of dll's that export classes using the COM
// mechanism. The classes are exported from DllExport.cpp files in the
// various subdirectories. 

// Hint from microsoft: DEFINE_GUID uses the extern C directive, so
// GUIDs declared outside all conditional defines will be instantiated 
// exactly once, suppressing errors, see:
// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/kmarch/hh/kmarch/Other_2239a4ad-e0b1-41c7-815c-17649f8fc09a.xml.asp

// {23170F69-40C1-278B-0401-080000000100}
DEFINE_GUID(CLSID_CCompressDeflateEncoder, 
0x23170F69, 0x40C1, 0x278B, 0x04, 0x01, 0x08, 0x00, 0x00, 0x00, 0x01, 0x00);

// {23170F69-40C1-278B-0401-080000000000}
DEFINE_GUID(CLSID_CCompressDeflateDecoder, 
0x23170F69, 0x40C1, 0x278B, 0x04, 0x01, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00);

// {23170F69-40C1-278B-0401-090000000100}
DEFINE_GUID(CLSID_CCompressDeflate64Encoder, 
0x23170F69, 0x40C1, 0x278B, 0x04, 0x01, 0x09, 0x00, 0x00, 0x00, 0x01, 0x00);

// {23170F69-40C1-278B-0402-020000000100}
DEFINE_GUID(CLSID_CCompressBZip2Encoder, 
0x23170F69, 0x40C1, 0x278B, 0x04, 0x02, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00);

// {23170F69-40C1-278B-0402-020000000000}
DEFINE_GUID(CLSID_CCompressBZip2Decoder, 
0x23170F69, 0x40C1, 0x278B, 0x04, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00);

// {23170F69-40C1-278B-0401-060000000000}
DEFINE_GUID(CLSID_CCompressImplodeDecoder, 
0x23170F69, 0x40C1, 0x278B, 0x04, 0x01, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00);

// {23170F69-40C1-278B-06F1-0101000000100}
DEFINE_GUID(CLSID_CCryptoZipEncoder, 
0x23170F69, 0x40C1, 0x278B, 0x06, 0xF1, 0x01, 0x01, 0x00, 0x00, 0x01, 0x00);

// {23170F69-40C1-278B-06F1-0101000000000}
DEFINE_GUID(CLSID_CCryptoZipDecoder, 
0x23170F69, 0x40C1, 0x278B, 0x06, 0xF1, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00);

// {23170F69-40C1-278A-1000-000110010000}
DEFINE_GUID(CLSID_CZipHandler, 
  0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x01, 0x00, 0x00);

// {23170F69-40C1-278B-0601-010000000000}
DEFINE_GUID(CLSID_CCrypto_AES128_Decoder, 
0x23170F69, 0x40C1, 0x278B, 0x06, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00);

// {23170F69-40C1-278B-0403-010000000000}
DEFINE_GUID(CLSID_CCompressRar15Decoder, 
0x23170F69, 0x40C1, 0x278B, 0x04, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00);

// {23170F69-40C1-278B-0403-020000000000}
DEFINE_GUID(CLSID_CCompressRar20Decoder, 
0x23170F69, 0x40C1, 0x278B, 0x04, 0x03, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00);

// {23170F69-40C1-278B-0403-030000000000}
DEFINE_GUID(CLSID_CCompressRar29Decoder, 
0x23170F69, 0x40C1, 0x278B, 0x04, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00);

// {23170F69-40C1-278A-1000-000110020000}
DEFINE_GUID(CLSID_CRarHandler, 
  0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x02, 0x00, 0x00);

// {23170F69-40C1-278B-06F1-070100000100}
DEFINE_GUID(CLSID_CCrypto7zAESEncoder, 
0x23170F69, 0x40C1, 0x278B, 0x06, 0xF1, 0x07, 0x01, 0x00, 0x00, 0x01, 0x00);

// {23170F69-40C1-278A-1000-0001100A0000}
DEFINE_GUID(CLSID_CArjHandler, 
  0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x0A, 0x00, 0x00);

// {23170F69-40C1-278A-1000-000110070000}
DEFINE_GUID(CLSID_CBZip2Handler, 
0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x07, 0x00, 0x00);

// {23170F69-40C1-278A-1000-000110060000}
DEFINE_GUID(CLSID_CCabHandler, 
  0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x06, 0x00, 0x00);

// {23170F69-40C1-278A-1000-0001100C0000}
DEFINE_GUID(CLSID_CDebHandler, 
  0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x0C, 0x00, 0x00);

// {23170F69-40C1-278A-1000-000110080000}
DEFINE_GUID(CLSID_CCpioHandler, 
  0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x08, 0x00, 0x00);

// {23170F69-40C1-278A-1000-000110030000}
DEFINE_GUID(CLSID_CGZipHandler, 
0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x03, 0x00, 0x00);

// {23170F69-40C1-278A-1000-000110090000}
DEFINE_GUID(CLSID_CRpmHandler, 
  0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x09, 0x00, 0x00);

// {23170F69-40C1-278A-1000-0001100B0000}
DEFINE_GUID(CLSID_CSplitHandler, 
  0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x0B, 0x00, 0x00);

// {23170F69-40C1-278A-1000-000110040000}
DEFINE_GUID(CLSID_CTarHandler, 
  0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x04, 0x00, 0x00);

// {23170F69-40C1-278B-0401-090000000000}
DEFINE_GUID(CLSID_CCompressDeflate64Decoder, 
0x23170F69, 0x40C1, 0x278B, 0x04, 0x01, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00);

// {23170F69-40C1-278B-0000-000000000000}
DEFINE_GUID(CLSID_CCompressCopyCoder, 
0x23170F69, 0x40C1, 0x278B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

// {23170F69-40C1-278B-0301-010000000000}
DEFINE_GUID(CLSID_CLZMADecoder, 
0x23170F69, 0x40C1, 0x278B, 0x03, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00);

// {23170F69-40C1-278B-0301-010000000100}
DEFINE_GUID(CLSID_CLZMAEncoder, 
0x23170F69, 0x40C1, 0x278B, 0x03, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00);

// {23170F69-40C1-278B-0304-010000000000}
DEFINE_GUID(CLSID_CCompressPPMDDecoder, 
0x23170F69, 0x40C1, 0x278B, 0x03, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00);

// {23170F69-40C1-278B-0304-010000000100}
DEFINE_GUID(CLSID_CCompressPPMDEncoder, 
0x23170F69, 0x40C1, 0x278B, 0x03, 0x04, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00);

// {23170F69-40C1-278B-06F1-070100000000}
DEFINE_GUID(CLSID_CCrypto7zAESDecoder, 
0x23170F69, 0x40C1, 0x278B, 0x06, 0xF1, 0x07, 0x01, 0x00, 0x00, 0x00, 0x00);

///////////////////////////////////////////////////////////////////////////////

HINSTANCE g_hInstance;

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
  if (dwReason == DLL_PROCESS_ATTACH)
    g_hInstance = hInstance;
  return TRUE;
}

/*STDAPI CreateArchiver(const GUID *clsid, const GUID *iid, void **outObject)
{
  COM_TRY_BEGIN
  {
    int needIn = (*iid == IID_IInArchive);
    int needOut = (*iid == IID_IOutArchive);
    if (!needIn && !needOut)
      return E_NOINTERFACE;
    int formatIndex = FindFormatCalssId(clsid);
    if (formatIndex < 0)
      return CLASS_E_CLASSNOTAVAILABLE;
    
    const CArcInfo &arc = *g_Arcs[formatIndex];
    if (needIn)
    {
      *outObject = arc.CreateInArchive();
      ((IInArchive *)*outObject)->AddRef();
    }
    else
    {
      if (!arc.CreateOutArchive)
        return CLASS_E_CLASSNOTAVAILABLE;
      *outObject = arc.CreateOutArchive();
      ((IOutArchive *)*outObject)->AddRef();
    }
  }
  COM_TRY_END
  return S_OK;
}

STDAPI CreateObject(
    const GUID *classID, 
    const GUID *interfaceID, 
    void **outObject)
{
  COM_TRY_BEGIN
  *outObject = 0;
  if (*classID != CLSID_CZipHandler)
    return CLASS_E_CLASSNOTAVAILABLE;
  int needIn = *interfaceID == IID_IInArchive;
  int needOut = *interfaceID == IID_IOutArchive;
  if (needIn || needOut)
  {
    NArchive::NZip::CHandler *temp = new NArchive::NZip::CHandler;
    if (needIn)
    {
      CMyComPtr<IInArchive> inArchive = (IInArchive *)temp;
      *outObject = inArchive.Detach();
    }
    else
    {
      CMyComPtr<IOutArchive> outArchive = (IOutArchive *)temp;
      *outObject = outArchive.Detach();
    }
  }
  else
    return E_NOINTERFACE;
  COM_TRY_END
  return S_OK;
}

STDAPI GetHandlerProperty(PROPID propID, PROPVARIANT *value)
{
  NWindows::NCOM::CPropVariant propVariant;
  switch(propID)
  {
    case NArchive::kName:
      propVariant = L"Zip";
      break;
    case NArchive::kClassID:
    {
      if ((value->bstrVal = ::SysAllocStringByteLen(
          (const char *)&CLSID_CZipHandler, sizeof(GUID))) != 0)
        value->vt = VT_BSTR;
      return S_OK;
    }
    case NArchive::kExtension:
      propVariant = L"zip jar xpi";
      break;
    case NArchive::kUpdate:
      propVariant = true;
      break;
    case NArchive::kKeepName:
      propVariant = false;
      break;
  }
  propVariant.Detach(value);
  return S_OK;
}

*/