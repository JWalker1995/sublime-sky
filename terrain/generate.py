import numpy as np
import pyfastnoisesimd as fns


def generate_materials(seed):
	return [
		{
			'special': 'REGENERATE',
		}, {
			'name': 'Air',
			'phase': 'gas',
			'mass': 1.0,
			'render_model': 'blinn',
			'color_ambient': [0.0, 0.0, 0.0, 1.0],
			'color_diffuse': [0.0, 0.0, 0.0, 0.0],
			'color_specular': [0.0, 0.0, 0.0, 0.0],
			'shininess': 1.0,
		}, {
			'name': 'Ground',
			'phase': 'solid',
			'mass': 1.0,
			'render_model': 'blinn',
			'color_ambient': [0.0, 0.0, 0.0, 1.0],
			'color_diffuse': [0.2, 0.2, 0.2, 1.0],
			'color_specular': [0.2, 0.2, 0.2, 1.0],
			'shininess': 32.0,
		}, {
			'name': 'Water',
			'phase': 'liquid',
			'mass': 1.0,
			'render_model': 'blinn',
			'color_ambient': [0.0, 0.0, 0.0, 1.0],
			'color_diffuse': [0.0, 0.08, 0.34, 1.0],
			'color_specular': [0.0, 0.1, 0.2, 1.0],
			'shininess': 32.0,
		}, {
			'name': 'Cloud',
			'phase': 'liquid',
			'mass': 1.0,
			'render_model': 'blinn',
			'color_ambient': [0.0, 0.01, 0.06, 1.0],
			'color_diffuse': [0.0, 0.01, 0.06, 1.0],
			'color_specular': [0.0, 0.0, 0.0, 1.0],
			'shininess': 32.0,
		},

		{'name': 'emerald', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.0215, 0.1745, 0.0215, 1.0], 'color_diffuse': [0.07568, 0.61424, 0.07568, 1.0], 'color_specular': [0.633, 0.727811, 0.633, 1.0], 'shininess': 76.8},
		{'name': 'jade', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.135, 0.2225, 0.135, 1.0], 'color_diffuse': [0.54, 0.89, 0.63, 1.0], 'color_specular': [0.316228, 0.316228, 0.316228, 1.0], 'shininess': 12.8},
		{'name': 'obsidian', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.05375, 0.05, 0.05375, 1.0], 'color_diffuse': [0.18275, 0.17, 0.22525, 1.0], 'color_specular': [0.332741, 0.328634, 0.346435, 1.0], 'shininess': 38.4},
		{'name': 'pearl', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.25, 0.20725, 0.25, 1.0], 'color_diffuse': [1, 0.829, 0.829, 1.0], 'color_specular': [0.296648, 0.296648, 0.296648, 1.0], 'shininess': 11.264},
		{'name': 'ruby', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.1745, 0.01175, 0.1745, 1.0], 'color_diffuse': [0.61424, 0.04136, 0.04136, 1.0], 'color_specular': [0.727811, 0.626959, 0.626959, 1.0], 'shininess': 76.8},
		{'name': 'turquoise', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.1, 0.18725, 0.1, 1.0], 'color_diffuse': [0.396, 0.74151, 0.69102, 1.0], 'color_specular': [0.297254, 0.30829, 0.306678, 1.0], 'shininess': 12.8},
		{'name': 'brass', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.329412, 0.223529, 0.329412, 1.0], 'color_diffuse': [0.780392, 0.568627, 0.113725, 1.0], 'color_specular': [0.992157, 0.941176, 0.807843, 1.0], 'shininess': 27.89743616},
		{'name': 'bronze', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.2125, 0.1275, 0.2125, 1.0], 'color_diffuse': [0.714, 0.4284, 0.18144, 1.0], 'color_specular': [0.393548, 0.271906, 0.166721, 1.0], 'shininess': 25.6},
		{'name': 'chrome', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.25, 0.25, 0.25, 1.0], 'color_diffuse': [0.4, 0.4, 0.4, 1.0], 'color_specular': [0.774597, 0.774597, 0.774597, 1.0], 'shininess': 76.8},
		{'name': 'copper', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.19125, 0.0735, 0.19125, 1.0], 'color_diffuse': [0.7038, 0.27048, 0.0828, 1.0], 'color_specular': [0.256777, 0.137622, 0.086014, 1.0], 'shininess': 12.8},
		{'name': 'gold', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.24725, 0.1995, 0.24725, 1.0], 'color_diffuse': [0.75164, 0.60648, 0.22648, 1.0], 'color_specular': [0.628281, 0.555802, 0.366065, 1.0], 'shininess': 51.2},
		{'name': 'silver', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.19225, 0.19225, 0.19225, 1.0], 'color_diffuse': [0.50754, 0.50754, 0.50754, 1.0], 'color_specular': [0.508273, 0.508273, 0.508273, 1.0], 'shininess': 51.2},
		{'name': 'black plastic', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.0, 0.0, 0.0, 1.0], 'color_diffuse': [0.01, 0.01, 0.01, 1.0], 'color_specular': [0.5, 0.5, 0.5, 1.0], 'shininess': 32.0},
		{'name': 'cyan plastic', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.0, 0.1, 0.0, 1.0], 'color_diffuse': [0.0, 0.50980392, 0.50980392, 1.0], 'color_specular': [0.50196078, 0.50196078, 0.50196078, 1.0], 'shininess': 32.0},
		{'name': 'green plastic', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.0, 0.0, 0.0, 1.0], 'color_diffuse': [0.1, 0.35, 0.1, 1.0], 'color_specular': [0.45, 0.55, 0.45, 1.0], 'shininess': 32.0},
		{'name': 'red plastic', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.0, 0.0, 0.0, 1.0], 'color_diffuse': [0.5, 0.0, 0.0, 1.0], 'color_specular': [0.7, 0.6, 0.6, 1.0], 'shininess': 32.0},
		{'name': 'white plastic', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.0, 0.0, 0.0, 1.0], 'color_diffuse': [0.55, 0.55, 0.55, 1.0], 'color_specular': [0.7, 0.7, 0.7, 1.0], 'shininess': 32.0},
		{'name': 'yellow plastic', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.0, 0.0, 0.0, 1.0], 'color_diffuse': [0.5, 0.5, 0.0, 1.0], 'color_specular': [0.6, 0.6, 0.5, 1.0], 'shininess': 32.0},
		{'name': 'black rubber', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.02, 0.02, 0.02, 1.0], 'color_diffuse': [0.01, 0.01, 0.01, 1.0], 'color_specular': [0.4, 0.4, 0.4, 1.0], 'shininess': 10.0},
		{'name': 'cyan rubber', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.0, 0.05, 0.0, 1.0], 'color_diffuse': [0.4, 0.5, 0.5, 1.0], 'color_specular': [0.04, 0.7, 0.7, 1.0], 'shininess': 10.0},
		{'name': 'green rubber', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.0, 0.05, 0.0, 1.0], 'color_diffuse': [0.4, 0.5, 0.4, 1.0], 'color_specular': [0.04, 0.7, 0.04, 1.0], 'shininess': 10.0},
		{'name': 'red rubber', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.05, 0.0, 0.05, 1.0], 'color_diffuse': [0.5, 0.4, 0.4, 1.0], 'color_specular': [0.7, 0.04, 0.04, 1.0], 'shininess': 10.0},
		{'name': 'white rubber', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.05, 0.05, 0.05, 1.0], 'color_diffuse': [0.5, 0.5, 0.5, 1.0], 'color_specular': [0.7, 0.7, 0.7, 1.0], 'shininess': 10.0},
		{'name': 'yellow rubber', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.05, 0.05, 0.05, 1.0], 'color_diffuse': [0.5, 0.5, 0.4, 1.0], 'color_specular': [0.7, 0.7, 0.04, 1.0], 'shininess': 10.0},
		{'name': 'Brass', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.329412, 0.223529, 0.329412, 1.0], 'color_diffuse': [0.780392, 0.568627, 0.113725, 1.0], 'color_specular': [0.992157, 0.941176, 0.807843, 1.0], 'shininess': 27.8974},
		{'name': 'Bronze', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.2125, 0.1275, 0.2125, 1.0], 'color_diffuse': [0.714, 0.4284, 0.18144, 1.0], 'color_specular': [0.393548, 0.271906, 0.166721, 1.0], 'shininess': 25.6},
		{'name': 'Polished Bronze', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.25, 0.148, 0.25, 1.0], 'color_diffuse': [0.4, 0.2368, 0.1036, 1.0], 'color_specular': [0.774597, 0.458561, 0.200621, 1.0], 'shininess': 76.8},
		{'name': 'Chrome', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.25, 0.25, 0.25, 1.0], 'color_diffuse': [0.4, 0.4, 0.4, 1.0], 'color_specular': [0.774597, 0.774597, 0.774597, 1.0], 'shininess': 76.8},
		{'name': 'Copper', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.19125, 0.0735, 0.19125, 1.0], 'color_diffuse': [0.7038, 0.27048, 0.0828, 1.0], 'color_specular': [0.256777, 0.137622, 0.086014, 1.0], 'shininess': 12.8},
		{'name': 'Polished Copper', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.2295, 0.08825, 0.2295, 1.0], 'color_diffuse': [0.5508, 0.2118, 0.066, 1.0], 'color_specular': [0.580594, 0.223257, 0.0695701, 1.0], 'shininess': 51.2},
		{'name': 'Gold', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.24725, 0.1995, 0.24725, 1.0], 'color_diffuse': [0.75164, 0.60648, 0.22648, 1.0], 'color_specular': [0.628281, 0.555802, 0.366065, 1.0], 'shininess': 51.2},
		{'name': 'Polished Gold', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.24725, 0.2245, 0.24725, 1.0], 'color_diffuse': [0.34615, 0.3143, 0.0903, 1.0], 'color_specular': [0.797357, 0.723991, 0.208006, 1.0], 'shininess': 83.2},
		{'name': 'Pewter', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.105882, 0.058824, 0.105882, 1.0], 'color_diffuse': [0.427451, 0.470588, 0.541176, 1.0], 'color_specular': [0.333333, 0.333333, 0.521569, 1.0], 'shininess': 9.84615},
		{'name': 'Silver', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.19225, 0.19225, 0.19225, 1.0], 'color_diffuse': [0.50754, 0.50754, 0.50754, 1.0], 'color_specular': [0.508273, 0.508273, 0.508273, 1.0], 'shininess': 51.2},
		{'name': 'Polished Silver', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.23125, 0.23125, 0.23125, 1.0], 'color_diffuse': [0.2775, 0.2775, 0.2775, 1.0], 'color_specular': [0.773911, 0.773911, 0.773911, 1.0], 'shininess': 89.6},
		{'name': 'Emerald', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.0215, 0.1745, 0.0215, 1.0], 'color_diffuse': [0.07568, 0.61424, 0.07568, 1.0], 'color_specular': [0.633, 0.727811, 0.633, 1.0], 'shininess': 76.8},
		{'name': 'Jade', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.135, 0.2225, 0.135, 1.0], 'color_diffuse': [0.54, 0.89, 0.63, 1.0], 'color_specular': [0.316228, 0.316228, 0.316228, 1.0], 'shininess': 12.8},
		{'name': 'Obsidian', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.05375, 0.05, 0.05375, 1.0], 'color_diffuse': [0.18275, 0.17, 0.22525, 1.0], 'color_specular': [0.332741, 0.328634, 0.346435, 1.0], 'shininess': 38.4},
		{'name': 'Pearl', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.25, 0.20725, 0.25, 1.0], 'color_diffuse': [1, 0.829, 0.829, 1.0], 'color_specular': [0.296648, 0.296648, 0.296648, 1.0], 'shininess': 11.264},
		{'name': 'Ruby', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.1745, 0.01175, 0.1745, 1.0], 'color_diffuse': [0.61424, 0.04136, 0.04136, 1.0], 'color_specular': [0.727811, 0.626959, 0.626959, 1.0], 'shininess': 76.8},
		{'name': 'Turquoise', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.1, 0.18725, 0.1, 1.0], 'color_diffuse': [0.396, 0.74151, 0.69102, 1.0], 'color_specular': [0.297254, 0.30829, 0.306678, 1.0], 'shininess': 12.8},
		{'name': 'Black Plastic', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0, 0, 0, 1.0], 'color_diffuse': [0.01, 0.01, 0.01, 1.0], 'color_specular': [0.5, 0.5, 0.5, 1.0], 'shininess': 32},
		{'name': 'Black Rubber', 'phase': 'solid', 'mass': 1.0, 'render_model': 'blinn', 'color_ambient': [0.02, 0.02, 0.02, 1.0], 'color_diffuse': [0.01, 0.01, 0.01, 1.0], 'color_specular': [0.4, 0.4, 0.4, 1.0], 'shininess': 10},
	]

def generate_terrain(seed, cubes):
	for c in cubes:
		if c['size_log2'] > 4:
			yield {
				**c,
				'cell_types': np.array([0] * 8, dtype=np.uint32),
			}
		else:
			axis = np.arange(1 << c['size_log2'])
			ox = (c['coord'].X() << c['size_log2']) - 0x35555555
			oy = (c['coord'].Y() << c['size_log2']) - 0x35555555
			oz = (c['coord'].Z() << c['size_log2']) - 0x35555555
			print('generated ' + str((1 << c['size_log2'], ox, oy, oz)))
			pts = np.stack(np.meshgrid(axis + ox, axis + oy, axis + oz), -1).reshape(-1, 3)

			coords = fns.empty_coords(pts.shape[0])
			coords[:] = pts.T / 1.0

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
			res = np.full(pts.shape[:1], 1, dtype=np.uint32)

			# Assign water
			# res[pts[:, 2] < 0.0] = 3

			# Assign dirt
			res[pts[:, 2] + noise_values * 10.0 < 0.0] = 2

			# # Assign clouds
			# res[(pts[:, 2] - 110) ** 2 * 1e-1 + noise_values * 100 < -60.0] = 3

			yield {
				**c,
				'cell_types': res,
			}