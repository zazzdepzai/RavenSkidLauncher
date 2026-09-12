#pragma once

namespace RavenXD_RPC {
    bool Initialize(const char* clientId = nullptr);
    void UpdatePresence(const char* details, const char* state);
    void UpdateAutoCycle();
    void Shutdown();
    bool IsConnected();
}
