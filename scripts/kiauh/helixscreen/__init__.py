# SPDX-License-Identifier: GPL-3.0-or-later
from pathlib import Path

MODULE_PATH = Path(__file__).resolve().parent
HELIXSCREEN_REPO = "https://github.com/prestonbrown/helixscreen"
HELIXSCREEN_DIR = Path.home().joinpath("helixscreen")
HELIXSCREEN_SERVICE_NAME = "helixscreen"
HELIXSCREEN_INSTALLER_URL = (
    "https://raw.githubusercontent.com/prestonbrown/helixscreen/main/scripts/install.sh"
)
