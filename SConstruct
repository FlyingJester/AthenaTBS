import os
import sys

environment = Environment(ENV = os.environ)

if os.name=="posix":
    environment.Append(
        CFLAGS = " -ansi -Wno-long-long ", 
        CXXFLAGS = " -std=c++11 -fno-rtti -fno-exceptions ",
        LINKFLAGS = " -g ")
    environment.Prepend(CCFLAGS = " -pedantic -Wall -Werror -g -fstrict-aliasing -pipe -fPIC")

    if False and sys.platform == "darwin":
        XFlag = " -arch x86_64 -arch i586 "
        environment.Append(CCFLAGS = XFlag, LINKFLAGS = XFlag)

environment.Prepend(
    CPPPATH = [os.path.join(os.getcwd(), "TurboJSON_src"), os.path.join(os.getcwd(), "lib")], 
    LIBPATH = [os.path.join(os.getcwd(), "lib")])

saved_env = environment.Clone()

turbojson = SConscript(dirs = ["TurboJSON_src/TurboJSON"], exports = ["environment"])

environment = saved_env.Clone()

athena = SConscript(dirs = ["src"], exports = ["environment", "turbojson"])
environment.Install(os.getcwd(), athena)
