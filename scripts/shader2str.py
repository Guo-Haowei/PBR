import os
import errno

py_source_dir = os.path.dirname(os.path.abspath(__file__))
temp_dir = py_source_dir + '/temp/'
py_source_dir_arr = py_source_dir.split("/")

glsl_source_dir_arr = py_source_dir_arr
glsl_source_dir_arr.pop()
glsl_source_dir_arr.extend(['data', 'shaders', 'glsl'])
glsl_source_dir = '/'.join(glsl_source_dir_arr)

# collect glsl files
glsl_files = []
for root, directory, files in os.walk(glsl_source_dir):
    for file in files:
        glsl_files.append({ 'path' : os.path.join(root, file), 'filename' : file })

# create temp folder
if not os.path.exists(os.path.dirname(temp_dir)):
    try:
        os.makedirs(os.path.dirname(temp_dir))
    except OSError as e: # Guard against race condition
        if e.errno != errno.EEXIST:
            raise

# temp directory

output_file_path = temp_dir + 'shaders.generated.h'
outfile = open(output_file_path, 'w+')
outfile.write('#pragma once\nnamespace generated {\n\n')

version_strings = [
    "#version 300 es",
    "precision highp float;",
    "precision highp int;",
]

for obj in glsl_files:
    infile = open(obj['path'], 'r')
    lines = infile.read().splitlines()
    infile.close()
    lines.pop(0) # remove version string
    lines = version_strings + lines
    filename = obj['filename']
    variable_name = filename.replace(r'.', '_')
    variable_name += '_c_str'

    outfile.write('static const char* ')
    outfile.write(variable_name)
    outfile.write(' =\n')
    for line in lines:
        # if line == '':
        #     continue
        ## by keeping the blank line, it's easier to debug shader error
        outfile.write('\t"')
        outfile.write(line)
        outfile.write('\\n"')
        outfile.write('\\\n')
    outfile.write('\t"\\n";\n\n')

outfile.write('} // namespace generated\n')
outfile.close()
