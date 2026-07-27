#include "ScannerHook.h"
#include <QTimer>
#include <QGuiApplication>
#include <windows.h>

static ScannerHook* g_scannerInstance = nullptr;
static HKL g_prevLayout = nullptr;  // 保存原始键盘布局

namespace {

constexpr DWORD kScannerInterKeyTimeoutMs = 100;

bool isShiftKey(DWORD vkCode)
{
    return vkCode == VK_SHIFT || vkCode == VK_LSHIFT || vkCode == VK_RSHIFT;
}

bool scannerInputExpired(DWORD currentTime, DWORD previousTime)
{
    return previousTime != 0
            && static_cast<DWORD>(currentTime - previousTime) > kScannerInterKeyTimeoutMs;
}

QString scannerTextFromKey(DWORD vkCode, bool shiftDown)
{
    if (vkCode >= 'A' && vkCode <= 'Z') {
        return QString(QChar(QLatin1Char(static_cast<char>(vkCode))));
    }

    if (vkCode >= '0' && vkCode <= '9') {
        if (shiftDown && vkCode == '2') {
            return QStringLiteral("@");
        }
        return QString(QChar(QLatin1Char(static_cast<char>(vkCode))));
    }

    if (vkCode >= VK_NUMPAD0 && vkCode <= VK_NUMPAD9) {
        return QString::number(static_cast<int>(vkCode - VK_NUMPAD0));
    }

    if (vkCode == VK_OEM_MINUS || vkCode == VK_SUBTRACT) {
        return QStringLiteral("-");
    }

    return QString();
}

}

ScannerHook::ScannerHook(QObject* parent)
    : QObject(parent)
{
}

ScannerHook::~ScannerHook()
{
    m_continuous = false;
    m_stopping = true;
    if (g_scannerInstance == this) {
        g_scannerInstance = nullptr;
    }
    stop();
}

bool ScannerHook::start(int timeoutMs)
{
    if (m_hook)
        return false;

    m_stopping = false;
    m_buffer.clear();
    m_shiftDown = false;
    m_lastKeyTime = 0;
    m_timeoutMs = timeoutMs;

    // 临时切换到英文键盘布局，防止中文 IME 拦截扫码枪输入
    g_prevLayout = GetKeyboardLayout(0);
    HKL enLayout = LoadKeyboardLayoutW(L"00000409", KLF_ACTIVATE);
    if (enLayout) {
        ActivateKeyboardLayout(enLayout, 0);
    }

    m_hook = SetWindowsHookEx(
        WH_KEYBOARD_LL,
        keyboardProc,
        GetModuleHandle(nullptr),
        0);

    if (!m_hook)
        return false;

    g_scannerInstance = this;

    // 超时（-1 = 无限）
    if (m_timeoutMs > 0)
    {
        QTimer::singleShot(m_timeoutMs, this, [this]() {
            finish();
            });
    }

    return true;
}

void ScannerHook::stop()
{
    if (g_scannerInstance == this) {
        g_scannerInstance = nullptr;
    }

    if (m_hook)
    {
        UnhookWindowsHookEx(m_hook);
        m_hook = nullptr;
    }

    // 恢复原始键盘布局（仅非持续模式恢复，持续模式保持英文）
    if (!m_continuous && g_prevLayout) {
        ActivateKeyboardLayout(g_prevLayout, 0);
        g_prevLayout = nullptr;
    }
}

void ScannerHook::finish()
{
    const bool shouldRestart = m_continuous;
    const QString code = m_buffer;
    m_buffer.clear();
    m_lastKeyTime = 0;

    stop();

    if (!code.isEmpty())
    {
        QTimer::singleShot(0, this, [this, code]() {
            if (m_stopping) {
                return;
            }
            emit scanFinished(code);
        });
    }

    // 持续监听模式：自动重新挂钩子
    if (shouldRestart && m_continuous)
    {
        start(-1);
    }
}

bool ScannerHook::startContinuous()
{
    m_continuous = true;
    return start(-1);
}

void ScannerHook::stopContinuous()
{
    m_continuous = false;
    m_stopping = true;
    stop();

    // 恢复键盘布局
    if (g_prevLayout) {
        ActivateKeyboardLayout(g_prevLayout, 0);
        g_prevLayout = nullptr;
    }
}

bool ScannerHook::isContinuous() const
{
    return m_continuous;
}

LRESULT CALLBACK ScannerHook::keyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        auto* key = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);

        if (!g_scannerInstance)
            return CallNextHookEx(nullptr, nCode, wParam, lParam);

        if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP)
        {
            if (isShiftKey(key->vkCode)) {
                g_scannerInstance->m_shiftDown = false;
            }
            return CallNextHookEx(nullptr, nCode, wParam, lParam);
        }

        if (wParam != WM_KEYDOWN && wParam != WM_SYSKEYDOWN)
            return CallNextHookEx(nullptr, nCode, wParam, lParam);

        if (isShiftKey(key->vkCode))
        {
            g_scannerInstance->m_shiftDown = true;
            return CallNextHookEx(nullptr, nCode, wParam, lParam);
        }

        // Enter 结束
        if (key->vkCode == VK_RETURN)
        {
            if (scannerInputExpired(key->time, g_scannerInstance->m_lastKeyTime)) {
                g_scannerInstance->m_buffer.clear();
                g_scannerInstance->m_lastKeyTime = 0;
            }
            const bool hasScanText = !g_scannerInstance->m_buffer.isEmpty();
            g_scannerInstance->finish();
            return hasScanText ? 1 : CallNextHookEx(nullptr, nCode, wParam, lParam);
        }

        const QString text = scannerTextFromKey(key->vkCode, g_scannerInstance->m_shiftDown);
        if (!text.isEmpty()) {
            if (scannerInputExpired(key->time, g_scannerInstance->m_lastKeyTime)) {
                g_scannerInstance->m_buffer.clear();
            }
            g_scannerInstance->m_buffer.append(text);
            g_scannerInstance->m_lastKeyTime = key->time;
        } else {
            g_scannerInstance->m_buffer.clear();
            g_scannerInstance->m_lastKeyTime = 0;
        }
    }

    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}
