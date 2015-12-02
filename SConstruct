import os
import sys
import json
from tools import athena_unit_class_generator, athena_bin_to_c

environment = Environment(ENV = os.environ)

use_intel_cc = False

AddOption('--use-intel-cc', dest="use-intel-cc", nargs=1, action='store')

if GetOption('use-intel-cc') == 'y':
    use_intel_cc = True

print os.name
print sys.platform

if os.name=="posix":
    environment.Append(
        CFLAGS = " -ansi -Wno-long-long ", 
        CXXFLAGS = " -std=c++11 -fno-rtti -fno-exceptions ",
        LINKFLAGS = " -g ")
    environment.Prepend(CCFLAGS = " -pedantic -Wall -Werror -g -fstrict-aliasing -pipe ")
    if not use_intel_cc:
         environment.Prepend(CCFLAGS = " -Wno-unused-result ")
         if not sys.platform.startswith("cyg") and not sys.platform=="msys":
             environment.Prepend(CCFLAGS = " -fPIC ")
    if False and sys.platform == "darwin":
        XFlag = " -arch x86_64 -arch i586 "
        environment.Append(CCFLAGS = XFlag, LINKFLAGS = XFlag)

if use_intel_cc:
    environment.Replace(CC = "icc")

environment.Prepend(
    CPPPATH = [os.path.join(os.getcwd(), "TurboJSON_src"), os.path.join(os.getcwd(), "include")], 
    LIBPATH = [os.path.join(os.getcwd(), "lib")])

saved_env = environment.Clone()

turbojson = SConscript(dirs = ["TurboJSON_src/TurboJSON"], exports = ["environment"])

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
    [font_sources_prefix + ".c", font_sources_prefix + ".h"],
    "res/fonts/sgi/sgi_screen.rfn",
    CreateBinaryCFiles
)

gen_cynical_font_include = environment.Command(
    ["src/cynical.c", "src/cynical.h"],
    "res/fonts/cynical.rfn",
    CreateBinaryCFiles
)

athena = SConscript(dirs = ["src"], exports = ["environment", "turbojson"])

environment.Install(os.getcwd(), athena)
