#!/usr/bin/env python3
"""
Convert YOLO segmentation/pose labels into Label Studio tasks and optionally
upload them to a Label Studio project via API.

Examples:
  python label_studio_converter.py seg \
    --images-dir .\\real_dataset\\images\\all \
    --labels-dir .\\real_dataset\\labels\\all \
    --image-url-prefix http://localhost:3000/real_dataset/images/all \
    --output .\\label_studio_import\\tasks_seg_all.json

  python label_studio_converter.py pose \
    --images-dir .\\real_pose_dataset\\images\\train \
    --labels-dir .\\real_pose_dataset\\labels\\train \
    --image-url-prefix http://localhost:3000/real_pose_dataset/images/train \
    --output .\\label_studio_import\\tasks_pose_train.json \
    --from-name kp

  python label_studio_converter.py prepare-all \
    --root . \
    --image-host http://localhost:3000
"""

from __future__ import annotations

import argparse
import json
import os
import sys
import urllib.error
import urllib.request
from pathlib import Path
from typing import Any, Dict, List, Optional, Sequence, Tuple

import numpy as np

SEG_CLASSES = {
    0: "Base",
    1: "Joint_1",
    2: "Joint_2",
    3: "Joint_3",
    4: "Joint_4_Finger",
}

POSE_KEYPOINT_LABELS = ["Base", "Joint_1", "Joint_2", "Joint_3", "Joint_4_Finger", "TCP"]

IMAGE_EXTS = {".png", ".jpg", ".jpeg", ".bmp", ".webp"}


def _as_posix(path: str) -> str:
    return path.replace("\\", "/")


def _join_url(prefix: str, filename: str) -> str:
    return prefix.rstrip("/") + "/" + filename


def _list_images(images_dir: Path) -> List[Path]:
    images = [p for p in images_dir.iterdir() if p.is_file() and p.suffix.lower() in IMAGE_EXTS]
    return sorted(images, key=lambda p: p.name)


def _read_label_lines(label_path: Path) -> List[str]:
    if not label_path.exists():
        return []
    text = label_path.read_text(encoding="utf-8").strip()
    if not text:
        return []
    return [ln.strip() for ln in text.splitlines() if ln.strip()]


def _safe_float(text: str) -> Optional[float]:
    try:
        return float(text)
    except ValueError:
        return None


def _clip_0_100(x: float) -> float:
    return max(0.0, min(100.0, x))


def _make_id(seed: str) -> str:
    # Stable ID per result element helps Label Studio preserve object grouping.
    return f"id_{abs(hash(seed)) % 10_000_000}"


def _parse_seg_line(
    line: str,
    from_name: str,
    to_name: str,
    image_name: str,
    line_idx: int,
) -> Optional[Dict[str, Any]]:
    parts = line.split()
    if len(parts) < 7:
        return None

    cls = parts[0]
    if not cls.isdigit():
        return None
    class_id = int(cls)
    label = SEG_CLASSES.get(class_id, f"class_{class_id}")

    coords = []
    for p in parts[1:]:
        f = _safe_float(p)
        if f is None:
            return None
        coords.append(f)

    if len(coords) % 2 != 0:
        return None

    points: List[List[float]] = []
    for i in range(0, len(coords), 2):
        x_pct = _clip_0_100(coords[i] * 100.0)
        y_pct = _clip_0_100(coords[i + 1] * 100.0)
        points.append([x_pct, y_pct])

    if len(points) < 3:
        return None

    return {
        "id": _make_id(f"seg:{image_name}:{line_idx}"),
        "from_name": from_name,
        "to_name": to_name,
        "type": "polygonlabels",
        "value": {
            "points": points,
            "polygonlabels": [label],
        },
    }


def _parse_seg_line_raw(line: str) -> Optional[Tuple[int, List[Tuple[float, float]]]]:
    parts = line.split()
    if len(parts) < 7:
        return None

    cls = parts[0]
    if not cls.isdigit():
        return None

    coords: List[float] = []
    for p in parts[1:]:
        f = _safe_float(p)
        if f is None:
            return None
        coords.append(f)

    if len(coords) % 2 != 0:
        return None

    points: List[Tuple[float, float]] = []
    for i in range(0, len(coords), 2):
        x = max(0.0, min(1.0, coords[i]))
        y = max(0.0, min(1.0, coords[i + 1]))
        points.append((x, y))

    if len(points) < 3:
        return None

    return int(cls), points


def _bits2byte(arr_str: str, n: int = 8) -> List[int]:
    out: List[int] = []
    numbers = [arr_str[i : i + n] for i in range(0, len(arr_str), n)]
    for num in numbers:
        out.append(int(num, 2))
    return out


def _base_rle_encode(inarray: np.ndarray) -> Tuple[np.ndarray, np.ndarray, np.ndarray]:
    ia = np.asarray(inarray)
    n = len(ia)
    if n == 0:
        return np.array([]), np.array([]), np.array([])
    y = ia[1:] != ia[:-1]
    i = np.append(np.where(y), n - 1)
    z = np.diff(np.append(-1, i))
    p = np.cumsum(np.append(0, z))[:-1]
    return z, p, ia[i]


def _encode_rle(arr: np.ndarray, wordsize: int = 8, rle_sizes: List[int] = [3, 4, 8, 16]) -> List[int]:
    num = len(arr)
    numbits = f"{num:032b}"
    wordsizebits = f"{wordsize - 1:05b}"
    rle_bits = "".join([f"{x - 1:04b}" for x in rle_sizes])
    base_str = numbits + wordsizebits + rle_bits

    out_str = ""
    lengths, positions, values = _base_rle_encode(arr)
    for length_reeks, _p, value in zip(lengths, positions, values):
        val = int(value)
        if length_reeks == 1:
            out_str += "0"
            out_str += "00"
            out_str += "000"
            out_str += f"{val:08b}"
        elif length_reeks > 1:
            if length_reeks <= 8:
                out_str += "1"
                out_str += "00"
                out_str += f"{int(length_reeks) - 1:03b}"
                out_str += f"{val:08b}"
            elif 8 < length_reeks <= 16:
                out_str += "1"
                out_str += "01"
                out_str += f"{int(length_reeks) - 1:04b}"
                out_str += f"{val:08b}"
            elif 16 < length_reeks <= 256:
                out_str += "1"
                out_str += "10"
                out_str += f"{int(length_reeks) - 1:08b}"
                out_str += f"{val:08b}"
            else:
                length_temp = int(length_reeks)
                while length_temp > 2**16:
                    out_str += "1"
                    out_str += "11"
                    out_str += f"{2**16 - 1:016b}"
                    out_str += f"{val:08b}"
                    length_temp -= 2**16

                out_str += "1"
                out_str += "11"
                out_str += f"{length_temp - 1:016b}"
                out_str += f"{val:08b}"

    nzfill = 8 - len(base_str + out_str) % 8
    total_str = base_str + out_str
    total_str = total_str + nzfill * "0"
    return _bits2byte(total_str)


def _mask2rle(mask: np.ndarray) -> List[int]:
    assert len(mask.shape) == 2, "mask must be 2D np.array"
    if mask.dtype != np.uint8:
        mask = mask.astype(np.uint8)
    array = mask.ravel()
    array = np.repeat(array, 4)
    return _encode_rle(array)


def _build_seg_brush_results(
    lines: List[str],
    image_path: Path,
    from_name: str,
    to_name: str,
) -> List[Dict[str, Any]]:
    try:
        import cv2
    except Exception as exc:
        raise RuntimeError(
            "Brush-Export benoetigt opencv-python. Bitte installieren und erneut starten."
        ) from exc

    img = cv2.imread(str(image_path), cv2.IMREAD_COLOR)
    if img is None:
        return []
    h, w = img.shape[:2]

    masks_by_label: Dict[str, np.ndarray] = {}
    for idx, line in enumerate(lines):
        parsed = _parse_seg_line_raw(line)
        if not parsed:
            continue

        class_id, points = parsed
        label = SEG_CLASSES.get(class_id, f"class_{class_id}")
        if label not in masks_by_label:
            masks_by_label[label] = np.zeros((h, w), dtype=np.uint8)

        pts = []
        for x, y in points:
            px = int(round(x * (w - 1)))
            py = int(round(y * (h - 1)))
            px = max(0, min(w - 1, px))
            py = max(0, min(h - 1, py))
            pts.append([px, py])

        if len(pts) >= 3:
            cv2.fillPoly(masks_by_label[label], [np.array(pts, dtype=np.int32)], 255)

    results: List[Dict[str, Any]] = []
    for label, mask in masks_by_label.items():
        if np.count_nonzero(mask) == 0:
            continue
        results.append(
            {
                "id": _make_id(f"seg_brush:{image_path.name}:{label}"),
                "type": "brushlabels",
                "value": {
                    "format": "rle",
                    "rle": _mask2rle(mask),
                    "brushlabels": [label],
                },
                "to_name": to_name,
                "from_name": from_name,
                "image_rotation": 0,
                "original_width": int(w),
                "original_height": int(h),
            }
        )

    return results


def _parse_pose_line(
    line: str,
    from_name: str,
    to_name: str,
    image_name: str,
    line_idx: int,
    keypoint_labels: Sequence[str],
) -> List[Dict[str, Any]]:
    parts = line.split()
    if len(parts) < 5:
        return []

    class_id = parts[0]
    if not class_id.isdigit():
        return []

    nums: List[float] = []
    for p in parts[1:]:
        f = _safe_float(p)
        if f is None:
            return []
        nums.append(f)

    if len(nums) < 4:
        return []

    cx, cy, w, h = nums[0], nums[1], nums[2], nums[3]
    kp_values = nums[4:]

    result: List[Dict[str, Any]] = []
    obj_id = _make_id(f"pose_obj:{image_name}:{line_idx}")

    x = _clip_0_100((cx - w / 2.0) * 100.0)
    y = _clip_0_100((cy - h / 2.0) * 100.0)
    width = _clip_0_100(w * 100.0)
    height = _clip_0_100(h * 100.0)

    result.append(
        {
            "id": obj_id,
            "from_name": "bbox",
            "to_name": to_name,
            "type": "rectanglelabels",
            "value": {
                "x": x,
                "y": y,
                "width": width,
                "height": height,
                "rotation": 0,
                "rectanglelabels": ["robot_arm"],
            },
        }
    )

    per_kp = 3
    kp_count = len(kp_values) // per_kp
    for kp_idx in range(min(kp_count, len(keypoint_labels))):
        base = kp_idx * per_kp
        kx = kp_values[base]
        ky = kp_values[base + 1]
        vis = kp_values[base + 2]
        if vis <= 0:
            continue

        kp_label = keypoint_labels[kp_idx]
        result.append(
            {
                "id": _make_id(f"pose_kp:{image_name}:{line_idx}:{kp_idx}"),
                "parentID": obj_id,
                "from_name": from_name,
                "to_name": to_name,
                "type": "keypointlabels",
                "value": {
                    "x": _clip_0_100(kx * 100.0),
                    "y": _clip_0_100(ky * 100.0),
                    "width": 0.6,
                    "keypointlabels": [kp_label],
                },
            }
        )

    return result


def _wrap_result(result_items: List[Dict[str, Any]], import_as: str) -> List[Dict[str, Any]]:
    container: Dict[str, Any] = {
        "result": result_items,
    }
    if import_as == "predictions":
        container["model_version"] = "yolo_auto_annotation"
        container["score"] = 1.0
    return [container]


def build_seg_tasks(
    images_dir: Path,
    labels_dir: Path,
    image_url_prefix: str,
    import_as: str,
    from_name: str,
    to_name: str,
    include_empty: bool,
    seg_format: str,
) -> List[Dict[str, Any]]:
    tasks: List[Dict[str, Any]] = []
    images = _list_images(images_dir)

    for img in images:
        label_path = labels_dir / f"{img.stem}.txt"
        lines = _read_label_lines(label_path)

        if seg_format == "brush":
            results = _build_seg_brush_results(
                lines=lines,
                image_path=img,
                from_name=from_name,
                to_name=to_name,
            )
        else:
            results = []
            for idx, line in enumerate(lines):
                parsed = _parse_seg_line(line, from_name, to_name, img.name, idx)
                if parsed:
                    results.append(parsed)

        if not include_empty and not results:
            continue

        task: Dict[str, Any] = {
            "data": {"image": _join_url(image_url_prefix, img.name)},
        }
        task[import_as] = _wrap_result(results, import_as)
        tasks.append(task)

    return tasks


def build_pose_tasks(
    images_dir: Path,
    labels_dir: Path,
    image_url_prefix: str,
    import_as: str,
    from_name: str,
    to_name: str,
    include_empty: bool,
    keypoint_labels: Sequence[str],
) -> List[Dict[str, Any]]:
    tasks: List[Dict[str, Any]] = []
    images = _list_images(images_dir)

    for img in images:
        label_path = labels_dir / f"{img.stem}.txt"
        lines = _read_label_lines(label_path)

        results: List[Dict[str, Any]] = []
        for idx, line in enumerate(lines):
            results.extend(
                _parse_pose_line(
                    line=line,
                    from_name=from_name,
                    to_name=to_name,
                    image_name=img.name,
                    line_idx=idx,
                    keypoint_labels=keypoint_labels,
                )
            )

        if not include_empty and not results:
            continue

        task: Dict[str, Any] = {
            "data": {"image": _join_url(image_url_prefix, img.name)},
        }
        task[import_as] = _wrap_result(results, import_as)
        tasks.append(task)

    return tasks


def write_json(path: Path, data: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(data, indent=2, ensure_ascii=False), encoding="utf-8")


def _xml_escape(text: str) -> str:
    return (
        text.replace("&", "&amp;")
        .replace("<", "&lt;")
        .replace(">", "&gt;")
        .replace('"', "&quot;")
        .replace("'", "&apos;")
    )


def seg_label_config(from_name: str = "label", to_name: str = "image") -> str:
    labels = "\n".join(
        [f'    <Label value="{_xml_escape(name)}" />' for _, name in sorted(SEG_CLASSES.items())]
    )
    return (
        "<View>\n"
        f"  <Image name=\"{_xml_escape(to_name)}\" value=\"$image\"/>\n"
        f"  <PolygonLabels name=\"{_xml_escape(from_name)}\" toName=\"{_xml_escape(to_name)}\">\n"
        f"{labels}\n"
        "  </PolygonLabels>\n"
        "</View>\n"
    )


def seg_brush_label_config(from_name: str = "brush", to_name: str = "image") -> str:
    labels = "\n".join(
        [f'    <Label value="{_xml_escape(name)}" />' for _, name in sorted(SEG_CLASSES.items())]
    )
    return (
        "<View>\n"
        f"  <Image name=\"{_xml_escape(to_name)}\" value=\"$image\"/>\n"
        f"  <BrushLabels name=\"{_xml_escape(from_name)}\" toName=\"{_xml_escape(to_name)}\">\n"
        f"{labels}\n"
        "  </BrushLabels>\n"
        "</View>\n"
    )


def pose_label_config(
    kp_from_name: str = "kp",
    to_name: str = "image",
    bbox_from_name: str = "bbox",
    rectangle_label: str = "robot_arm",
) -> str:
    # model_index defines a stable keypoint order required by COCO keypoint export.
    kp_labels = "\n".join(
        [
            f'    <Label value="{_xml_escape(lbl)}" model_index="{idx}" />'
            for idx, lbl in enumerate(POSE_KEYPOINT_LABELS)
        ]
    )
    return (
        "<View>\n"
        f"  <Image name=\"{_xml_escape(to_name)}\" value=\"$image\"/>\n"
        f"  <RectangleLabels name=\"{_xml_escape(bbox_from_name)}\" toName=\"{_xml_escape(to_name)}\">\n"
        f"    <Label value=\"{_xml_escape(rectangle_label)}\" />\n"
        "  </RectangleLabels>\n"
        f"  <KeyPointLabels name=\"{_xml_escape(kp_from_name)}\" toName=\"{_xml_escape(to_name)}\" smart=\"false\">\n"
        f"{kp_labels}\n"
        "  </KeyPointLabels>\n"
        "</View>\n"
    )


def _http_json(
    method: str,
    url: str,
    payload: Optional[Any] = None,
    headers: Optional[Dict[str, str]] = None,
    timeout: int = 60,
) -> Tuple[int, str]:
    req_headers = headers.copy() if headers else {}
    data_bytes = None
    if payload is not None:
        data_bytes = json.dumps(payload).encode("utf-8")
        req_headers["Content-Type"] = "application/json"

    request = urllib.request.Request(url=url, method=method.upper(), data=data_bytes, headers=req_headers)
    try:
        with urllib.request.urlopen(request, timeout=timeout) as resp:
            code = resp.getcode()
            body = resp.read().decode("utf-8", errors="replace")
            return code, body
    except urllib.error.HTTPError as err:
        body = err.read().decode("utf-8", errors="replace")
        return err.code, body


def _whoami(base_url: str, headers: Dict[str, str]) -> bool:
    status, _ = _http_json("GET", f"{base_url.rstrip('/')}/api/users/whoami", headers=headers, payload=None)
    return 200 <= status < 300


def _resolve_auth_headers(base_url: str, api_key: str, auth_scheme: str) -> Dict[str, str]:
    auth_scheme = auth_scheme.lower()

    if auth_scheme == "bearer":
        return {"Authorization": f"Bearer {api_key}"}
    if auth_scheme == "token":
        return {"Authorization": f"Token {api_key}"}
    if auth_scheme != "auto":
        raise ValueError("auth-scheme must be one of: auto, bearer, token")

    bearer = {"Authorization": f"Bearer {api_key}"}
    if _whoami(base_url, bearer):
        return bearer

    token = {"Authorization": f"Token {api_key}"}
    if _whoami(base_url, token):
        return token

    raise RuntimeError("Could not authenticate with Bearer or Token scheme.")


def upload_tasks(
    base_url: str,
    api_key: str,
    project_id: int,
    tasks: List[Dict[str, Any]],
    auth_scheme: str = "auto",
) -> None:
    headers = _resolve_auth_headers(base_url, api_key, auth_scheme)
    url = f"{base_url.rstrip('/')}/api/projects/{project_id}/import"

    status, body = _http_json("POST", url, payload=tasks, headers=headers)
    if not (200 <= status < 300):
        raise RuntimeError(f"Upload failed ({status}): {body[:1000]}")

    print(f"Upload erfolgreich: HTTP {status}")
    if body.strip():
        print(body[:1000])


def add_common_args(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--images-dir", type=Path, required=True)
    parser.add_argument("--labels-dir", type=Path, required=True)
    parser.add_argument("--image-url-prefix", type=str, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--import-as", choices=["predictions", "annotations"], default="predictions")
    parser.add_argument("--to-name", default="image")
    parser.add_argument("--include-empty", action="store_true", help="Include images with no parsed labels")

    parser.add_argument("--upload", action="store_true")
    parser.add_argument("--project-id", type=int)
    parser.add_argument("--label-studio-url", default=os.getenv("LABEL_STUDIO_URL", "http://localhost:8080"))
    parser.add_argument("--api-key", default=os.getenv("LABEL_STUDIO_API_KEY", ""))
    parser.add_argument("--auth-scheme", choices=["auto", "bearer", "token"], default="auto")


def _validate_common(args: argparse.Namespace) -> None:
    if not args.images_dir.exists():
        raise FileNotFoundError(f"images-dir nicht gefunden: {args.images_dir}")
    if not args.labels_dir.exists():
        raise FileNotFoundError(f"labels-dir nicht gefunden: {args.labels_dir}")
    if args.upload:
        if not args.project_id:
            raise ValueError("--project-id ist erforderlich mit --upload")
        if not args.api_key:
            raise ValueError(
                "Kein API-Key gefunden. Nutze --api-key oder setze LABEL_STUDIO_API_KEY."
            )


def _command_seg(args: argparse.Namespace) -> int:
    _validate_common(args)

    tasks = build_seg_tasks(
        images_dir=args.images_dir,
        labels_dir=args.labels_dir,
        image_url_prefix=args.image_url_prefix,
        import_as=args.import_as,
        from_name=args.from_name,
        to_name=args.to_name,
        include_empty=args.include_empty,
        seg_format=args.seg_format,
    )
    write_json(args.output, tasks)

    cfg_name = "label_config_seg_brush.xml" if args.seg_format == "brush" else "label_config_seg.xml"
    cfg_path = args.output.parent / cfg_name
    if args.seg_format == "brush":
        cfg_path.write_text(
            seg_brush_label_config(from_name=args.from_name, to_name=args.to_name),
            encoding="utf-8",
        )
    else:
        cfg_path.write_text(seg_label_config(from_name=args.from_name, to_name=args.to_name), encoding="utf-8")

    print(f"Seg-Tasks geschrieben: {args.output} ({len(tasks)} Tasks)")
    print(f"Seg-Label-Config: {cfg_path}")

    if args.upload:
        upload_tasks(
            base_url=args.label_studio_url,
            api_key=args.api_key,
            project_id=args.project_id,
            tasks=tasks,
            auth_scheme=args.auth_scheme,
        )

    return 0


def _command_pose(args: argparse.Namespace) -> int:
    _validate_common(args)

    keypoint_labels = [k.strip() for k in args.keypoint_labels.split(",") if k.strip()]
    if not keypoint_labels:
        keypoint_labels = list(POSE_KEYPOINT_LABELS)

    tasks = build_pose_tasks(
        images_dir=args.images_dir,
        labels_dir=args.labels_dir,
        image_url_prefix=args.image_url_prefix,
        import_as=args.import_as,
        from_name=args.from_name,
        to_name=args.to_name,
        include_empty=args.include_empty,
        keypoint_labels=keypoint_labels,
    )
    write_json(args.output, tasks)

    cfg_path = args.output.parent / "label_config_pose.xml"
    cfg_path.write_text(
        pose_label_config(
            kp_from_name=args.from_name,
            to_name=args.to_name,
            bbox_from_name="bbox",
            rectangle_label="robot_arm",
        ),
        encoding="utf-8",
    )

    print(f"Pose-Tasks geschrieben: {args.output} ({len(tasks)} Tasks)")
    print(f"Pose-Label-Config: {cfg_path}")

    if args.upload:
        upload_tasks(
            base_url=args.label_studio_url,
            api_key=args.api_key,
            project_id=args.project_id,
            tasks=tasks,
            auth_scheme=args.auth_scheme,
        )

    return 0


def _command_prepare_all(args: argparse.Namespace) -> int:
    root = args.root.resolve()

    out_dir = root / "label_studio_import"
    out_dir.mkdir(parents=True, exist_ok=True)

    seg_output_name = "tasks_seg_all_brush.json" if args.seg_format == "brush" else "tasks_seg_all.json"

    seg_args = argparse.Namespace(
        images_dir=root / "real_dataset" / "images" / "all",
        labels_dir=root / "real_dataset" / "labels" / "all",
        image_url_prefix=f"{args.image_host.rstrip('/')}/real_dataset/images/all",
        output=out_dir / seg_output_name,
        import_as=args.import_as,
        from_name=("brush" if args.seg_format == "brush" else "label"),
        to_name="image",
        include_empty=args.include_empty,
        seg_format=args.seg_format,
        upload=args.upload,
        project_id=args.seg_project_id,
        label_studio_url=args.label_studio_url,
        api_key=args.api_key,
        auth_scheme=args.auth_scheme,
    )

    pose_train_args = argparse.Namespace(
        images_dir=root / "real_pose_dataset" / "images" / "train",
        labels_dir=root / "real_pose_dataset" / "labels" / "train",
        image_url_prefix=f"{args.image_host.rstrip('/')}/real_pose_dataset/images/train",
        output=out_dir / "tasks_pose_train.json",
        import_as=args.import_as,
        from_name="kp",
        to_name="image",
        include_empty=args.include_empty,
        keypoint_labels=",".join(POSE_KEYPOINT_LABELS),
        upload=args.upload,
        project_id=args.pose_project_id,
        label_studio_url=args.label_studio_url,
        api_key=args.api_key,
        auth_scheme=args.auth_scheme,
    )

    pose_val_args = argparse.Namespace(
        images_dir=root / "real_pose_dataset" / "images" / "val",
        labels_dir=root / "real_pose_dataset" / "labels" / "val",
        image_url_prefix=f"{args.image_host.rstrip('/')}/real_pose_dataset/images/val",
        output=out_dir / "tasks_pose_val.json",
        import_as=args.import_as,
        from_name="kp",
        to_name="image",
        include_empty=args.include_empty,
        keypoint_labels=",".join(POSE_KEYPOINT_LABELS),
        upload=args.upload,
        project_id=args.pose_project_id,
        label_studio_url=args.label_studio_url,
        api_key=args.api_key,
        auth_scheme=args.auth_scheme,
    )

    # Upload is optional and per project. If IDs are missing, only write JSON.
    if args.upload and (not args.seg_project_id or not args.pose_project_id):
        raise ValueError("Mit --upload sind --seg-project-id und --pose-project-id erforderlich.")

    _command_seg(seg_args)
    _command_pose(pose_train_args)
    _command_pose(pose_val_args)

    print("Alle Dateien vorbereitet.")
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="YOLO -> Label Studio Konverter")
    sub = parser.add_subparsers(dest="command", required=True)

    seg = sub.add_parser("seg", help="YOLO-Segmentation Labels -> Label Studio Tasks")
    add_common_args(seg)
    seg.add_argument("--from-name", default="label")
    seg.add_argument("--seg-format", choices=["polygon", "brush"], default="polygon")

    pose = sub.add_parser("pose", help="YOLO-Pose Labels -> Label Studio Tasks")
    add_common_args(pose)
    pose.add_argument("--from-name", default="kp")
    pose.add_argument(
        "--keypoint-labels",
        default=",".join(POSE_KEYPOINT_LABELS),
        help="Comma-separated labels for keypoints in index order",
    )

    all_cmd = sub.add_parser("prepare-all", help="Prepare seg + pose train + pose val in one run")
    all_cmd.add_argument("--root", type=Path, default=Path("."), help="Training_Scripts root")
    all_cmd.add_argument("--image-host", default="http://localhost:3000")
    all_cmd.add_argument("--import-as", choices=["predictions", "annotations"], default="predictions")
    all_cmd.add_argument("--seg-format", choices=["polygon", "brush"], default="polygon")
    all_cmd.add_argument("--include-empty", action="store_true")

    all_cmd.add_argument("--upload", action="store_true")
    all_cmd.add_argument("--seg-project-id", type=int)
    all_cmd.add_argument("--pose-project-id", type=int)
    all_cmd.add_argument("--label-studio-url", default=os.getenv("LABEL_STUDIO_URL", "http://localhost:8080"))
    all_cmd.add_argument("--api-key", default=os.getenv("LABEL_STUDIO_API_KEY", ""))
    all_cmd.add_argument("--auth-scheme", choices=["auto", "bearer", "token"], default="auto")

    return parser


def main(argv: Optional[Sequence[str]] = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)

    try:
        if args.command == "seg":
            return _command_seg(args)
        if args.command == "pose":
            return _command_pose(args)
        if args.command == "prepare-all":
            return _command_prepare_all(args)
    except Exception as exc:
        print(f"FEHLER: {exc}", file=sys.stderr)
        return 1

    return 1


if __name__ == "__main__":
    raise SystemExit(main())
