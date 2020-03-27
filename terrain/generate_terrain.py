import numpy as np

def generate_terrain(points):
	# Create an empty array of air
	res = np.full(points.shape[:1], 2, dtype=np.uint32)

	# Assign dirt
	res[points[:, 2] < 5.0] = 100

	return res