#pragma once
#include <string>
#include <Windows.h>

namespace Launcher {
class DiscordRPC {
public:
    bool Start(const std::string& username);
    void Stop();
    void Update(const std::string& state, const std::string& details);
    bool Connected() const;
private:
    HANDLE m_Pipe = nullptr;
    long long m_Nonce = 1;
    bool m_Connected = false;
    bool WriteFrame(int opcode, const std::string& json);
};
}
