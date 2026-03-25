#!/usr/bin/env python3
"""
Automated import pipeline for Label Studio exports into local training datasets.

Supported workflows:
1) Pose from Label Studio YOLO export -> real_pose_dataset labels train/val
2) Segmentation from Label Studio COCO export -> real_dataset/labels/all

This script is intentionally standalone (stdlib only) so it can run in any Python env.
"""

from __future__ import annotations

import argparse
import json
import random
import re
import shutil
import sys
from dataclasses import dataclass
from datetime import datetime
from pathlib import Path
from typing import Dict, Iterable, List, Optional, Tuple


EXPECTED_SEG_CLASS_MAP = {
    "Base": 0,
    "Joint_1": 1,
    "Joint_2": 2,
    "Joint_3": 3,
    "Joint_4_Finger": 4,
}

DEFAULT_POSE_VAL_RATIO = 0.20
DEFAULT_SPLIT_SEED = 42


@dataclass
class PoseImportStats:
    imported_train: int = 0
    imported_val: int = 0
    not_found: int = 0
    matched_train_only: int = 0
    matched_val_only: int = 0
    matched_both_splits: int = 0
    split_stem_collisions_total: int = 0
    stale_removed_train: int = 0
    stale_removed_val: int = 0
    normalized_files: int = 0
    normalized_lines_ok: int = 0
    normalized_lines_fixed: int = 0
    normalized_lines_invalid: int = 0
    decimal_comma_lines_fixed: int = 0
    auto_split_enabled: bool = False
    split_seed: int = DEFAULT_SPLIT_SEED
    split_val_ratio: float = DEFAULT_POSE_VAL_RATIO
    split_target_train: int = 0
    split_target_val: int = 0
    split_labels_moved: int = 0
    split_images_moved: int = 0
    split_duplicate_labels_removed: int = 0
    split_duplicate_images_removed: int = 0
    split_missing_images: int = 0


@dataclass
class SegImportStats:
    images_seen: int = 0
    annotations_seen: int = 0
    categories_mapped: int = 0
    polygons_written: int = 0
    files_written: int = 0
    files_skipped_missing_local_image: int = 0
    stale_removed: int = 0


def _timestamp() -> str:
    """Liefert einen Dateisystem-sicheren Zeitstempel für Backup-Namen."""
    return datetime.now().strftime("%Y%m%d_%H%M%S")


def _read_json(path: Path) -> dict:
    """Lädt eine JSON-Datei als Python-Dictionary."""
    return json.loads(path.read_text(encoding="utf-8"))


def _backup_directory(src_dir: Path, backup_parent: Path, backup_prefix: str, dry_run: bool) -> Path:
    """Erstellt ein TXT-Backup eines Label-Ordners vor dem Überschreiben."""
    backup_dir = backup_parent / f"{backup_prefix}_{_timestamp()}"
    if dry_run:
        return backup_dir

    backup_dir.mkdir(parents=True, exist_ok=True)
    for p in src_dir.glob("*.txt"):
        shutil.copy2(p, backup_dir / p.name)
    return backup_dir


def _read_kpt_count_from_yaml(yaml_path: Path, fallback: int = 9) -> int:
    """Liest die erwartete Keypoint-Anzahl aus kpt_shape der Pose-YAML."""
    if not yaml_path.exists():
        return fallback

    text = yaml_path.read_text(encoding="utf-8", errors="replace")
    m = re.search(r"kpt_shape:\s*\[(\d+)\s*,\s*3\]", text)
    return int(m.group(1)) if m else fallback


def _collect_stems(img_dir: Path) -> Dict[str, bool]:
    """Sammelt Bild-Stems aus einem Ordner zur schnellen Split-Zuordnung."""
    stems: Dict[str, bool] = {}
    if not img_dir.exists():
        return stems
    for p in img_dir.iterdir():
        if p.is_file():
            stems[p.stem] = True
    return stems


def _collect_label_stems(lbl_dir: Path) -> set[str]:
    """Sammelt Label-Stems aus einem Label-Ordner."""
    stems: set[str] = set()
    if not lbl_dir.exists():
        return stems
    for p in lbl_dir.glob("*.txt"):
        stems.add(p.stem)
    return stems


def _find_image_candidates_by_stem(img_dir: Path, stem: str) -> List[Path]:
    """Findet alle Bilddateien mit identischem Stem über unterschiedliche Endungen."""
    if not img_dir.exists():
        return []
    return sorted([p for p in img_dir.glob(f"{stem}.*") if p.is_file()])


def _auto_split_pose_dataset(
    dataset_dir: Path,
    stems: set[str],
    val_ratio: float,
    split_seed: int,
    dry_run: bool,
) -> dict:
    """Verteilt importierte Pose-Beispiele automatisch auf train/val (inkl. Bilder + Labels)."""
    train_img_dir = dataset_dir / "images" / "train"
    val_img_dir = dataset_dir / "images" / "val"
    train_lbl_dir = dataset_dir / "labels" / "train"
    val_lbl_dir = dataset_dir / "labels" / "val"

    train_img_dir.mkdir(parents=True, exist_ok=True)
    val_img_dir.mkdir(parents=True, exist_ok=True)
    train_lbl_dir.mkdir(parents=True, exist_ok=True)
    val_lbl_dir.mkdir(parents=True, exist_ok=True)

    ratio = max(0.0, min(0.9, float(val_ratio)))
    ordered = sorted(stems)
    rng = random.Random(split_seed)
    rng.shuffle(ordered)

    if not ordered:
        return {
            "target_train": 0,
            "target_val": 0,
            "labels_moved": 0,
            "images_moved": 0,
            "duplicate_labels_removed": 0,
            "duplicate_images_removed": 0,
            "missing_images": 0,
        }

    val_count = int(round(len(ordered) * ratio))
    if len(ordered) > 1:
        val_count = max(1, min(len(ordered) - 1, val_count))
    else:
        val_count = 0

    val_stems = set(ordered[:val_count])

    stats = {
        "target_train": len(ordered) - len(val_stems),
        "target_val": len(val_stems),
        "labels_moved": 0,
        "images_moved": 0,
        "duplicate_labels_removed": 0,
        "duplicate_images_removed": 0,
        "missing_images": 0,
    }

    for stem in ordered:
        target_split = "val" if stem in val_stems else "train"
        src_lbl_dir = val_lbl_dir if target_split == "train" else train_lbl_dir
        dst_lbl_dir = train_lbl_dir if target_split == "train" else val_lbl_dir
        src_img_dir = val_img_dir if target_split == "train" else train_img_dir
        dst_img_dir = train_img_dir if target_split == "train" else val_img_dir

        src_lbl = src_lbl_dir / f"{stem}.txt"
        dst_lbl = dst_lbl_dir / f"{stem}.txt"

        if src_lbl.exists() and not dst_lbl.exists():
            stats["labels_moved"] += 1
            if not dry_run:
                shutil.move(str(src_lbl), str(dst_lbl))

        # Doppelte Labels in Nicht-Zielsplit entfernen.
        if src_lbl.exists() and dst_lbl.exists():
            stats["duplicate_labels_removed"] += 1
            if not dry_run:
                src_lbl.unlink()

        src_imgs = _find_image_candidates_by_stem(src_img_dir, stem)
        dst_imgs = _find_image_candidates_by_stem(dst_img_dir, stem)

        if not src_imgs and not dst_imgs:
            stats["missing_images"] += 1
            continue

        # Falls Zielsplit noch kein Bild hat, ein Bild aus Quellsplit verschieben.
        if not dst_imgs and src_imgs:
            img_to_move = src_imgs[0]
            stats["images_moved"] += 1
            if not dry_run:
                shutil.move(str(img_to_move), str(dst_img_dir / img_to_move.name))
            src_imgs = src_imgs[1:]

        # Restliche Nicht-Ziel-Bilder für denselben Stem entfernen (Kollisionen vermeiden).
        for extra in src_imgs:
            stats["duplicate_images_removed"] += 1
            if not dry_run:
                extra.unlink()

    return stats


def _detect_pose_labels_dir(export_dir: Path) -> Path:
    """Erkennt automatisch den Pose-Labelordner in einem Label-Studio-Export."""
    if not export_dir.exists():
        raise FileNotFoundError(f"Pose export folder not found: {export_dir}")

    candidates = [
        export_dir / "labels",
        export_dir / "obj_train_data",
        export_dir / "data" / "labels",
        export_dir,
    ]
    for cand in candidates:
        if cand.exists() and cand.is_dir() and any(cand.glob("*.txt")):
            return cand

    found_dirs = sorted({p.parent for p in export_dir.rglob("*.txt") if p.is_file()})
    if found_dirs:
        return found_dirs[0]

    raise FileNotFoundError(
        f"Pose export labels folder not found under {export_dir} (expected labels/*.txt)"
    )


def _recent_label_studio_project_dirs() -> List[Path]:
    """Liefert Downloads-Exportordner nach Änderungszeit sortiert (neueste zuerst)."""
    downloads = Path.home() / "Downloads"
    if not downloads.exists():
        return []

    dirs = [p for p in downloads.glob("project-*-at-*") if p.is_dir()]
    return sorted(dirs, key=lambda p: p.stat().st_mtime, reverse=True)


def _guess_default_pose_export_dir(fallback: Path) -> Path:
    """Wählt den neuesten plausiblen Pose-Exportordner als interaktiven Default."""
    for p in _recent_label_studio_project_dirs():
        try:
            _detect_pose_labels_dir(p)
            return p
        except Exception:
            continue
    return fallback


def _guess_default_seg_export_path(fallback: Path) -> Path:
    """Wählt den neuesten plausiblen Seg-COCO-Export als interaktiven Default."""
    for p in _recent_label_studio_project_dirs():
        try:
            _detect_coco_file(p)
            return p
        except Exception:
            continue
    return fallback


def _prompt_path(label: str, default: Path) -> Path:
    """Fragt einen Pfad ab und nutzt bei Enter den vorgeschlagenen Default."""
    raw = input(f"{label} [Enter={default}]: ").strip()
    return Path(raw) if raw else default


def _prompt_yes_no(label: str, default: bool = False) -> bool:
    """Standardisierte Ja/Nein-Abfrage für das interaktive Menü."""
    suffix = "Y/n" if default else "y/N"
    raw = input(f"{label} [{suffix}]: ").strip().lower()
    if not raw:
        return default
    return raw in {"y", "yes", "j", "ja", "1", "true"}


def _interactive_menu() -> argparse.Namespace | None:
    """Interaktiver Menüeinstieg, der CLI-Args als Namespace vorbereitet."""
    here = Path(__file__).resolve().parent
    default_pose_export = _guess_default_pose_export_dir(here / "label_studio_exports" / "pose")
    default_seg_export = _guess_default_seg_export_path(here / "label_studio_exports" / "seg")

    print("=" * 64)
    print("Label Studio Sync")
    print("=" * 64)
    print("[1] Pose importieren (YOLO Export)")
    print("[2] Segmente importieren (COCO Export)")
    print("[3] Beides importieren")
    print("[0] Abbrechen")
    print()

    choice = input("Auswahl (0/1/2/3): ").strip()
    if choice == "0":
        print("Abgebrochen.")
        return None
    if choice not in {"1", "2", "3"}:
        print("Ungueltige Auswahl.")
        return None

    dry_run = _prompt_yes_no("Dry-Run (nur pruefen, nichts schreiben)", default=False)

    if choice == "1":
        return argparse.Namespace(
            cmd="pose-yolo",
            dry_run=dry_run,
            no_auto_split=False,
            pose_val_ratio=DEFAULT_POSE_VAL_RATIO,
            split_seed=DEFAULT_SPLIT_SEED,
            export_dir=_prompt_path("Pose Export-Ordner", default_pose_export),
            dataset_dir=_prompt_path("Pose Ziel-Dataset", here / "real_pose_dataset"),
            pose_yaml=_prompt_path("Pose YAML", here / "real_pose_dataset.yaml"),
        )

    if choice == "2":
        return argparse.Namespace(
            cmd="seg-coco",
            dry_run=dry_run,
            no_auto_split=False,
            pose_val_ratio=DEFAULT_POSE_VAL_RATIO,
            split_seed=DEFAULT_SPLIT_SEED,
            coco=_prompt_path("COCO Datei oder Export-Ordner", default_seg_export),
            images_dir=_prompt_path("Segment-Bilder Ordner", here / "real_dataset" / "images" / "all"),
            labels_dir=_prompt_path("Segment-Labels Zielordner", here / "real_dataset" / "labels" / "all"),
        )

    return argparse.Namespace(
        cmd="all",
        dry_run=dry_run,
        no_auto_split=False,
        pose_val_ratio=DEFAULT_POSE_VAL_RATIO,
        split_seed=DEFAULT_SPLIT_SEED,
        pose_export_dir=_prompt_path("Pose Export-Ordner", default_pose_export),
        seg_coco=_prompt_path("COCO Datei oder Export-Ordner", default_seg_export),
        pose_dataset_dir=_prompt_path("Pose Ziel-Dataset", here / "real_pose_dataset"),
        pose_yaml=_prompt_path("Pose YAML", here / "real_pose_dataset.yaml"),
        seg_images_dir=_prompt_path("Segment-Bilder Ordner", here / "real_dataset" / "images" / "all"),
        seg_labels_dir=_prompt_path("Segment-Labels Zielordner", here / "real_dataset" / "labels" / "all"),
    )


def import_pose_yolo(
    export_dir: Path,
    dataset_dir: Path,
    pose_yaml: Path,
    auto_split: bool = True,
    val_ratio: float = DEFAULT_POSE_VAL_RATIO,
    split_seed: int = DEFAULT_SPLIT_SEED,
    dry_run: bool = False,
) -> Tuple[PoseImportStats, Path, int]:
    """Importiert Pose-YOLO-Labels aus Label Studio in train/val inkl. Normalisierung."""
    labels_dir = _detect_pose_labels_dir(export_dir)

    train_img_dir = dataset_dir / "images" / "train"
    val_img_dir = dataset_dir / "images" / "val"
    train_lbl_dir = dataset_dir / "labels" / "train"
    val_lbl_dir = dataset_dir / "labels" / "val"

    for required in [train_img_dir, val_img_dir, train_lbl_dir, val_lbl_dir]:
        if not required.exists():
            raise FileNotFoundError(f"Required dataset folder not found: {required}")

    expected_kpt = _read_kpt_count_from_yaml(pose_yaml)
    expected_tokens = 5 + (3 * expected_kpt)

    train_stems = _collect_stems(train_img_dir)
    val_stems = _collect_stems(val_img_dir)
    stem_collisions = set(train_stems).intersection(val_stems)

    backup_root = dataset_dir / f"labels_backup_before_pose_sync_{_timestamp()}"
    backup_train = backup_root / "train"
    backup_val = backup_root / "val"
    if not dry_run:
        backup_train.mkdir(parents=True, exist_ok=True)
        backup_val.mkdir(parents=True, exist_ok=True)
        for p in train_lbl_dir.glob("*.txt"):
            shutil.copy2(p, backup_train / p.name)
        for p in val_lbl_dir.glob("*.txt"):
            shutil.copy2(p, backup_val / p.name)

    stats = PoseImportStats()
    stats.auto_split_enabled = auto_split
    stats.split_seed = split_seed
    stats.split_val_ratio = val_ratio
    stats.split_stem_collisions_total = len(stem_collisions)
    touched_train: set[str] = set()
    touched_val: set[str] = set()

    # 1) Import labels from Label Studio naming pattern <hash>__<image_stem>.txt
    for src in labels_dir.glob("*.txt"):
        base = src.stem
        stem = base.split("__", 1)[1] if "__" in base else base
        target_name = f"{stem}.txt"

        in_train = stem in train_stems
        in_val = stem in val_stems

        if in_train and in_val:
            # Behalte existierendes Verhalten: bei Namenskollision zuerst train.
            stats.matched_both_splits += 1
            if not dry_run:
                dst = train_lbl_dir / target_name
                text = src.read_text(encoding="utf-8", errors="replace")
                dst.write_text(text.replace(",", "."), encoding="utf-8")
            touched_train.add(target_name)
            stats.imported_train += 1
        elif in_train:
            stats.matched_train_only += 1
            if not dry_run:
                dst = train_lbl_dir / target_name
                text = src.read_text(encoding="utf-8", errors="replace")
                dst.write_text(text.replace(",", "."), encoding="utf-8")
            touched_train.add(target_name)
            stats.imported_train += 1
        elif in_val:
            stats.matched_val_only += 1
            if not dry_run:
                dst = val_lbl_dir / target_name
                text = src.read_text(encoding="utf-8", errors="replace")
                dst.write_text(text.replace(",", "."), encoding="utf-8")
            touched_val.add(target_name)
            stats.imported_val += 1
        else:
            stats.not_found += 1

    # 1b) Stale labels entfernen, damit nur aktueller Export im Training bleibt.
    for p in train_lbl_dir.glob("*.txt"):
        if p.name not in touched_train:
            stats.stale_removed_train += 1
            if not dry_run:
                p.unlink()
    for p in val_lbl_dir.glob("*.txt"):
        if p.name not in touched_val:
            stats.stale_removed_val += 1
            if not dry_run:
                p.unlink()

    if auto_split:
        imported_stems = {Path(n).stem for n in touched_train}.union({Path(n).stem for n in touched_val})

        # Fallback: falls kein touched-Set verfügbar ist, aktuelle Label-Stems nehmen.
        if not imported_stems:
            imported_stems = _collect_label_stems(train_lbl_dir).union(_collect_label_stems(val_lbl_dir))

        split_stats = _auto_split_pose_dataset(
            dataset_dir=dataset_dir,
            stems=imported_stems,
            val_ratio=val_ratio,
            split_seed=split_seed,
            dry_run=dry_run,
        )
        stats.split_target_train = split_stats["target_train"]
        stats.split_target_val = split_stats["target_val"]
        stats.split_labels_moved = split_stats["labels_moved"]
        stats.split_images_moved = split_stats["images_moved"]
        stats.split_duplicate_labels_removed = split_stats["duplicate_labels_removed"]
        stats.split_duplicate_images_removed = split_stats["duplicate_images_removed"]
        stats.split_missing_images = split_stats["missing_images"]

    # 2) Normalize token counts to expected kpt_shape width
    all_label_files = list(train_lbl_dir.glob("*.txt")) + list(val_lbl_dir.glob("*.txt"))
    for lbl in all_label_files:
        text = lbl.read_text(encoding="utf-8", errors="replace")
        lines = [ln.strip() for ln in text.splitlines() if ln.strip()]
        if not lines:
            continue

        changed = False
        new_lines: List[str] = []
        for ln in lines:
            if "," in ln:
                ln = ln.replace(",", ".")
                stats.decimal_comma_lines_fixed += 1
                changed = True

            parts = ln.split()
            token_count = len(parts)

            if token_count == expected_tokens:
                stats.normalized_lines_ok += 1
                new_lines.append(ln)
                continue

            # Padding missing keypoints with invisible 0 0 0 triplets
            if token_count < expected_tokens and (expected_tokens - token_count) % 3 == 0:
                missing_triplets = (expected_tokens - token_count) // 3
                pad = " ".join(["0 0 0"] * missing_triplets)
                new_lines.append(f"{ln} {pad}")
                stats.normalized_lines_fixed += 1
                changed = True
                continue

            stats.normalized_lines_invalid += 1
            new_lines.append(ln)

        if changed:
            stats.normalized_files += 1
            if not dry_run:
                lbl.write_text("\n".join(new_lines) + "\n", encoding="utf-8")

    # 3) Clear stale ultralytics caches
    for cache in [train_lbl_dir / "train.cache", val_lbl_dir / "val.cache"]:
        if cache.exists() and not dry_run:
            cache.unlink()

    return stats, backup_root, expected_tokens


def _detect_coco_file(path_or_dir: Path) -> Path:
    """Findet die zu nutzende COCO-JSON-Datei in Datei- oder Exportordnerform."""
    if path_or_dir.suffix.lower() == ".json":
        if path_or_dir.exists() and path_or_dir.is_file():
            return path_or_dir
        raise FileNotFoundError(f"COCO json file not found: {path_or_dir}")

    if path_or_dir.is_file():
        return path_or_dir

    if not path_or_dir.exists():
        raise FileNotFoundError(f"COCO path not found: {path_or_dir}")

    candidates = [
        path_or_dir / "result_coco.json",
        path_or_dir / "result.json",
    ]
    for c in candidates:
        if c.exists():
            return c

    raise FileNotFoundError(f"No COCO json found under: {path_or_dir}")


def import_seg_coco(
    coco_path_or_dir: Path,
    images_dir: Path,
    labels_dir: Path,
    dry_run: bool = False,
) -> Tuple[SegImportStats, Path, Path]:
    """Importiert COCO-Segmentierung und schreibt YOLO-Polygonlabels für lokale Bilder."""
    coco_path = _detect_coco_file(coco_path_or_dir)
    data = _read_json(coco_path)

    images = data.get("images", [])
    annotations = data.get("annotations", [])
    categories = data.get("categories", [])

    if not images or not annotations:
        raise ValueError(
            f"COCO file appears empty for training (images={len(images)}, annotations={len(annotations)}): {coco_path}"
        )

    if not images_dir.exists():
        raise FileNotFoundError(f"Segmentation image directory not found: {images_dir}")
    labels_dir.mkdir(parents=True, exist_ok=True)

    backup_dir = _backup_directory(
        src_dir=labels_dir,
        backup_parent=labels_dir.parent,
        backup_prefix="labels_backup_before_seg_sync",
        dry_run=dry_run,
    )

    stats = SegImportStats(
        images_seen=len(images),
        annotations_seen=len(annotations),
        categories_mapped=0,
    )

    # Stale Seg-Labels entfernen, damit nur aktueller Export erhalten bleibt.
    for p in labels_dir.glob("*.txt"):
        stats.stale_removed += 1
        if not dry_run:
            p.unlink()

    # category_id -> expected training class_id
    cat_to_class: Dict[int, int] = {}
    for c in categories:
        name = c.get("name")
        cid = c.get("id")
        if name in EXPECTED_SEG_CLASS_MAP and isinstance(cid, int):
            cat_to_class[cid] = EXPECTED_SEG_CLASS_MAP[name]

    image_by_id: Dict[int, dict] = {}
    for img in images:
        if isinstance(img.get("id"), int):
            image_by_id[img["id"]] = img

    lines_by_filename: Dict[str, List[str]] = {}
    stats.categories_mapped = len(cat_to_class)

    for ann in annotations:
        image_id = ann.get("image_id")
        category_id = ann.get("category_id")
        segmentation = ann.get("segmentation")

        if image_id not in image_by_id or category_id not in cat_to_class:
            continue
        if not isinstance(segmentation, list):
            # RLE or unsupported segmentation form
            continue

        img = image_by_id[image_id]
        filename = Path(str(img.get("file_name", ""))).name
        width = float(img.get("width", 0) or 0)
        height = float(img.get("height", 0) or 0)
        if width <= 0 or height <= 0 or not filename:
            continue

        cls = cat_to_class[category_id]

        for poly in segmentation:
            if not isinstance(poly, list) or len(poly) < 6:
                continue
            pts: List[str] = []
            for i in range(0, len(poly) - 1, 2):
                x = max(0.0, min(1.0, float(poly[i]) / width))
                y = max(0.0, min(1.0, float(poly[i + 1]) / height))
                pts.append(f"{x:.6f} {y:.6f}")

            if len(pts) >= 3:
                lines_by_filename.setdefault(filename, []).append(f"{cls} {' '.join(pts)}")
                stats.polygons_written += 1

    # Write only for images mentioned in COCO and present locally.
    for filename, lines in lines_by_filename.items():
        local_image = images_dir / filename
        if not local_image.exists():
            stats.files_skipped_missing_local_image += 1
            continue

        out = labels_dir / f"{local_image.stem}.txt"
        stats.files_written += 1
        if not dry_run:
            out.write_text("\n".join(lines) + "\n", encoding="utf-8")

    return stats, backup_dir, coco_path


def _print_pose_summary(stats: PoseImportStats, backup_dir: Path, expected_tokens: int) -> None:
    """Gibt eine kompakte Importzusammenfassung für Pose aus."""
    print("POSE import completed")
    print(f"  backup: {backup_dir}")
    print(f"  expected tokens/label line: {expected_tokens}")
    print(f"  imported train: {stats.imported_train}")
    print(f"  imported val: {stats.imported_val}")
    print(f"  not found in split: {stats.not_found}")
    print(f"  matched train-only stems: {stats.matched_train_only}")
    print(f"  matched val-only stems: {stats.matched_val_only}")
    print(f"  matched in BOTH splits (assigned to train): {stats.matched_both_splits}")
    print(f"  total train/val stem collisions in dataset: {stats.split_stem_collisions_total}")
    print(f"  stale removed train: {stats.stale_removed_train}")
    print(f"  stale removed val: {stats.stale_removed_val}")
    print(f"  normalized files: {stats.normalized_files}")
    print(f"  normalized lines already ok: {stats.normalized_lines_ok}")
    print(f"  normalized lines fixed: {stats.normalized_lines_fixed}")
    print(f"  normalized lines invalid: {stats.normalized_lines_invalid}")
    print(f"  decimal-comma lines fixed: {stats.decimal_comma_lines_fixed}")

    if stats.auto_split_enabled:
        print("  auto split: enabled")
        print(f"  split seed: {stats.split_seed}")
        print(f"  split ratio val: {stats.split_val_ratio:.2f}")
        print(f"  split target train: {stats.split_target_train}")
        print(f"  split target val: {stats.split_target_val}")
        print(f"  split labels moved: {stats.split_labels_moved}")
        print(f"  split images moved: {stats.split_images_moved}")
        print(f"  split duplicate labels removed: {stats.split_duplicate_labels_removed}")
        print(f"  split duplicate images removed: {stats.split_duplicate_images_removed}")
        print(f"  split missing images: {stats.split_missing_images}")

    if stats.split_stem_collisions_total > 0:
        print("  WARNUNG: Gleiche Bild-Stems existieren in train und val.")
        print("           Dadurch kann der Import val-Labels nach train ziehen.")


def _print_seg_summary(stats: SegImportStats, backup_dir: Path, coco_path: Path) -> None:
    """Gibt eine kompakte Importzusammenfassung für Segmentierung aus."""
    print("SEG import completed")
    print(f"  coco source: {coco_path}")
    print(f"  backup: {backup_dir}")
    print(f"  images in coco: {stats.images_seen}")
    print(f"  annotations in coco: {stats.annotations_seen}")
    print(f"  mapped categories: {stats.categories_mapped}")
    print(f"  polygons written: {stats.polygons_written}")
    print(f"  label files written: {stats.files_written}")
    print(f"  skipped (local image missing): {stats.files_skipped_missing_local_image}")
    print(f"  stale labels removed: {stats.stale_removed}")


def build_parser() -> argparse.ArgumentParser:
    """Definiert das CLI mit Subcommands pose-yolo, seg-coco und all."""
    here = Path(__file__).resolve().parent

    parser = argparse.ArgumentParser(description="Sync Label Studio exports into local YOLO training datasets")
    parser.add_argument("--dry-run", action="store_true", help="Print actions without writing files")
    parser.add_argument(
        "--no-auto-split",
        action="store_true",
        help="Disable automatic 80/20 split after pose import",
    )
    parser.add_argument(
        "--pose-val-ratio",
        type=float,
        default=DEFAULT_POSE_VAL_RATIO,
        help="Validation ratio for automatic pose split (default: 0.20)",
    )
    parser.add_argument(
        "--split-seed",
        type=int,
        default=DEFAULT_SPLIT_SEED,
        help="Random seed for deterministic pose split",
    )

    sub = parser.add_subparsers(dest="cmd")

    pose = sub.add_parser("pose-yolo", help="Import Label Studio YOLO pose export")
    pose.add_argument("--export-dir", type=Path, required=True, help="Label Studio YOLO export directory")
    pose.add_argument(
        "--dataset-dir",
        type=Path,
        default=here / "real_pose_dataset",
        help="Target pose dataset root (images/labels train/val)",
    )
    pose.add_argument(
        "--pose-yaml",
        type=Path,
        default=here / "real_pose_dataset.yaml",
        help="Pose YAML with kpt_shape",
    )

    seg = sub.add_parser("seg-coco", help="Import Label Studio COCO segmentation export")
    seg.add_argument(
        "--coco",
        type=Path,
        required=True,
        help="COCO json file OR export directory containing result_coco.json/result.json",
    )
    seg.add_argument(
        "--images-dir",
        type=Path,
        default=here / "real_dataset" / "images" / "all",
        help="Local segmentation image directory",
    )
    seg.add_argument(
        "--labels-dir",
        type=Path,
        default=here / "real_dataset" / "labels" / "all",
        help="Target segmentation label directory",
    )

    both = sub.add_parser("all", help="Run both pose-yolo and seg-coco imports")
    both.add_argument("--pose-export-dir", type=Path, required=True, help="Label Studio YOLO pose export directory")
    both.add_argument(
        "--seg-coco",
        type=Path,
        required=True,
        help="COCO json file OR export directory for segmentation",
    )
    both.add_argument(
        "--pose-dataset-dir",
        type=Path,
        default=here / "real_pose_dataset",
        help="Target pose dataset root",
    )
    both.add_argument(
        "--pose-yaml",
        type=Path,
        default=here / "real_pose_dataset.yaml",
        help="Pose YAML with kpt_shape",
    )
    both.add_argument(
        "--seg-images-dir",
        type=Path,
        default=here / "real_dataset" / "images" / "all",
        help="Local segmentation image directory",
    )
    both.add_argument(
        "--seg-labels-dir",
        type=Path,
        default=here / "real_dataset" / "labels" / "all",
        help="Target segmentation label directory",
    )

    return parser


def main() -> None:
    """Programmstart: nutzt interaktives Menü oder CLI und führt den gewünschten Import aus."""
    if len(sys.argv) == 1:
        args = _interactive_menu()
        if args is None:
            return
    else:
        parser = build_parser()
        args = parser.parse_args()

    if not getattr(args, "cmd", None):
        parser = build_parser()
        parser.print_help()
        print("\nBeispiele:")
        print("  python Training_Scripts/label_studio_sync.py pose-yolo --export-dir ./label_studio_exports/pose")
        print("  python Training_Scripts/label_studio_sync.py seg-coco --coco ./label_studio_exports/seg/result_coco.json")
        print("  python Training_Scripts/label_studio_sync.py all --pose-export-dir ./label_studio_exports/pose --seg-coco ./label_studio_exports/seg")
        return

    try:
        if args.cmd == "pose-yolo":
            stats, backup_dir, expected_tokens = import_pose_yolo(
                export_dir=args.export_dir,
                dataset_dir=args.dataset_dir,
                pose_yaml=args.pose_yaml,
                auto_split=not args.no_auto_split,
                val_ratio=args.pose_val_ratio,
                split_seed=args.split_seed,
                dry_run=args.dry_run,
            )
            _print_pose_summary(stats, backup_dir, expected_tokens)
            return

        if args.cmd == "seg-coco":
            stats, backup_dir, coco_path = import_seg_coco(
                coco_path_or_dir=args.coco,
                images_dir=args.images_dir,
                labels_dir=args.labels_dir,
                dry_run=args.dry_run,
            )
            _print_seg_summary(stats, backup_dir, coco_path)
            return

        if args.cmd == "all":
            pose_stats, pose_backup, expected_tokens = import_pose_yolo(
                export_dir=args.pose_export_dir,
                dataset_dir=args.pose_dataset_dir,
                pose_yaml=args.pose_yaml,
                auto_split=not args.no_auto_split,
                val_ratio=args.pose_val_ratio,
                split_seed=args.split_seed,
                dry_run=args.dry_run,
            )
            _print_pose_summary(pose_stats, pose_backup, expected_tokens)

            seg_stats, seg_backup, coco_path = import_seg_coco(
                coco_path_or_dir=args.seg_coco,
                images_dir=args.seg_images_dir,
                labels_dir=args.seg_labels_dir,
                dry_run=args.dry_run,
            )
            _print_seg_summary(seg_stats, seg_backup, coco_path)
            return
    except FileNotFoundError as exc:
        print(f"FEHLER: {exc}")
        print("Hinweis: Bitte den korrekten Label-Studio Export-Ordner aus Downloads angeben,")
        print("z. B. C:/Users/<user>/Downloads/project-<id>-at-<timestamp>")
        return
    except Exception as exc:
        print(f"FEHLER: {exc}")
        return


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\nAbgebrochen.")
