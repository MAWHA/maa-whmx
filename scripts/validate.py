from itertools import chain, product
from collections import Counter
import os
import json
import sys

def format_path(path: str) -> str:
    if path.startswith('./') or path.startswith('.\\'):
        path = path[2:]
    return path.replace('\\', '/')

def get_string_or_array(item: dict, key: str) -> list[str] | None:
    value = item.get(key)
    if isinstance(value, str):
        return [value]
    elif isinstance(value, list):
        return value
    else:
        return None

def get_task_template_pairs(entry: dict) -> list[tuple[str, str]]:
    template = entry[1].get('template') or ''
    if not isinstance(template, list):
        template = [template]
    template = [format_path(t) for t in template]
    return list(product([entry[0]], template))

def flatten(items: any) -> list[any]:
    return list(chain.from_iterable(items))

def validate_assets(assets_dir: str) -> None:
    if not os.path.exists(assets_dir):
        print(f"{assets_dir} does not exist")
        return

    pipeline_dir = f'{assets_dir}/pipeline'
    image_dir = f'{assets_dir}/image'

    pipeline_assets = flatten([[f'{dir}/{file}' for file in files if file.endswith(".json")] for dir, _, files in os.walk(pipeline_dir)])
    pipelines = [json.load(open(f, 'r', encoding='utf-8')) for f in pipeline_assets]
    defined_tasks = list(flatten([list(pipeline.keys()) for pipeline in pipelines]))
    ref_tasks = set(flatten([product([entry[0]], get_string_or_array(entry[1], 'next') or []) for pipeline in pipelines for entry in pipeline.items()]))


    duplicate_tasks = list(filter(lambda e: e[1] > 1, Counter(defined_tasks).items()))
    if len(duplicate_tasks) > 0:
        print(f"{len(duplicate_tasks)} duplicate tasks found in assets:")
        for task, _ in duplicate_tasks:
            print(f"  * {task}")

    if not all([e[1] in defined_tasks for e in ref_tasks]):
        undefined_tasks = [e for e in ref_tasks if e[1] not in defined_tasks]
        print(f"{len(undefined_tasks)} undefined but referenced tasks found in assets:")
        for parent, task in undefined_tasks:
            print(f"  * {task}, referenced by {parent}")

    image_assets = list(flatten([[f'{format_path(os.path.relpath(dir, image_dir))}/{file}' for file in files] for dir, _, files in os.walk(image_dir)]))
    ref_images = set(filter(lambda e: len(e[1]) > 0, flatten([get_task_template_pairs(entry) for pipeline in pipelines for entry in pipeline.items()])))
    if not all([e[1] in image_assets for e in ref_images]):
        unknown_images = [e for e in ref_images if e[1] not in image_assets]
        print(f"{len(unknown_images)} not found but referenced images found in assets:")
        for parent, image in unknown_images:
            print(f"  * {image}, referenced by {parent}")

if __name__ == "__main__":
    if len(sys.argv) > 2:
        for arg in sys.argv[1:]:
            print(f"VALIDATE {arg}\n---")
            validate_assets(arg)
            print("")
    elif len(sys.argv) == 2:
        validate_assets(sys.argv[1])
    else:
        print("Usage: python validate.py [assets_dir...]")
