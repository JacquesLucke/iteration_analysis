import os

directory_path = "/home/jacques/blender-git/blender/source/blender/"

paths_to_check = []

file_endings = {".h", ".c", ".hh", ".hpp", ".cc", ".cpp"}

for root, dirs, file_names in os.walk(directory_path):
    for file_name in file_names:
        file_path = os.path.join(root, file_name)
        if any(file_path.endswith(ext) for ext in file_endings):
            if file_name != "BLI_listbase.h":
                paths_to_check.append(file_path)

def update_next_occurence(code):
    lines = list(code.splitlines())
    for line_index, line in enumerate(lines):
        if "LISTBASE_FOREACH (" not in line:
            continue

        indentation = line.index("LISTBASE_FOREACH")
        for other_line_index, other_line in enumerate(lines[line_index + 1:], start=line_index + 1):
            if "}" in other_line and other_line.index("}") == indentation:
                print(line)
                print(other_line)
                break

        if any(("break" in line or "continue" in line or "return" in line) for line in lines[line_index:other_line_index]):
            continue
        if "pass->shgroups" in line:
            continue

        print(line)
        split = line.split(",")
        if len(split) == 1:
            continue
        var_name = split[1].strip()
        lines[line_index] = lines[line_index].replace("LISTBASE_FOREACH", "LISTBASE_FOREACH_FAST_BEGIN")
        lines.insert(other_line_index + 1, " " * indentation + f"LISTBASE_FOREACH_FAST_END({var_name});")
        return True, "\n".join(lines)

    return False, code

counter = 0
for i, file_path in enumerate(paths_to_check):
    # print(f"[{i+1}/{len(paths_to_check)}] {file_path}")
    with open(file_path) as f:
        code = f.read()

    changed_at_all = False
    while True:
        code_changed, code = update_next_occurence(code)
        if code_changed:
            changed_at_all = True
        else:
            break

    if changed_at_all:
        with open(file_path, "w") as f:
            f.write(code)
