import json
from pathlib import Path

nb_path = Path('Validierung.ipynb')
with open(nb_path, 'r', encoding='utf-8') as f:
    nb = json.load(f)

# Cell 20 (Index 19) - Blender Pose
cell_20_source = [
    'from ultralytics import YOLO',
    'import cv2',
    'import matplotlib.pyplot as plt',
    'import numpy as np',
    'from pathlib import Path',
    '',
    'pose_model_path = POSE_BLENDER_MODEL',
    'if not pose_model_path.exists():',
    '    print(f"Blender pose model not found: {pose_model_path}")',
    'else:',
    '    model_pose = YOLO(str(pose_model_path))',
    '    print(f"Verwendetes Pose-Modell (Blender, neuester Run): {POSE_BLENDER_RUN}")',
    '    ',
    '    val_image_dir = BLENDER_DIR / "yolo_dataset" / "images" / "val"',
    '    image_paths = sorted(val_image_dir.glob("*.png"))[:3]',
    '    ',
    '    for img_path in image_paths:',
    '        results = model_pose(str(img_path), conf=0.25)',
    '        fig, axes = plt.subplots(1, 2, figsize=(16, 6))',
    '        ',
    '        original = cv2.cvtColor(cv2.imread(str(img_path)), cv2.COLOR_BGR2RGB)',
    '        axes[0].imshow(original)',
    '        axes[0].set_title(f"Original: {img_path.name}")',
    '        axes[0].axis("off")',
    '        ',
    '        annotated = results[0].plot()',
    '        axes[1].imshow(cv2.cvtColor(annotated, cv2.COLOR_BGR2RGB))',
    '        axes[1].set_title(f"YOLO Pose auf Blender-Bild | {POSE_BLENDER_RUN}")',
    '        axes[1].axis("off")',
    '        ',
    '        plt.tight_layout()',
    '        plt.show()'
]

nb['cells'][19]['source'] = cell_20_source

# Cell 24 (Index 23) - Real Pose
cell_24_source = [
    'from ultralytics import YOLO',
    'import cv2',
    'import matplotlib.pyplot as plt',
    'import numpy as np',
    'from pathlib import Path',
    '',
    'DESIRED_KPTS = 9',
    'pose_model_path = POSE_REAL_MODEL',
    'if not pose_model_path.exists():',
    '    print(f"Real pose model not found: {pose_model_path}")',
    'else:',
    '    model_pose = YOLO(str(pose_model_path))',
    '    print(f"Verwendetes Pose-Modell (Real, neuester Run): {POSE_REAL_RUN}")',
    '    ',
    '    REAL_IMAGE_CANDIDATES = [',
    '        PROJECT_ROOT.parent / "Image_Capture_Python" / "dataset" / "images" / "angepasstes_seitenv_20260311_170850",',
    '        NOTEBOOK_DIR / "real_dataset" / "images" / "all",',
    '    ]',
    '    REAL_IMAGE_PATH = _first_existing_path(REAL_IMAGE_CANDIDATES)',
    '    p = Path(REAL_IMAGE_PATH)',
    '    ',
    '    if p.is_dir():',
    '        image_paths = sorted(p.glob("*.jpg")) + sorted(p.glob("*.png")) + sorted(p.glob("*.jpeg"))',
    '        image_paths = image_paths[:5]',
    '    else:',
    '        image_paths = [p]',
    '    ',
    '    kpt_shape = getattr(model_pose.model, "kpt_shape", None)',
    '    model_kpt_count = kpt_shape[0] if isinstance(kpt_shape, (list, tuple)) and len(kpt_shape) > 0 else None',
    '    print(f"Modell-Keypoints: {model_kpt_count}")',
    '    ',
    '    for img_path in image_paths:',
    '        results = model_pose(str(img_path), conf=0.25)',
    '        fig, axes = plt.subplots(1, 2, figsize=(18, 7))',
    '        ',
    '        original = cv2.cvtColor(cv2.imread(str(img_path)), cv2.COLOR_BGR2RGB)',
    '        axes[0].imshow(original)',
    '        axes[0].set_title(f"Original: {img_path.name}", fontsize=12)',
    '        axes[0].axis("off")',
    '        ',
    '        annotated = results[0].plot()',
    '        axes[1].imshow(cv2.cvtColor(annotated, cv2.COLOR_BGR2RGB))',
    '        axes[1].set_title(f"YOLO Pose (Real Keypoints) | {POSE_REAL_RUN}", fontsize=12)',
    '        axes[1].axis("off")',
    '        ',
    '        plt.tight_layout()',
    '        plt.show()'
]

nb['cells'][23]['source'] = cell_24_source

with open(nb_path, 'w', encoding='utf-8') as f:
    json.dump(nb, f, indent=1, ensure_ascii=False)

print('✓ Keypoint-Zellen (20 & 24) erfolgreich aktualisiert')
