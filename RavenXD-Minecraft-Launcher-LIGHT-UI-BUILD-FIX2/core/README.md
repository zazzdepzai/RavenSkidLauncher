# RavenXD Minecraft Core

This folder is the Node.js backend used by the RavenXD C++ launcher.

It uses the published XMCL packages:
- `@xmcl/core` 2.16.1 — Minecraft version parsing and launching.
- `@xmcl/installer` 6.3.3 — Minecraft, libraries, assets and Forge installation.

Do not run this folder with the archived XMCL monorepo's workspace setup. It is a standalone npm project, so `npm install --omit=dev` is sufficient.

The GitHub Actions workflow installs Node.js 20, installs these dependencies, then bundles `node.exe`, this folder, and `node_modules` into the launcher package.
