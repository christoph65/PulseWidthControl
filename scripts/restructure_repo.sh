#!/usr/bin/env bash
# scripts/restructure_repo.sh
# Verschiebt / benennt vorhandene Top-Level-Ordner in die neue Repo-Struktur.
# Usage:
#   ./scripts/restructure_repo.sh        # führt aus (verwendet git mv wenn möglich)
#   ./scripts/restructure_repo.sh --dry-run
#   ./scripts/restructure_repo.sh --commit
#   ./scripts/restructure_repo.sh --force
#
# Optionen:
#  --dry-run   : Zeigt, was gemacht würde, führt nichts aus.
#  --commit    : Führt nach den git mv ein git commit -m "Reorganize repo" aus (nur bei git repo).
#  --force     : Überschreibt vorhandene Ziele (VORSICHT).
#
set -euo pipefail

DRY_RUN=false
COMMIT=false
FORCE=false

while [[ $# -gt 0 ]]; do
  case "$1" in
    --dry-run|-n) DRY_RUN=true; shift ;;
    --commit|-c)  COMMIT=true; shift ;;
    --force|-f)   FORCE=true; shift ;;
    -h|--help) echo "Usage: $0 [--dry-run] [--commit] [--force]"; exit 0 ;;
    *) echo "Unknown option: $1"; echo "Usage: $0 [--dry-run] [--commit] [--force]"; exit 1 ;;
  esac
done

# Prüfe, ob wir in einem Git-Repository sind
GIT_REPO=false
if [ -d ".git" ]; then
  GIT_REPO=true
fi

# Mapping: "Quell-Ordner::Ziel-Pfad"
mappings=(
  "Boot/ADC_Test::experiments/ADC_Test"
)

echo "Restructure script running..."
echo "Dry-run: $DRY_RUN"
echo "Commit: $COMMIT"
echo "Force: $FORCE"
echo "Git repo: $GIT_REPO"
echo

do_move() {
  local src="$1"
  local dest="$2"

  if [ ! -e "$src" ]; then
    echo "SKIP: Quellpfad nicht gefunden: '$src'"
    return 0
  fi

  if [ -e "$dest" ]; then
    if [ "$FORCE" = true ]; then
      if [ "$DRY_RUN" = true ]; then
        echo "DRY-RUN: Ziel existiert, würde entfernt: '$dest'"
      else
        echo "Ziel existiert - wird entfernt (force): $dest"
        rm -rf "$dest"
      fi
    else
      echo "FEHLER: Ziel existiert bereits: '$dest'  -> Nutze --force um zu überschreiben."
      exit 1
    fi
  fi

  mkdir -p "$(dirname "$dest")"

  if [ "$DRY_RUN" = true ]; then
    if [ "$GIT_REPO" = true ]; then
      echo "DRY-RUN: git mv -- '$src' -> '$dest'"
    else
      echo "DRY-RUN: mv -- '$src' -> '$dest'"
    fi
  else
    if [ "$GIT_REPO" = true ]; then
      echo "GIT-MOVE: '$src' -> '$dest'"
      git mv -- "$src" "$dest"
    else
      echo "MOVE: '$src' -> '$dest'"
      mv -- "$src" "$dest"
      # wenn mv erzeugt Fehler, script beendet (set -e)
    fi
  fi
}

for pair in "${mappings[@]}"; do
  src="${pair%%::*}"
  dest="${pair##*::}"
  do_move "$src" "$dest"
done

# Optional: .code-workspace und .vscode anlegen, falls noch nicht vorhanden
if [ ! -e "Boot.code-workspace" ]; then
  if [ "$DRY_RUN" = true ]; then
    echo "DRY-RUN: Würde 'Boot.code-workspace' erzeugen."
  else
    echo "Erzeuge minimale 'Boot.code-workspace' ..."
    cat > Boot.code-workspace <<'EOW'
{
  "folders": [
    { "path": "." },
    { "path": "projects/Controler" },
    { "path": "projects/GyroMotorControler" },
    { "path": "experiments/ADC_Test" }
  ],
  "settings": {}
}
EOW
    if [ "$GIT_REPO" = true ]; then git add Boot.code-workspace; fi
  fi
else
  echo "Boot.code-workspace existiert bereits -> übersprungen."
fi

# Option: commit changes
if [ "$COMMIT" = true ]; then
  if [ "$GIT_REPO" = true ]; then
    if [ "$DRY_RUN" = true ]; then
      echo "DRY-RUN: Würde committen: 'Reorganize repo: move projects and experiments'"
    else
      echo "git commit -m 'Reorganize repo: move projects and experiments' (inkl. Änderungen)"
      # Füge ggf. neue Dateien hinzu (z. B. Boot.code-workspace)
      git add -A
      git commit -m "Reorganize repo: move projects and experiments"
    fi
  else
    echo "WARNUNG: --commit angefordert, aber dies ist kein Git-Repository (.git fehlt)."
  fi
fi

echo
echo "Fertig. Nächste Schritte:"
echo "- Prüfe Include-Pfade in Makefiles / Projektdateien."
echo "- Öffne das Workspace: code Boot.code-workspace"
echo "- Passe ggf. Makefiles in projects/* an."