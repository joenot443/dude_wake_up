import os
import fileinput

def insert_line(directory):
    for subdir, dirs, files in os.walk(directory):
        for file in files:
            if file.endswith('.hpp'):
                filepath = subdir + os.sep + file

                with fileinput.FileInput(filepath, inplace=True, backup='.bak') as file:
                    for line in file:
                        # This print will write the current line back to the file
                        print(line, end='')
                        if 'load(j);' in line:
                            # This print will write the new line after the matched line
                            print('  registerParameters();')

# Provide the directory here where you want to start the search
insert_line(".")
