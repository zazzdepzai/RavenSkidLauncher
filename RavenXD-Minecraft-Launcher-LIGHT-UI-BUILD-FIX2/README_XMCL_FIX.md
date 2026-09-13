# RavenXD XMCL Core Fix

The standalone core uses published, non-workspace package releases:

- @xmcl/core 2.13.0
- @xmcl/installer 5.4.0

These versions use normal npm dependency ranges instead of `workspace:*`, so the GitHub Actions build can run `npm install` inside `core` without requiring the XMCL monorepo/pnpm workspace.

Minecraft 1.8.9 installation uses the Mojang version manifest and XMCL installer. Forge installation requests the Forge 1.8.9 version list explicitly.
