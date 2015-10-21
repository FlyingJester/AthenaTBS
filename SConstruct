import os
import sys
import json
from tools import athena_unit_class_generator

environment = Environment(ENV = os.environ)

use_intel_cc = False

AddOption('--use-intel-cc', dest="use-intel-cc", nargs=1, action='store')

if GetOption('use-intel-cc') == 'y':
    use_intel_cc = True

if os.name=="posix":
    environment.Append(
        CFLAGS = " -ansi -Wno-long-long ", 
        CXXFLAGS = " -std=c++11 -fno-rtti -fno-exceptions ",
        LINKFLAGS = " -g ")
    environment.Prepend(CCFLAGS = " -pedantic -Wall -Werror -g -fstrict-aliasing -pipe -fPIC ")
    if not use_intel_cc:
         environment.Prepend(CCFLAGS = " -Wno-unused-result ")

    if False and sys.platform == "darwin":
        XFlag = " -arch x86_64 -arch i586 "
        environment.Append(CCFLAGS = XFlag, LINKFLAGS = XFlag)

if use_intel_cc:
    environment.Replace(CC = "icc")

environment.Prepend(
    CPPPATH = [os.path.join(os.getcwd(), "TurboJSON_src"), os.path.join(os.getcwd(), "include")], 
    LIBPATH = [os.path.join(os.getcwd(), "lib")])

ó
saved_env = environment.Clone()

turbojson = SConscript(dirs = ["TurboJSON_src/TurboJSON"], exports = ["environment"])

environment = saved_env.Clone()

unit_classes_prefix = os.path.join("src", "unit_classes")

def CreateAthenaClassesSourceCommand(env, target, source):
    unit_classes_file = open(unit_classes_prefix + ".json", "r")
    unit_classes_data = json.load(unit_classes_file)
    unit_classes_file.close()
    athena_unit_class_generator.CreateAthenaClassesSource(unit_classes_data, target)
    return []

gen_unit_class_definitions = environment.Command(
    [unit_classes_prefix + ".c", unit_classes_prefix + ".h"], 
     unit_classes_prefix + ".json", 
    CreateAthenaClassesSourceCommand)

athena = SConscript(dirs = ["src"], exports = ["environment", "turbojson"])
environment.Install(os.getcwd(), athena)
