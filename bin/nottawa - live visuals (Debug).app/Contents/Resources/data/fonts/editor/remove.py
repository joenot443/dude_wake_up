import os
import re

def camel_to_spaced(name):
    return re.sub(r'(?<!^)(?=[A-Z])', ' ', name)

for filename in os.listdir('.'):
    if not os.path.isfile(filename):
        continue

    base, ext = os.path.splitext(filename)

    # Remove '-regular' or '- Regular' (case-insensitive, optional space)
    base = re.sub(r'-\s*regular', '', base, flags=re.IGNORECASE)

    # Convert CamelCase to Spaced Case
    spaced = camel_to_spaced(base)

    new_filename = spaced.strip() + ext
    if filename != new_filename:
        os.rename(filename, new_filename)
        print(f'Renamed: "{filename}" → "{new_filename}"')