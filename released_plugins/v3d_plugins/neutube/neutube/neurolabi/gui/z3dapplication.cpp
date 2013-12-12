#include "z3dapplication.h"

#include "QsLog.h"
#include "z3dshaderprogram.h"
#include "z3dgpuinfo.h"

#if !defined(Q_OS_WIN) && !defined(Q_OS_DARWIN)
#include <sys/utsname.h> // for uname
#endif

#ifdef Q_OS_WIN
#include <windows.h>
#include <string>
#include <sstream>
typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);
//#define PRODUCT_PROFESSIONAL	0x00000030   //defined by winnt.h of mingw
#define VER_SUITE_WH_SERVER	0x00008000
#endif

#ifdef Q_OS_DARWIN
#include <CoreServices/CoreServices.h>
#include <CoreFoundation/CFBundle.h>
#endif

namespace {

#ifdef Q_OS_WIN
bool windowsVersionName(std::wstring &osString)
{
  OSVERSIONINFOEX osvi;
  SYSTEM_INFO si;
  BOOL bOsVersionInfoEx;
  DWORD dwType;
  ZeroMemory(&si, sizeof(SYSTEM_INFO));
  ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
  bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO*) &osvi);
  if(bOsVersionInfoEx == 0)
    return false; // Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.
  PGNSI pGNSI = (PGNSI) GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetNativeSystemInfo");
  if(NULL != pGNSI)
    pGNSI(&si);
  else GetSystemInfo(&si); // Check for unsupported OS
  if (VER_PLATFORM_WIN32_NT != osvi.dwPlatformId || osvi.dwMajorVersion <= 4 ) {
    return false;
  }
  std::wstringstream os;
  os << L"Microsoft "; // Test for the specific product. if ( osvi.dwMajorVersion == 6 )
  {
    if( osvi.dwMinorVersion == 0 )
    {
      if( osvi.wProductType == VER_NT_WORKSTATION )
        os << "Windows Vista ";
      else os << "Windows Server 2008 ";
    }  if ( osvi.dwMinorVersion == 1 )
    {
      if( osvi.wProductType == VER_NT_WORKSTATION )
        os << "Windows 7 ";
      else os << "Windows Server 2008 R2 ";
    }
    PGPI pGPI = (PGPI) GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetProductInfo");
    pGPI( osvi.dwMajorVersion, osvi.dwMinorVersion, 0, 0, &dwType);
    switch( dwType )
    {
    case PRODUCT_ULTIMATE:
      os << "Ultimate Edition";
      break;
    case PRODUCT_PROFESSIONAL:
      os << "Professional";
      break;
    case PRODUCT_HOME_PREMIUM:
      os << "Home Premium Edition";
      break;
    case PRODUCT_HOME_BASIC:
      os << "Home Basic Edition";
      break;
    case PRODUCT_ENTERPRISE:
      os << "Enterprise Edition";
      break;
    case PRODUCT_BUSINESS:
      os << "Business Edition";
      break;
    case PRODUCT_STARTER:
      os << "Starter Edition";
      break;
    case PRODUCT_CLUSTER_SERVER:
      os << "Cluster Server Edition";
      break;
    case PRODUCT_DATACENTER_SERVER:
      os << "Datacenter Edition";
      break;
    case PRODUCT_DATACENTER_SERVER_CORE:
      os << "Datacenter Edition (core installation)";
      break;
    case PRODUCT_ENTERPRISE_SERVER:
      os << "Enterprise Edition";
      break;
    case PRODUCT_ENTERPRISE_SERVER_CORE:
      os << "Enterprise Edition (core installation)";
      break;
    case PRODUCT_ENTERPRISE_SERVER_IA64:
      os << "Enterprise Edition for Itanium-based Systems";
      break;
    case PRODUCT_SMALLBUSINESS_SERVER:
      os << "Small Business Server";
      break;
    case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
      os << "Small Business Server Premium Edition";
      break;
    case PRODUCT_STANDARD_SERVER:
      os << "Standard Edition";
      break;
    case PRODUCT_STANDARD_SERVER_CORE:
      os << "Standard Edition (core installation)";
      break;
    case PRODUCT_WEB_SERVER:
      os << "Web Server Edition";
      break;
    }
  } if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
  {
    if( GetSystemMetrics(SM_SERVERR2) )
      os <<  "Windows Server 2003 R2, ";
    else if ( osvi.wSuiteMask & VER_SUITE_STORAGE_SERVER )
      os <<  "Windows Storage Server 2003";
    else if ( osvi.wSuiteMask & VER_SUITE_WH_SERVER )
      os <<  "Windows Home Server";
    else if( osvi.wProductType == VER_NT_WORKSTATION &&
             si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
    {
      os <<  "Windows XP Professional x64 Edition";
    }
    else os << "Windows Server 2003, ";  // Test for the server type.
    if ( osvi.wProductType != VER_NT_WORKSTATION )
    {
      if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_IA64 )
      {
        if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
          os <<  "Datacenter Edition for Itanium-based Systems";
        else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
          os <<  "Enterprise Edition for Itanium-based Systems";
      }   else if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64 )
      {
        if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
          os <<  "Datacenter x64 Edition";
        else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
          os <<  "Enterprise x64 Edition";
        else os <<  "Standard x64 Edition";
      }   else
      {
        if ( osvi.wSuiteMask & VER_SUITE_COMPUTE_SERVER )
          os <<  "Compute Cluster Edition";
        else if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
          os <<  "Datacenter Edition";
        else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
          os <<  "Enterprise Edition";
        else if ( osvi.wSuiteMask & VER_SUITE_BLADE )
          os <<  "Web Edition";
        else os <<  "Standard Edition";
      }
    }
  }
  if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
  {
    os << "Windows XP ";
    if( osvi.wSuiteMask & VER_SUITE_PERSONAL )
      os <<  "Home Edition";
    else os <<  "Professional";
  }
  if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
  {
    os << "Windows 2000 ";  if ( osvi.wProductType == VER_NT_WORKSTATION )
    {
      os <<  "Professional";
    }
    else
    {
      if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
        os <<  "Datacenter Server";
      else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
        os <<  "Advanced Server";
      else os <<  "Server";
    }
  } // Include service pack (if any) and build number.
  if(wcslen(osvi.szCSDVersion) > 0) {
    os << " " << osvi.szCSDVersion;
  }
  os << L" (build " << osvi.dwBuildNumber << L")";
  if ( osvi.dwMajorVersion >= 6 ) {
    if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64 )
      os <<  ", 64-bit";
    else if (si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_INTEL )
      os << ", 32-bit";
  }
  osString = os.str();
  return true;
}
#endif

} // namespace

Z3DApplication* Z3DApplication::m_app = NULL;

Z3DApplication::Z3DApplication(const QString &appDir)
  : m_applicationDirPath(appDir)
  , m_initialized(false)
  , m_glInitialized(false)
  , m_stereoViewSupported(false)
{
  m_app = this;
}

Z3DApplication::~Z3DApplication()
{
}

void Z3DApplication::initialize()
{
  if (m_initialized) {
    return;
  }

  detectOS();
  LINFO() << "OS:" << m_osString;

  // shader path
  m_shaderPath = ":/Resources/shader";

  // font path
  m_fontPath = ":/Resources/fonts";

  m_initialized = true;
}

void Z3DApplication::deinitialize()
{
  m_initialized = false;
}

bool Z3DApplication::initializeGL()
{
  if (!m_initialized) {
    initialize();
  }

  if (m_glInitialized) {
    LINFO() << "OpenGL already initialized. Skip.";
    return false;
  }

  GLenum err = glewInit();
  if (err != GLEW_OK) {
    m_errorMsg = "glewInit failed, error: ";
    m_errorMsg += reinterpret_cast<const char*>(glewGetErrorString(err));
    LERROR() << m_errorMsg;
    LWARN() << "3D functions will be disabled.";
    return false;
  } else {
    //LINFO() << "GLEW version:" << (const char*)(glewGetString(GLEW_VERSION));
    Z3DGpuInfoInstance.logGpuInfo();
    if (Z3DGpuInfoInstance.isSupported()) {
      m_glInitialized = true;
      return m_glInitialized;
    } else {
      m_errorMsg = Z3DGpuInfoInstance.getNotSupportedReason();
      m_glInitialized = false;
      return m_glInitialized;
    }
  }
}

bool Z3DApplication::deinitializeGL()
{
  m_glInitialized = false;
  return true;
}


QString Z3DApplication::getShaderPath(const QString& filename) const
{
  return m_shaderPath + (filename.isEmpty() ? QString("") : QString("/") + filename);
}

QString Z3DApplication::getFontPath(const QString& filename) const
{
  return m_fontPath + (filename.isEmpty() ? QString("") : QString("/") + filename);
}

void Z3DApplication::detectOS()
{
#ifdef Q_OS_WIN
  std::wstring osString;
  if (windowsVersionName(osString)) {
    m_osString = QString::fromStdWString(osString);
  } else {
    switch (QSysInfo::WindowsVersion) {
    case QSysInfo::WV_NT:
      m_osString = "Windows NT (operating system version 4.0)";
      break;
    case QSysInfo::WV_2000:
      m_osString = "Windows 2000 (operating system version 5.0)";
      break;
    case QSysInfo::WV_XP:
      m_osString = "Windows XP (operating system version 5.1)";
      break;
    case QSysInfo::WV_2003:
      m_osString = "Windows Server 2003, Windows Server 2003 R2, Windows Home Server, \
          Windows XP Professional x64 Edition (operating system version 5.2)";
          break;
    case QSysInfo::WV_VISTA:
      m_osString = "Windows Vista, Windows Server 2008 (operating system version 6.0)";
      break;
    case QSysInfo::WV_WINDOWS7:
      m_osString = "Windows 7, Windows Server 2008 R2 (operating system version 6.1)";
      break;
    default:
      m_osString = "unknown win os";
    }
  }
#elif defined(Q_OS_DARWIN)
  switch (QSysInfo::MacintoshVersion) {
  case QSysInfo::MV_10_5:
    m_osString = "Mac OS X LEOPARD";
    break;
  case QSysInfo::MV_10_6:
    m_osString = "Mac OS X SNOW LEOPARD";
    break;
#if (QT_VERSION > QT_VERSION_CHECK(4, 7, 5))
  case QSysInfo::MV_10_7:
    m_osString = "Mac OS X LION";
    break;
#endif

#if (QT_VERSION > QT_VERSION_CHECK(4, 8, 1))
  case QSysInfo::MV_10_8:
    m_osString = "Mac OS X MOUNTAIN LION";
    break;
#endif
  default:
    m_osString = "unsupported mac os";
    return;
  }
  SInt32 majorVersion,minorVersion,bugFixVersion;
  Gestalt(gestaltSystemVersionMajor, &majorVersion);
  Gestalt(gestaltSystemVersionMinor, &minorVersion);
  Gestalt(gestaltSystemVersionBugFix, &bugFixVersion);
  m_osString += QString(" %1.%2.%3").arg(majorVersion).arg(minorVersion).arg(bugFixVersion);
#else
  utsname name;
  if (uname(&name) != 0)
    return; // command not successful

  m_osString = QString("%1 %2 %3 %4").arg(name.sysname).arg(name.release).arg(name.version).arg(name.machine);

#endif // Q_OS_WIN
}
