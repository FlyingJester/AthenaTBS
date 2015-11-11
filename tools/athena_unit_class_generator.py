from string import Template
import inspect

autogen_warning =  """
/* THIS FILE IS AUTOGENERATED BY:
 *    tools/athena_unit_class_generator.py
 * DO NOT EDIT THIS FILE MANUALLY
 */
 \n
"""
# struct Athena_Class {
#     float defense, attack;
#     unsigned movement, attacks;
#     unsigned range;
#     const char *name;
#     struct Athena_Spriteset *spriteset;

def gen_int_to_str(i):
    return str(i).split('.')[0]

def gen_bool_to_int(n, that):
    if ((n in that) and that[n]):
        return 1
    else:
        return 0

def gen_bool_to_str(n, dict):
    return gen_int_to_str(gen_bool_to_int(n, dict))

def GenerateAthenaClassDictionary(that, n):
    return dict(
        defense = str(that["defense"]),
        attack  = str(that["attack"]),
        movement= gen_int_to_str(that["movement"]),
        actions = gen_int_to_str(that["actions"]),
        range   = gen_int_to_str(that["range"]),
        is_building = gen_bool_to_str("is_building", that),
        can_build = gen_bool_to_str("can_build", that),
        is_path = gen_bool_to_str("is_path", that),
        name    = str(that["name"]),
        i = n
    )

def CreateAthenaClassesSource(classes, paths):
    for p in paths:
        path = str(p)
        if path.endswith(".c"):
            source_path = path
        elif path.endswith(".h"):
            header_path = path

    source = open( source_path, "w" )
    header = open( header_path, "w" )
    
    header_preamble = "#pragma once\n" + autogen_warning + "#include \"unit.h\"\n"
    source_preamble = autogen_warning + "#include \"unit.h\"\n"
    
    init_definition = "int Athena_UnitClassesInit()"
    
    source.write(source_preamble)
    header.write(header_preamble)
    
    header.write("\n#define ATHENA_NUM_UNIT_CLASSES " + str(len(classes)) + "\n\n")
    
    source.write( "#include \"" + header_path.split('/')[-1] + "\"\n")
    source.write( "#include \"spriteset.h\"\n\n")
    
    class_array_def = "const struct Athena_Class athena_unit_classes[ATHENA_NUM_UNIT_CLASSES]"
    
    source.write("static struct Athena_Spriteset athena_unit_classes_spritesets[ATHENA_NUM_UNIT_CLASSES];\n")
    
    header.write("extern " + class_array_def + ";\n")
    source.write(class_array_def + " = {\n")
    
    class_template = Template(" $defense, $attack, $movement, $actions, $range, $is_building, $can_build, $is_path, \"$name\",\n        athena_unit_classes_spritesets + $i ")
    
    i = 0
    for that in classes:
        source.write("    {" + class_template.substitute(GenerateAthenaClassDictionary(that, i)) + "}")
        if not (that == classes[-1]):
            source.write(",")
        source.write("\n")
        i = i+1
        
    source.write("};\n")
    
    header.write(init_definition + ";\n")
    source.write(init_definition + "{\n")
    
    source.write("    int err = 0;\n")
    
    i = 0
    for that in classes:
        if "spriteset" in that:
            spriteset = str(that["spriteset"])
        else:
            spriteset = str(that["name"])
        load_command =  "Athena_LoadSpritesetFromFile(\"res/spritesets/" + spriteset + "/ss.json\", athena_unit_classes_spritesets + " + gen_int_to_str(i) + ")"
        source.write("    {\n        const int n_err = " + load_command + ";\n");
        source.write("        if(n_err) fprintf(stderr, \"[Athena_UnitClassesInit]Error %i loading spriteset " + spriteset + "\\n\", n_err);\n");
        source.write("        err = n_err || err;\n")
        source.write("    }\n")
        i = i+1
    
    source.write("    return err;\n}\n")
    
    source.flush()
    source.close()
    
    header.flush()
    header.close()
