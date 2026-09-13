import process from 'node:process'
import { spawn } from 'node:child_process'
import fs from 'node:fs/promises'
import { Version, launch } from '@xmcl/core'
import {
  getVersionList,
  install,
  getForgeVersionList,
  installForge,
  installDependencies,
} from '@xmcl/installer'

function fail(message, error) {
  console.error(`[RavenXD-Core] ${message}`)
  if (error) console.error(error?.stack || error?.message || String(error))
  process.exitCode = 1
}

const [command, ...args] = process.argv.slice(2)

// --- Install-cache helpers -------------------------------------------------
// Previously every "launch"/"forge-launch" call re-ran installDependencies()
// against the ENTIRE .minecraft directory (all libraries + all assets) on
// every single play, which is what made the launcher feel like it "always
// re-downloads the whole .minecraft folder". installDependencies() only
// fetches files that are actually missing/corrupt, but the full walk+hash of
// every asset on every launch is slow and, if a file was ever left in a bad
// state, looks identical to a full re-download to the user.
//
// Fix: once a version has been verified successfully, drop a small marker
// file next to it and trust that marker on subsequent launches. The explicit
// "repair" command (wired to a "Repair / Reinstall" button in the UI) clears
// the marker and forces a full re-verify/re-download on demand.
function markerPath(gamePath, versionId) {
  return `${gamePath}/versions/${versionId}/.raven-verified`
}

async function isVerified(gamePath, versionId) {
  return fs.access(markerPath(gamePath, versionId)).then(() => true).catch(() => false)
}

async function markVerified(gamePath, versionId) {
  await fs.writeFile(markerPath(gamePath, versionId), new Date().toISOString()).catch(() => {})
}

async function clearVerified(gamePath, versionId) {
  await fs.rm(markerPath(gamePath, versionId), { force: true }).catch(() => {})
}

async function installMinecraft(gamePath) {
  if (!gamePath) throw new Error('Missing game directory.')
  console.log(`[RavenXD-Core] Installing Minecraft 1.8.9 into: ${gamePath}`)

  const manifest = await getVersionList()
  const version = manifest.versions.find(v => v.id === '1.8.9')
  if (!version) throw new Error('Minecraft 1.8.9 was not found in Mojang version manifest.')

  await install(version, gamePath)
  const resolved = await Version.parse(gamePath, '1.8.9')
  await installDependencies(resolved)
  await markVerified(gamePath, '1.8.9')
  console.log('[RavenXD-Core] INSTALL_OK')
}

async function installForge18(gamePath, javaPath) {
  if (!gamePath) throw new Error('Missing game directory.')
  console.log('[RavenXD-Core] Installing Minecraft 1.8.9 Forge...')

  const base = await Version.parse(gamePath, '1.8.9').catch(() => null)
  if (!base) {
    await installMinecraft(gamePath)
  }

  const page = await getForgeVersionList({ minecraft: '1.8.9' })
  const versions = page?.versions || []
  if (!versions.length) throw new Error('Forge did not return a 1.8.9 version list.')

  const preferred =
    versions.find(v => v.type === 'recommended') ||
    versions.find(v => v.type === 'latest') ||
    versions[0]

  const options = javaPath ? { java: javaPath } : undefined
  const forgeId = await installForge(preferred, gamePath, options)
  const resolved = await Version.parse(gamePath, forgeId)
  await installDependencies(resolved)
  await markVerified(gamePath, forgeId)
  console.log(`[RavenXD-Core] FORGE_OK ${forgeId}`)
}

async function start(gamePath, javaPath, username, version = '1.8.9') {
  if (!gamePath) throw new Error('Missing game directory.')
  if (!javaPath) throw new Error('Missing Java executable path.')
  const name = (username || 'RavenXD').trim() || 'RavenXD'

  console.log(`[RavenXD-Core] Launching ${version} as ${name}`)
  const resolved = await Version.parse(gamePath, version)
  if (!(await isVerified(gamePath, version))) {
    console.log('[RavenXD-Core] Verifying libraries/assets (first launch of this version only)...')
    await installDependencies(resolved)
    await markVerified(gamePath, version)
  }

  const child = await launch({
    gamePath,
    resourcePath: gamePath,
    javaPath,
    version,
    userType: 'legacy',
    gameProfile: { name, id: 'RavenXD-Offline-Profile' },
    launcherName: 'RavenXD',
    launcherBrand: 'RavenXD',
    maxMemory: 2048,
    minMemory: 512,
    extraExecOption: { detached: true, stdio: 'ignore' },
  })

  if (!child || !child.pid) throw new Error('Minecraft process could not be created.')
  child.once('error', (error) => console.error(`[RavenXD-Core] Minecraft process error: ${error?.message || error}`))
  child.unref()
  console.log(`[RavenXD-Core] LAUNCH_OK pid=${child.pid}`)
}

async function forgeLaunch(gamePath, javaPath, username) {
  let entries = await fs.readdir(`${gamePath}/versions`, { withFileTypes: true }).catch(() => [])
  let version = entries.map(e => e.name).find(n => n.startsWith('1.8.9-forge-'))

  if (!version) {
    await installForge18(gamePath, javaPath)
    entries = await fs.readdir(`${gamePath}/versions`, { withFileTypes: true }).catch(() => [])
    version = entries.map(e => e.name).find(n => n.startsWith('1.8.9-forge-'))
  }

  if (!version) throw new Error('Forge 1.8.9 installation did not produce a version directory.')
  await start(gamePath, javaPath, username, version)
}

async function repair(gamePath) {
  if (!gamePath) throw new Error('Missing game directory.')
  console.log('[RavenXD-Core] Repair requested: forcing a full re-verify/re-download.')
  await clearVerified(gamePath, '1.8.9')
  await installMinecraft(gamePath)
}

try {
  if (command === 'install') await installMinecraft(args[0])
  else if (command === 'repair') await repair(args[0])
  else if (command === 'install-forge') await installForge18(args[0], args[1])
  else if (command === 'launch') await start(args[0], args[1], args[2], '1.8.9')
  else if (command === 'forge-launch') await forgeLaunch(args[0], args[1], args[2])
  else throw new Error('Usage: install | repair | install-forge | launch | forge-launch')
} catch (error) {
  fail('Operation failed.', error)
}
