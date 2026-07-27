#pragma once
#include <string>
#include <QLibrary>
#include <QCoreApplication>
#include <QDebug>

namespace PCI
{
    class PCIControl
    {
    public:
        PCIControl() = default;
        ~PCIControl() { Destroy(); }

        bool loadDll()
        {
            if (m_loaded) return true;

            QString path = QCoreApplication::applicationDirPath() + "/VC3000HDLL";
            m_lib.setFileName(path);
            if (!m_lib.load()) {
                qWarning() << "VC3000HDLL load failed:" << m_lib.errorString();
                return false;
            }

            m_fnCtor           = resolve<Fn_Ctor>("??0PCIControl@PCI@@QEAA@XZ");
            m_fnDtor           = resolve<Fn_Dtor>("??1PCIControl@PCI@@QEAA@XZ");
            m_fnInitialSystem  = resolve<Fn_Simple>("?InitialSystem@PCIControl@PCI@@QEAAHXZ");
            m_fnSavePar        = resolve<Fn_Simple>("?SaveParam@PCIControl@PCI@@QEAAHXZ");
            m_fnSetPNP         = resolve<Fn_SetPNP>("?setPNP@PCIControl@PCI@@QEAAHIPEBD@Z");
            m_fnOpenSeral      = resolve<Fn_Str>("?openseral@PCIControl@PCI@@QEAAHPEBD@Z");
            m_fnSetOutput      = resolve<Fn_IntBool>("?setoutput@PCIControl@PCI@@QEAAHH_N@Z");
            m_fnGetInput       = resolve<Fn_Int>("?getinput@PCIControl@PCI@@QEAAHH@Z");
            m_fnSetOutputMode  = resolve<Fn_SetOutputMode>("?setOutputMode@PCIControl@PCI@@QEAAHH_NG@Z");
            m_fnSetLight       = resolve<Fn_SetLight>("?setlight@PCIControl@PCI@@QEAAHHGGH_NH@Z");
            m_fnDestroy        = resolve<Fn_Void>("?Destroy@PCIControl@PCI@@QEAAXXZ");

            if (!m_fnInitialSystem || !m_fnSetPNP || !m_fnOpenSeral ||
                !m_fnSetOutput || !m_fnGetInput || !m_fnDestroy || !m_fnCtor) {
                qWarning() << "VC3000HDLL: failed to resolve symbols";
                m_lib.unload();
                return false;
            }

            m_obj = reinterpret_cast<void*>(new char[512]());
            m_fnCtor(m_obj);

            m_loaded = true;
            return true;
        }

        int InitialSystem()
        {
            if (!ensureLoaded()) return -1;
            return m_fnInitialSystem(m_obj);
        }

        int SaveParam()
        {
            if (!ensureLoaded()) return -1;
            return m_fnSavePar ? m_fnSavePar(m_obj) : -1;
        }

        int setPNP(unsigned int enable, const char* mode)
        {
            if (!ensureLoaded()) return -1;
            return m_fnSetPNP(m_obj, enable, mode);
        }

        int openseral(const char* portName)
        {
            if (!ensureLoaded()) return -1;
            return m_fnOpenSeral(m_obj, portName);
        }

        int setoutput(int pointNumber, bool lowOrHigh)
        {
            if (!ensureLoaded()) return -1;
            return m_fnSetOutput(m_obj, pointNumber, lowOrHigh);
        }

        int getinput(int inputPortNumber)
        {
            if (!ensureLoaded()) return -1;
            return m_fnGetInput(m_obj, inputPortNumber);
        }

        int setOutputMode(int pointNumber, bool level, unsigned short durationMs)
        {
            if (!ensureLoaded()) return -1;
            return m_fnSetOutputMode ? m_fnSetOutputMode(m_obj, pointNumber, level, durationMs) : -1;
        }

        int setlight(int portNumber, unsigned short lightValue, unsigned short continueTime,
                     int lightState, bool edgeUpOrDown, int LightTriggerSource)
        {
            if (!ensureLoaded()) return -1;
            return m_fnSetLight ? m_fnSetLight(m_obj, portNumber, lightValue, continueTime,
                                               lightState, edgeUpOrDown, LightTriggerSource) : -1;
        }

        void Destroy()
        {
            if (m_loaded && m_obj) {
                if (m_fnDestroy) m_fnDestroy(m_obj);
                if (m_fnDtor) m_fnDtor(m_obj);
                delete[] reinterpret_cast<char*>(m_obj);
                m_obj = nullptr;
            }
            if (m_lib.isLoaded()) {
                m_lib.unload();
            }
            m_loaded = false;
        }

    private:
        bool ensureLoaded()
        {
            if (m_loaded) return true;
            return loadDll();
        }

        template<typename T>
        T resolve(const char* symbol)
        {
            auto fn = reinterpret_cast<T>(m_lib.resolve(symbol));
            if (!fn) qWarning() << "VC3000HDLL: cannot resolve" << symbol;
            return fn;
        }

        QLibrary m_lib;
        bool m_loaded = false;
        void* m_obj = nullptr;

        using Fn_Ctor          = void  (*)(void*);
        using Fn_Dtor          = void  (*)(void*);
        using Fn_Void          = void  (*)(void*);
        using Fn_Simple        = int   (*)(void*);
        using Fn_Int           = int   (*)(void*, int);
        using Fn_IntBool       = int   (*)(void*, int, bool);
        using Fn_Str           = int   (*)(void*, const char*);
        using Fn_SetPNP        = int   (*)(void*, unsigned int, const char*);
        using Fn_SetOutputMode = int   (*)(void*, int, bool, unsigned short);
        using Fn_SetLight      = int   (*)(void*, int, unsigned short, unsigned short,
                                           int, bool, int);

        Fn_Ctor           m_fnCtor = nullptr;
        Fn_Dtor           m_fnDtor = nullptr;
        Fn_Simple         m_fnInitialSystem = nullptr;
        Fn_Simple         m_fnSavePar = nullptr;
        Fn_SetPNP         m_fnSetPNP = nullptr;
        Fn_Str            m_fnOpenSeral = nullptr;
        Fn_IntBool        m_fnSetOutput = nullptr;
        Fn_Int            m_fnGetInput = nullptr;
        Fn_SetOutputMode  m_fnSetOutputMode = nullptr;
        Fn_SetLight       m_fnSetLight = nullptr;
        Fn_Void           m_fnDestroy = nullptr;
    };

    inline PCIControl& pci() {
        static PCIControl instance;
        return instance;
    }
}
