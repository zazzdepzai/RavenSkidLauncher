import process from 'node:process'
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

async function installMinecraft(gamePath) {
  if (!gamePath) throw new Error('Missing game directory.')
  console.log(`[RavenXD-Core] Installing Minecraft 1.8.9 into: ${gamePath}`)

  const manifest = await getVersionList()
  const version = manifest.versions.find(v => v.id === '1.8.9')
  if (!version) throw new Error('Minecraft 1.8.9 was not found in Mojang version manifest.')

  await install(version, gamePath)
  console.log('[RavenXD-Core] INSTALL_OK')
}

async function installForge18(gamePath, javaPath) {
  if (!gamePath) throw new Error('Missing game directory.')
  console.log('[RavenXD-Core] Installing Minecraft 1.8.9 Forge...')

  const base = await Version.parse(gamePath, '1.8.9').catch(() => null)
  if (!base) {
    await installMinecraft(gamePath)
  }

  const page = await getForgeVersionList({ mcversion: '1.8.9' })
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
  console.log(`[RavenXD-Core] FORGE_OK ${forgeId}`)
}

async function start(gamePath, javaPath, username, version = '1.8.9') {
  if (!gamePath) throw new Error('Missing game directory.')
  if (!javaPath) throw new Error('Missing Java executable path.')
  const name = (username || 'RavenXD').trim() || 'RavenXD'

  console.log(`[RavenXD-Core] Launching ${version} as ${name}`)
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

  child.unref()
  console.log('[RavenXD-Core] LAUNCH_OK')
}

async function forgeLaunch(gamePath, javaPath, username) {
  const fs = await import('node:fs/promises')
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

try {
  if (command === 'install') await installMinecraft(args[0])
  else if (command === 'install-forge') await installForge18(args[0], args[1])
  else if (command === 'launch') await start(args[0], args[1], args[2], '1.8.9')
  else if (command === 'forge-launch') await forgeLaunch(args[0], args[1], args[2])
  else throw new Error('Usage: install | install-forge | launch | forge-launch')
} catch (error) {
  fail('Operation failed.', error)
}
