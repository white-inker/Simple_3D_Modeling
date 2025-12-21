from pathlib import Path
import shutil


def copy_dir_contents(src_dir: Path, dst_dir: Path) -> None:
    """Copy contents of src_dir into dst_dir (merge/overwrite)."""
    if not src_dir.exists() or not src_dir.is_dir():
        print(f"[WARN] Source not found: {src_dir}")
        return

    dst_dir.mkdir(parents=True, exist_ok=True)

    for item in src_dir.iterdir():
        src = item
        dst = dst_dir / item.name

        if item.is_dir():
            shutil.copytree(src, dst, dirs_exist_ok=True)  # Python 3.8+
        else:
            shutil.copy2(src, dst)

    print(f"[OK] Copied: {src_dir} -> {dst_dir}")


def main() -> None:
    # ===== 1) Project root (hardcoded relative to this script) =====
    # Assumption: this script is at <project_root>/scripts/copy_resources.py
    PROJECT_ROOT = Path(__file__).resolve().parent

    # ===== 2) Hardcoded target output directory (EDIT THIS) =====
    # Example for CLion default build dir:
    TARGET_DIR = PROJECT_ROOT / "build/default"
    # If you want a fixed absolute path, do this instead:
    # TARGET_DIR = Path(r"D:\myCode\C++\Simple_3D_Modeling\build\default\bin")

    # ===== 3) Copy =====
    assets_src = PROJECT_ROOT / "assets"
    config_src = PROJECT_ROOT / "config"

    TARGET_DIR.mkdir(parents=True, exist_ok=True)

    copy_dir_contents(assets_src, TARGET_DIR / "assets")
    copy_dir_contents(config_src, TARGET_DIR / "config")


if __name__ == "__main__":
    main()