#!/usr/bin/python

from ninja_syntax import Writer
import os, sys

source_dirs = [
        ".",
        "driverlib",
        "inc",
        "drivers"
]

include_dirs = source_dirs + [
        "registers",
        ]

def subst_ext(fname, ext):
    return os.path.splitext(fname)[0] + ext

def get_sources():
    fnames = []
    for d in source_dirs:
        for f in os.listdir(d):
            fnames.append(os.path.join(d, f))
    return fnames

def get_includes():
    return " ".join(map(lambda x : "-I"+x, include_dirs))

cflags = ("-g -c -Os -ffunction-sections -fdata-sections " +
         "-mthumb -mcpu=cortex-m4 -mfloat-abi=hard " +
         "-mfpu=fpv4-sp-d16 -fsingle-precision-constant " +
         "-DF_CPU=80000000L -DPART_TM4C123GH6PM " +
         get_includes())

cxxflags = ("-g -c -Os -std=c++14 -fno-rtti -fno-exceptions " +
           "-ffunction-sections -fdata-sections -mthumb " +
           "-mcpu=cortex-m4 -mfloat-abi=hard " +
           "-mfpu=fpv4-sp-d16 -fsingle-precision-constant " +
           "-DF_CPU=80000000L -DPART_TM4C123GH6PM " +
           get_includes())

lflags = ("-g -Os -nostartfiles -Wl,--gc-sections " +
         "-T tm4c123gh6pm.ld -Wl,--entry=ResetISR -mthumb " +
         "-mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 " +
         "-fsingle-precision-constant -lm -lstdc++ -lc")

def write_buildfile():
    with open("build.ninja", "w") as buildfile:
        n = Writer(buildfile)

        # Variable declarations
        n.variable("lib_path", "/usr/arm-none-eabi/lib")
        n.variable("cflags", cflags)
        n.variable("cxxflags", cxxflags)
        n.variable("lflags", lflags)

        # Rule declarations
        n.rule("cxx",
               command = "arm-none-eabi-g++ $cxxflags -c $in -o $out")

        n.rule("cc",
               command = "arm-none-eabi-gcc $cflags -c $in -o $out")

        n.rule("cl",
               command = "arm-none-eabi-gcc $lflags $in -o $out")

        n.rule("oc",
               command = "arm-none-eabi-objcopy -O binary $in $out")

        n.rule("cdb",
              command = "ninja -t compdb cc cxx > compile_commands.json")

        n.rule("cscf",
              command = "find " + " ".join(set(source_dirs + include_dirs)) +
                        " -regex \".*\\(\\.c\\|\\.h\\|.cpp\\|.hpp\\)$$\" -and " +
                        "-not -type d > $out")

        n.rule("cscdb",
              command = "cscope -bq")

        # Build rules
        n.build("compile_commands.json", "cdb")
        n.build("cscope.files", "cscf")
        n.build(["cscope.in.out", "cscope.po.out", "cscope.out"], "cscdb",
                "cscope.files")

        objects = []

        def cc(name):
            ofile = subst_ext(name, ".o")
            n.build(ofile, "cc", name)
            objects.append(ofile)
        def cxx(name):
            ofile = subst_ext(name, ".o")
            n.build(ofile, "cxx", name)
            objects.append(ofile)
        def cl(oname, ofiles):
            n.build(oname, "cl", ofiles)

        sources = get_sources()
        map(cc, filter(lambda x : x.endswith(".c") or x.endswith(".S"), sources))
        map(cxx, filter(lambda x : x.endswith(".cpp"), sources))

        cl("main.elf", objects)

        n.build("main.bin", "oc", "main.elf")

if __name__ == "__main__":
    write_buildfile()

