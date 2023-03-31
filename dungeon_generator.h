#ifndef DUNGEON_GENERATOR_H
#define DUNGEON_GENERATOR_H

#include <unordered_set>

#include "../../scene/main/node.h"

class DungeonGenerator : public Node {
	struct Vector2iHash {
		inline std::size_t operator()(const Vector2i &v) const {
			return ((size_t)v.x << sizeof(uint8_t)) + v.y;
		}
	};

	typedef std::unordered_set<Vector2i, Vector2iHash> LocationSet;
	typedef std::vector<LocationSet> LocationSetList;
	template <class T>
	using Matrix = std::vector<std::vector<T>>;

	GDCLASS(DungeonGenerator, Node);

	String tile_scene_path;

	uint8_t dungeon_width;
	uint8_t dungeon_height;
	uint8_t tile_size;

	int noise_seed;
	float noise_scale;

	Matrix<bool> _generate_ground();

	int _find_island(const Vector2i &location, const LocationSetList &islands);

	void _connect_borders(const LocationSetList &borders, Matrix<bool> &pangea);

	void _find_island_distances(const LocationSetList &borders, Matrix<unsigned int> &closest_distances, Matrix<Vector2i> &closest_points);

	unsigned int _find_closest_locations(const LocationSet &island1, const LocationSet &island2, Vector2i &location1, Vector2i &location2);

	void _find_closest_island_locations(
			const Matrix<unsigned int> &closest_distances,
			const Matrix<Vector2i> &closest_points,
			Vector2i &location1,
			Vector2i &location2,
			int &closest_island_index);

	void _connect_locations(const Vector2i &location1, const Vector2i &location2, Matrix<bool> &pangea);

	void _merge_island_distances(Matrix<unsigned int> &closest_distances, Matrix<Vector2i> &closest_points, const int closest_island_index);

	void _spawn_ground(const Matrix<bool> &pangea);

protected:
	static void _bind_methods();

public:
	void generate();

	DungeonGenerator();

	void set_tile_scene_path(const String &path);
	String get_tile_scene_path() const;

	void set_dungeon_width(uint8_t width);
	uint8_t get_dungeon_width() const;

	void set_dungeon_height(uint8_t height);
	uint8_t get_dungeon_height() const;

	void set_tile_size(uint8_t size);
	uint8_t get_tile_size() const;

	void set_noise_seed(int seed);
	int get_noise_seed() const;

	void set_noise_scale(float scale);
	float get_noise_scale() const;
};

#endif // DUNGEON_GENERATOR_H
