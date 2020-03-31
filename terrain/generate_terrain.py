import numpy as np
import pyfastnoisesimd as fns


def generate_terrain(seed, points):
	coords = fns.empty_coords(points.shape[0])
	coords[:] = points.T

	noise = fns.Noise(seed=seed, numWorkers=1)
	noise.frequency = 0.02
	noise.noiseType = fns.NoiseType.Perlin
	noise.fractal.octaves = 4
	noise.fractal.lacunarity = 2.1
	noise.fractal.gain = 0.45
	noise.perturb.perturbType = fns.PerturbType.NoPerturb
	noise_values = noise.genFromCoords(coords)

	# Create an empty array of air
	res = np.full(points.shape[:1], 2, dtype=np.uint32)

	# Assign dirt
	res[points[:, 2] + noise_values * 40 < 5.0] = 100

	return res