import json
from tools import athena_bin_to_c

# Generate Cynical
print("Generating font 'Sphere Cynical'")
athena_bin_to_c.BinToC("res/fonts/cynical.rfn", ["src/cynical.c", "src/cynical.h"])

# Generate Selawik
print("Generating font 'MS Selawik'")
athena_bin_to_c.BinToC("res/fonts/selawik/selawik-small-bold.rfn", ["src/selawik.c", "src/selawik.h"])

# Generate SGI font
print("Generating font 'SGI Screen'")
athena_bin_to_c.BinToC("res/fonts/sgi/sgi_screen.rfn", ["src/sgi_screen.c", "src/sgi_screen.h"])
