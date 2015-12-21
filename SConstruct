import os
import sys
import json
from tools import athena_unit_class_generator, athena_bin_to_c

environment = Environment(ENV = os.environ)
environment.Append(tools=["nasm"])
print os.name
print sys.platform

environment = SConscript("SConsBuildOptions", exports="environment")

environment.Prepend(
    CPPPATH = [os.path.join(os.getcwd(), "TurboJSON_src"), os.path.join(os.getcwd(), "include")], 
    LIBPATH = [os.path.join(os.getcwd(), "lib")])

saved_env = environment.Clone()

build_tests = False
turbojson = SConscript(dirs = ["TurboJSON_src/TurboJSON"], exports = ["environment", "build_tests"])

environment = saved_env.Clone()

unit_classes_prefix = os.path.join("src", "unit_classes")
font_sources_prefix = os.path.join("src", "sgi_screen")

def CreateAthenaClassesSourceCommand(env, target, source):
    unit_classes_file = open(str(source[0]), "r")
    unit_classes_data = json.load(unit_classes_file)
    unit_classes_file.close()
    athena_unit_class_generator.CreateAthenaClassesSource(unit_classes_data, target)
    return []

def CreateBinaryCFiles(env, target, source):
    athena_bin_to_c.BinToC(source[0], target)

gen_unit_class_definitions = environment.Command(
    [unit_classes_prefix + ".c", unit_classes_prefix + ".h"], 
     unit_classes_prefix + ".json", 
    CreateAthenaClassesSourceCommand)

gen_sgi_font_include = environment.Command(
    ["src/sgi_screen.c", "src/sgi_screen.h"],
    "res/fonts/sgi/sgi_screen.rfn",
    CreateBinaryCFiles
)

gen_cynical_font_include = environment.Command(
    ["src/cynical.c", "src/cynical.h"],
    "res/fonts/cynical.rfn",
    CreateBinaryCFiles
)

gen_selawik_font_include = environment.Command(
    ["src/selawik.c", "src/selawik.h"],
    "res/fonts/selawik/selawik-small-bold.rfn",
    CreateBinaryCFiles
)

libathena = SConscript(dirs = ["src"], exports = ["environment", "turbojson"])

environment.Install(os.getcwd(), libathena)

AddOption('--editor', dest="editor", nargs=1, action='store')

if GetOption('editor') == 'y':
    editor = SConscript(dirs = ["editor"], exports = ["environment", "turbojson", "libathena"])
    environment.Install(os.getcwd(), editor)
