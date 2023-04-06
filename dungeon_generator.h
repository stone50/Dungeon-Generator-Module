#ifndef DUNGEON_GENERATOR_H
#define DUNGEON_GENERATOR_H

#include <memory>
#include <unordered_map>
#include <vector>

#include "../../scene/main/node.h"

class DungeonGenerator : public Node {
	template <class T>
	using VList = std::vector<T>;
	template <class T>
	using Matrix = VList<VList<T>>;
	template <class T>
	using Pointer = std::shared_ptr<T>;
	template <class K, class V>
	using Map = std::unordered_map<K, V>;

	GDCLASS(DungeonGenerator, Node);

	String tile_scene_path;

	uint8_t dungeon_width;
	uint8_t dungeon_height;
	uint8_t tile_size;

	int noise_seed;
	float noise_scale;

	Matrix<bool> _generate_ground();

	void _connect_borders(const Matrix<Vector2i> &borders, Matrix<bool> &pangea);

	void _find_island_distances(const Matrix<Vector2i> &borders, Matrix<char16_t> &closest_distances, Matrix<Vector2i> &closest_points);

	char16_t _find_closest_locations(const VList<Vector2i> &island1, const VList<Vector2i> &island2, Vector2i &location1, Vector2i &location2);

	void _find_closest_island_locations(
			const Matrix<char16_t> &closest_distances,
			const Matrix<Vector2i> &closest_points,
			Vector2i &location1,
			Vector2i &location2,
			char16_t &closest_island_index);

	void _connect_locations(const Vector2i &location1, const Vector2i &location2, Matrix<bool> &pangea);

	void _merge_island_distances(Matrix<char16_t> &closest_distances, Matrix<Vector2i> &closest_points, const char16_t closest_island_index);

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
