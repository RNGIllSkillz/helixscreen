# Upgrading HelixScreen

This guide helps you upgrade HelixScreen to a newer version.

---

## Quick Upgrade

The easiest way to upgrade is using the install script:

```bash
# MainsailOS (Pi)
curl -sSL https://raw.githubusercontent.com/prestonbrown/helixscreen/main/scripts/install.sh | bash -s -- --update

# Adventurer 5M
curl -sSL https://raw.githubusercontent.com/prestonbrown/helixscreen/main/scripts/install.sh | sh -s -- --update
```

This preserves your settings and updates to the latest version.

---

## If the Setup Wizard Keeps Appearing

After upgrading, if HelixScreen keeps showing the setup wizard on every boot, your configuration file format may have changed in a way that's incompatible with the new version.

### Quick Fix

The easiest solution is to delete your config file and let the wizard create a new one:

**MainsailOS (Pi):**
```bash
sudo rm /opt/helixscreen/helixconfig.json
sudo systemctl restart helixscreen
```

**Adventurer 5M (Forge-X):**
```bash
rm /opt/helixscreen/helixconfig.json
/etc/init.d/S90helixscreen restart
```

**Adventurer 5M (Klipper Mod):**
```bash
rm /root/printer_software/helixscreen/helixconfig.json
/etc/init.d/S80helixscreen restart
```

After restarting, the wizard will guide you through setup again. Your printer settings (Klipper, Moonraker) are not affected - only HelixScreen's display preferences need to be reconfigured.

### Alternative: Factory Reset from UI

If you can access the settings panel before the wizard appears:

1. Navigate to **Settings** (gear icon in sidebar)
2. Scroll down to **Factory Reset**
3. Tap **Factory Reset** and confirm

This clears all HelixScreen settings and restarts the wizard.

---

## What Settings Are Affected

When you reset, you'll need to reconfigure:

- WiFi connection (if not using Ethernet)
- Moonraker connection (usually auto-detected)
- Display preferences (brightness, theme, sleep timeout)
- Sound settings
- Safety preferences (E-Stop confirmation)

Your Klipper configuration, Moonraker settings, print history, and G-code files are **not affected** - they're stored separately.

---

## Upgrade to Specific Version

To install a specific version instead of the latest:

```bash
curl -sSL https://raw.githubusercontent.com/prestonbrown/helixscreen/main/scripts/install.sh | bash -s -- --update --version v1.2.0
```

---

## Checking Your Version

**On the touchscreen:** Settings → scroll down → Version row shows current version

**Via SSH:**
```bash
/opt/helixscreen/bin/helix-screen --help | head -1
```

---

## Getting Help

If you encounter issues after upgrading:

1. Check [TROUBLESHOOTING.md](TROUBLESHOOTING.md) for common problems
2. View logs for error messages:
   - **Pi:** `sudo journalctl -u helixscreen -n 50`
   - **AD5M:** `cat /tmp/helixscreen.log | tail -50`
3. Open an issue on [GitHub](https://github.com/prestonbrown/helixscreen/issues) with your version and any error messages

---

*Back to: [Installation Guide](INSTALL.md) | [User Guide](USER_GUIDE.md)*
