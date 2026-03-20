#!/usr/bin/env python3
"""
create_calibration_npz.py

Erstellt eine calibration .npz Datei aus Bilddaten (Ordner/Datei/TXT-Liste)
oder optional aus einem YOLO-style dataset.yaml.

Usage:
    python "C:\Dev\AI\Beachlor_Thesis_AI\Training_Scripts\npzConverter.py" --images "C:\Dev\Image_Capture_Python\dataset\images\angepasstes_seitenv_20260311_170850" --out "C:\Dev\Image_Capture_Python\dataset\images\angepasstes_seitenv_20260311_170850\calibration.npz" --imgsz 640 --count 3 --norm_to_1

    # optional weiterhin mit dataset.yaml
    python create_calibration_npz.py --dataset dataset.yaml --split val \
        --out calibration.npz --imgsz 640 --count 200 --seed 42 --norm_to_1

Output .npz enthält das Array unter dem Schlüssel "input" als shape (N,3,H,W) float32.

Änderung ggü. vorher:
- Preprocessing nutzt jetzt YOLO-typische Letterbox (Aspect Ratio erhalten, padding=114),
  statt hartem Resize auf imgsz x imgsz.
"""

import argparse
import sys
import yaml
import random
from pathlib import Path
import numpy as np
import cv2
from tqdm import tqdm

SUPPORTED_EXTS = ('.jpg', '.jpeg', '.png', '.bmp', '.tif', '.tiff')

def read_dataset_yaml(path: str) -> dict:
    with open(path, 'r') as f:
        return yaml.safe_load(f)

def resolve_image_list(entry: str, dataset_dir: Path) -> list[str]:
    """
    entry: path in dataset.yaml (dir or txt listing images)
    dataset_dir: directory containing dataset.yaml (for relative paths)
    returns: list of absolute image file paths
    """
    p = Path(entry)
    if not p.is_absolute():
        p = (dataset_dir / p).resolve()

    if p.is_file():
        ext = p.suffix.lower()
        if ext in SUPPORTED_EXTS:
            return [str(p)]
        # text file listing images
        imgs = []
        with open(p, 'r') as fh:
            for line in fh:
                line = line.strip()
                if not line:
                    continue
                ip = Path(line)
                if not ip.is_absolute():
                    ip = (p.parent / ip).resolve()
                if ip.exists():
                    imgs.append(str(ip))
        return imgs

    if p.is_dir():
        imgs = []
        for ext in SUPPORTED_EXTS:
            imgs.extend(sorted([str(x.resolve()) for x in p.glob(f'*{ext}')]))
        return imgs

    # fallback: try globbing relative to dataset_dir
    imgs = []
    base = dataset_dir
    if (base / entry).exists():
        return resolve_image_list(str(base / entry), dataset_dir)

    for ext in SUPPORTED_EXTS:
        imgs.extend(sorted([str(x.resolve()) for x in base.rglob(f'*{ext}')]))
    return imgs

def collect_images_from_dataset_yaml(yaml_path: str, split: str) -> list[str]:
    data = read_dataset_yaml(yaml_path)
    dataset_dir = Path(yaml_path).parent
    imgs = []

    if split == 'all':
        keys = [k for k in ('train', 'val', 'test') if k in data]
    else:
        if split not in data:
            raise ValueError(f"split '{split}' not found in dataset.yaml; available keys: {list(data.keys())}")
        keys = [split]

    for k in keys:
        entry = data.get(k)
        if entry:
            imgs.extend(resolve_image_list(entry, dataset_dir))

    if not imgs:
        # fallback: rare variants
        for candidate in ('path', 'images'):
            if data.get(candidate):
                imgs = resolve_image_list(data[candidate], dataset_dir)
                break

    # unique and keep order
    seen = set()
    imgs_unique = []
    for p in imgs:
        if p not in seen:
            seen.add(p)
            imgs_unique.append(p)
    return imgs_unique

def collect_images_from_input(images_entry: str) -> list[str]:
    """
    Resolve images directly from an image source:
    - image file
    - directory with images
    - txt file containing image paths
    """
    p = Path(images_entry)
    if not p.exists():
        raise FileNotFoundError(f"Image source not found: {images_entry}")

    base_dir = Path.cwd()
    imgs = resolve_image_list(images_entry, base_dir)

    # unique and keep order
    seen = set()
    imgs_unique = []
    for p in imgs:
        if p not in seen:
            seen.add(p)
            imgs_unique.append(p)
    return imgs_unique

def letterbox_bgr(
    img_bgr: np.ndarray,
    new_shape: int,
    color: tuple[int, int, int] = (114, 114, 114),
    auto: bool = False,
    stride: int = 32
) -> np.ndarray:
    """
    YOLO-typische Letterbox:
    - Aspect Ratio beibehalten
    - auf new_shape x new_shape in ein gepaddetes Canvas legen
    - Padding-Farbe wie YOLO üblich: 114

    auto=True und stride=32 würde Padding so wählen, dass es stride-align ist.
    Für Kalibrierung ist auto=False (exakt new_shape) meist am sinnvollsten.
    """
    h, w = img_bgr.shape[:2]
    if h == 0 or w == 0:
        raise ValueError("Empty image")

    # scale ratio (new / old)
    r = min(new_shape / w, new_shape / h)
    # compute unpadded new size
    new_unpad = (int(round(w * r)), int(round(h * r)))  # (w, h)

    # compute padding
    dw = new_shape - new_unpad[0]
    dh = new_shape - new_unpad[1]

    if auto:
        # make padding multiples of stride (like YOLO's auto mode)
        dw = dw % stride
        dh = dh % stride

    dw /= 2
    dh /= 2

    # resize
    if (w, h) != new_unpad:
        # interpolation: downscale -> AREA, upscale -> LINEAR (wie in vielen YOLO impl.)
        interp = cv2.INTER_AREA if r < 1.0 else cv2.INTER_LINEAR
        img_bgr = cv2.resize(img_bgr, new_unpad, interpolation=interp)

    # pad
    top = int(round(dh - 0.1))
    bottom = int(round(dh + 0.1))
    left = int(round(dw - 0.1))
    right = int(round(dw + 0.1))
    img_bgr = cv2.copyMakeBorder(img_bgr, top, bottom, left, right, cv2.BORDER_CONSTANT, value=color)

    # safety: enforce exact shape
    img_bgr = img_bgr[:new_shape, :new_shape]
    if img_bgr.shape[0] != new_shape or img_bgr.shape[1] != new_shape:
        # rare rounding corner case: force exact resize
        img_bgr = cv2.resize(img_bgr, (new_shape, new_shape), interpolation=cv2.INTER_LINEAR)

    return img_bgr

def load_and_preprocess(
    img_paths: list[str],
    imgsz: int,
    norm_to_1: bool = True,
    use_letterbox: bool = True,
    letterbox_auto: bool = False,
    letterbox_stride: int = 32,
    show_progress: bool = True
) -> np.ndarray:
    """
    Load images, YOLO-preprocess:
    - BGR read
    - force 3-channel BGR
    - letterbox to imgsz x imgsz (default) OR direct resize
    - BGR -> RGB
    - HWC -> CHW
    - float32 and normalize to [0,1] if requested
    Returns numpy array shape (N,3,H,W) float32.
    """
    out = []
    iterator = tqdm(img_paths, desc='Loading images') if show_progress else img_paths

    for p in iterator:
        img = cv2.imread(p, cv2.IMREAD_UNCHANGED)
        if img is None:
            print(f"Warning: could not read {p}, skipping.", file=sys.stderr)
            continue

        # Ensure 3-channel BGR
        if len(img.shape) == 2:
            img = cv2.cvtColor(img, cv2.COLOR_GRAY2BGR)
        elif img.shape[2] == 4:
            img = cv2.cvtColor(img, cv2.COLOR_BGRA2BGR)

        if use_letterbox:
            img = letterbox_bgr(img, new_shape=imgsz, auto=letterbox_auto, stride=letterbox_stride)
        else:
            img = cv2.resize(img, (imgsz, imgsz), interpolation=cv2.INTER_LINEAR)

        # BGR -> RGB
        img_rgb = img[:, :, ::-1]

        # HWC -> CHW
        img_chw = np.transpose(img_rgb, (2, 0, 1)).astype(np.float32)

        if norm_to_1:
            img_chw /= 255.0

        out.append(img_chw)

    if not out:
        raise RuntimeError("No images were loaded. Check paths and dataset.yaml content.")
    return np.stack(out, axis=0)

def main():
    parser = argparse.ArgumentParser(description="Create calibration .npz for STM32 quantization from images (YOLO letterbox)")
    parser.add_argument('--images', '-i', help='Image source: image file, folder, or txt file with image paths')
    parser.add_argument('--dataset', '-d', help='Optional path to dataset.yaml')
    parser.add_argument('--split', choices=['train', 'val', 'test', 'all'], default='val', help='Which split to use (default: val)')
    parser.add_argument('--out', '-o', default='calibration.npz', help='Output .npz filename (default: calibration.npz)')
    parser.add_argument('--imgsz', type=int, default=640, help='Target square size. Must match the model input size (default: 640)')
    parser.add_argument('--count', type=int, default=200, help='Number of calibration images to sample (default: 200)')
    parser.add_argument('--seed', type=int, default=42, help='Random seed (default: 42)')
    parser.add_argument('--shuffle', action='store_true', help='Shuffle available images before sampling')

    # Für YOLOv8 ist norm_to_1 praktisch immer korrekt (img/255.0)
    parser.add_argument('--norm_to_1', action='store_true', help='Normalize images to [0,1] floats (recommended)')

    # Letterbox-Optionen
    parser.add_argument('--no_letterbox', action='store_true', help='Disable letterbox (fallback to direct resize)')
    parser.add_argument('--letterbox_auto', action='store_true', help='Stride-aligned padding (YOLO auto mode). Default OFF for fixed size.')
    parser.add_argument('--letterbox_stride', type=int, default=32, help='Stride for letterbox auto mode (default: 32)')

    parser.add_argument('--verbose', action='store_true', help='Verbose output')
    args = parser.parse_args()

    if not args.images and not args.dataset:
        print("Error: provide either --images or --dataset.", file=sys.stderr)
        sys.exit(2)

    try:
        if args.images:
            imgs = collect_images_from_input(args.images)
        else:
            yaml_path = Path(args.dataset)
            if not yaml_path.exists():
                print(f"Error: dataset.yaml not found at {yaml_path}", file=sys.stderr)
                sys.exit(2)
            imgs = collect_images_from_dataset_yaml(str(yaml_path), args.split)
    except Exception as e:
        print(f"Error resolving input images: {e}", file=sys.stderr)
        sys.exit(2)

    if len(imgs) == 0:
        print("No images found for the requested split. Exiting.", file=sys.stderr)
        sys.exit(2)

    # Shuffle deterministisch
    if args.shuffle:
        random.Random(args.seed).shuffle(imgs)

    # sample up to count
    count = min(args.count, len(imgs))
    sampled = imgs[:count]

    if args.verbose:
        print(f"Found {len(imgs)} images total; sampling {count} images for calibration.")
        print("Sampled example files:")
        for p in sampled[:5]:
            print("  ", p)
        print(f"Letterbox: {'OFF' if args.no_letterbox else 'ON'} (auto={args.letterbox_auto}, stride={args.letterbox_stride})")
        print(f"Normalize to [0,1]: {'ON' if args.norm_to_1 else 'OFF'}")

    arr = load_and_preprocess(
        sampled,
        imgsz=args.imgsz,
        norm_to_1=args.norm_to_1,
        use_letterbox=(not args.no_letterbox),
        letterbox_auto=args.letterbox_auto,
        letterbox_stride=args.letterbox_stride,
        show_progress=not args.verbose
    )

    out_path = Path(args.out)
    if out_path.parent != Path(''):
        out_path.parent.mkdir(parents=True, exist_ok=True)

    np.savez(out_path, input=arr)
    print(f"Saved calibration file: {out_path} with shape {arr.shape} and dtype {arr.dtype}")

if __name__ == '__main__':
    main()