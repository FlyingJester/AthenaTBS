import json
from tools import athena_unit_class_generator

# Generate unit_classes.c
print("Generating Unit Class definitions")
unit_classes_file = open("src/unit_classes.json", "r")
unit_classes_data = json.load(unit_classes_file)
unit_classes_file.close()
athena_unit_class_generator.CreateAthenaClassesSource(unit_classes_data, ["src/unit_classes.c", "src/unit_classes.h"])
