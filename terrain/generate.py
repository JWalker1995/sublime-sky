import numpy as np
import pyfastnoisesimd as fns


def generate_materials(seed):
	return [
		{
			'name': 'Air',
			'phase': 'gas',
			'mass': 1.0,
			'render_model': 'blinn',
			'color_diffuse': [0.0, 0.0, 0.0, 0.0],
			'color_specular': [0.0, 0.0, 0.0, 0.0],
			'shininess': 1.0,
		}, {
			'name': 'Ground',
			'phase': 'solid',
			'mass': 1.0,
			'render_model': 'blinn',
			'color_diffuse': [0.2, 0.2, 0.2, 1.0],
			'color_specular': [0.2, 0.2, 0.2, 1.0],
			'shininess': 32.0,
		}, {
			'name': 'Red',
			'phase': 'solid',
			'mass': 1.0,
			'render_model': 'blinn',
			'color_diffuse': [1.0, 0.2, 0.2, 1.0],
			'color_specular': [1.0, 0.2, 0.2, 1.0],
			'shininess': 32.0,
		},
		{'name': 'emerald', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [0.09717999999999999, 0.78874, 0.09717999999999999, 1.0], 'color_specular': [0.633, 0.727811, 0.633, 1.0], 'shininess': 76.8},
		{'name': 'jade', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [0.675, 1.1125, 0.7875, 1.0], 'color_specular': [0.316228, 0.316228, 0.316228, 1.0], 'shininess': 12.8},
		{'name': 'obsidian', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [0.2365, 0.22000000000000003, 0.2915, 1.0], 'color_specular': [0.332741, 0.328634, 0.346435, 1.0], 'shininess': 38.4},
		{'name': 'pearl', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [1.25, 1.03625, 1.03625, 1.0], 'color_specular': [0.296648, 0.296648, 0.296648, 1.0], 'shininess': 11.264},
		{'name': 'ruby', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [0.78874, 0.053110000000000004, 0.053110000000000004, 1.0], 'color_specular': [0.727811, 0.626959, 0.626959, 1.0], 'shininess': 76.8},
		{'name': 'turquoise', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [0.496, 0.92876, 0.86552, 1.0], 'color_specular': [0.297254, 0.30829, 0.306678, 1.0], 'shininess': 12.8},
		{'name': 'brass', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [1.109804, 0.792156, 0.141176, 1.0], 'color_specular': [0.992157, 0.941176, 0.807843, 1.0], 'shininess': 27.89743616},
		{'name': 'bronze', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [0.9265, 0.5559000000000001, 0.23543999999999998, 1.0], 'color_specular': [0.393548, 0.271906, 0.166721, 1.0], 'shininess': 25.6},
		{'name': 'chrome', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [0.65, 0.65, 0.65, 1.0], 'color_specular': [0.774597, 0.774597, 0.774597, 1.0], 'shininess': 76.8},
		{'name': 'copper', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [0.89505, 0.34398, 0.1053, 1.0], 'color_specular': [0.256777, 0.137622, 0.086014, 1.0], 'shininess': 12.8},
		{'name': 'gold', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [0.99889, 0.80598, 0.30097999999999997, 1.0], 'color_specular': [0.628281, 0.555802, 0.366065, 1.0], 'shininess': 51.2},
		{'name': 'silver', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [0.69979, 0.69979, 0.69979, 1.0], 'color_specular': [0.508273, 0.508273, 0.508273, 1.0], 'shininess': 51.2},
		{'name': 'black plastic', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [0.01, 0.01, 0.01, 1.0], 'color_specular': [0.5, 0.5, 0.5, 1.0], 'shininess': 32.0},
		{'name': 'cyan plastic', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [0.0, 0.6098039199999999, 0.56980392, 1.0], 'color_specular': [0.50196078, 0.50196078, 0.50196078, 1.0], 'shininess': 32.0},
		{'name': 'green plastic', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [0.1, 0.35, 0.1, 1.0], 'color_specular': [0.45, 0.55, 0.45, 1.0], 'shininess': 32.0},
		{'name': 'red plastic', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [0.5, 0.0, 0.0, 1.0], 'color_specular': [0.7, 0.6, 0.6, 1.0], 'shininess': 32.0},
		{'name': 'white plastic', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [0.55, 0.55, 0.55, 1.0], 'color_specular': [0.7, 0.7, 0.7, 1.0], 'shininess': 32.0},
		{'name': 'yellow plastic', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [0.5, 0.5, 0.0, 1.0], 'color_specular': [0.6, 0.6, 0.5, 1.0], 'shininess': 32.0},
		{'name': 'black rubber', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [0.03, 0.03, 0.03, 1.0], 'color_specular': [0.4, 0.4, 0.4, 1.0], 'shininess': 10.0},
		{'name': 'cyan rubber', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [0.4, 0.55, 0.55, 1.0], 'color_specular': [0.04, 0.7, 0.7, 1.0], 'shininess': 10.0},
		{'name': 'green rubber', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [0.4, 0.55, 0.4, 1.0], 'color_specular': [0.04, 0.7, 0.04, 1.0], 'shininess': 10.0},
		{'name': 'red rubber', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [0.55, 0.4, 0.4, 1.0], 'color_specular': [0.7, 0.04, 0.04, 1.0], 'shininess': 10.0},
		{'name': 'white rubber', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [0.55, 0.55, 0.55, 1.0], 'color_specular': [0.7, 0.7, 0.7, 1.0], 'shininess': 10.0},
		{'name': 'yellow rubber', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [0.55, 0.55, 0.4, 1.0], 'color_specular': [0.7, 0.7, 0.04, 1.0], 'shininess': 10.0},
		{'name': 'Brass', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [1.109804, 0.792156, 0.141176, 1.0], 'color_specular': [0.992157, 0.941176, 0.807843, 1.0], 'shininess': 27.8974},
		{'name': 'Bronze', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [0.9265, 0.5559000000000001, 0.23543999999999998, 1.0], 'color_specular': [0.393548, 0.271906, 0.166721, 1.0], 'shininess': 25.6},
		{'name': 'Polished Bronze', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [0.65, 0.38480000000000003, 0.16835, 1.0], 'color_specular': [0.774597, 0.458561, 0.200621, 1.0], 'shininess': 76.8},
		{'name': 'Chrome', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [0.65, 0.65, 0.65, 1.0], 'color_specular': [0.774597, 0.774597, 0.774597, 1.0], 'shininess': 76.8},
		{'name': 'Copper', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [0.89505, 0.34398, 0.1053, 1.0], 'color_specular': [0.256777, 0.137622, 0.086014, 1.0], 'shininess': 12.8},
		{'name': 'Polished Copper', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [0.7803, 0.30005, 0.0935, 1.0], 'color_specular': [0.580594, 0.223257, 0.0695701, 1.0], 'shininess': 51.2},
		{'name': 'Gold', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [0.99889, 0.80598, 0.30097999999999997, 1.0], 'color_specular': [0.628281, 0.555802, 0.366065, 1.0], 'shininess': 51.2},
		{'name': 'Polished Gold', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [0.5934, 0.5388000000000001, 0.1548, 1.0], 'color_specular': [0.797357, 0.723991, 0.208006, 1.0], 'shininess': 83.2},
		{'name': 'Pewter', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [0.5333330000000001, 0.529412, 0.654901, 1.0], 'color_specular': [0.333333, 0.333333, 0.521569, 1.0], 'shininess': 9.84615},
		{'name': 'Silver', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [0.69979, 0.69979, 0.69979, 1.0], 'color_specular': [0.508273, 0.508273, 0.508273, 1.0], 'shininess': 51.2},
		{'name': 'Polished Silver', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [0.50875, 0.50875, 0.50875, 1.0], 'color_specular': [0.773911, 0.773911, 0.773911, 1.0], 'shininess': 89.6},
		{'name': 'Emerald', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [0.09717999999999999, 0.78874, 0.09717999999999999, 1.0], 'color_specular': [0.633, 0.727811, 0.633, 1.0], 'shininess': 76.8},
		{'name': 'Jade', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [0.675, 1.1125, 0.7875, 1.0], 'color_specular': [0.316228, 0.316228, 0.316228, 1.0], 'shininess': 12.8},
		{'name': 'Obsidian', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [0.2365, 0.22000000000000003, 0.2915, 1.0], 'color_specular': [0.332741, 0.328634, 0.346435, 1.0], 'shininess': 38.4},
		{'name': 'Pearl', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [1.25, 1.03625, 1.03625, 1.0], 'color_specular': [0.296648, 0.296648, 0.296648, 1.0], 'shininess': 11.264},
		{'name': 'Ruby', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [0.78874, 0.053110000000000004, 0.053110000000000004, 1.0], 'color_specular': [0.727811, 0.626959, 0.626959, 1.0], 'shininess': 76.8},
		{'name': 'Turquoise', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [0.496, 0.92876, 0.86552, 1.0], 'color_specular': [0.297254, 0.30829, 0.306678, 1.0], 'shininess': 12.8},
		{'name': 'Black Plastic', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [0.01, 0.01, 0.01, 1.0], 'color_specular': [0.5, 0.5, 0.5, 1.0], 'shininess': 32},
		{'name': 'Black Rubber', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_diffuse': [0.03, 0.03, 0.03, 1.0], 'color_specular': [0.4, 0.4, 0.4, 1.0], 'shininess': 10},
	]

def generate_terrain(seed, points):
	coords = fns.empty_coords(points.shape[0])
	coords[:] = points.T

	# Cellular, PerlinFractal, ValueFractal, Cubic, Simplex, WhiteNoise, CubicFractal, SimplexFractal, Perlin, Value

	noise = fns.Noise(seed=seed, numWorkers=1)
	noise.frequency = 0.02
	noise.noiseType = fns.NoiseType.SimplexFractal
	noise.fractal.octaves = 4
	noise.fractal.lacunarity = 2.1
	noise.fractal.gain = 0.45
	noise.perturb.perturbType = fns.PerturbType.NoPerturb
	noise_values = noise.genFromCoords(coords)

	# Create an empty array of air
	res = np.full(points.shape[:1], 0, dtype=np.uint32)

	# Assign dirt
	res[points[:, 2] + noise_values * 40 < 5.0] = 1

	return res