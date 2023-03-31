#include "dungeon_generator.h"

#include "../../modules/noise/fastnoise_lite.h"
#include "../../scene/2d/sprite_2d.h"
#include "../../scene/resources/packed_scene.h"

//#include "../../core/string/print_string.h"

void DungeonGenerator::generate() {
	Matrix<bool> pangea = _generate_ground();

	_spawn_ground(pangea);
}

DungeonGenerator::Matrix<bool> DungeonGenerator::_generate_ground() {
	FastNoiseLite noise = FastNoiseLite();
	noise.set_noise_type(FastNoiseLite::TYPE_SIMPLEX_SMOOTH);
	noise.set_seed(noise_seed);

	Matrix<bool> pangea = Matrix<bool>(dungeon_width, std::vector<bool>(dungeon_height, false));
	LocationSetList island_borders = {};

	for (int x = 0; x < dungeon_width; x++) {
		for (int y = 0; y < dungeon_height; y++) {
			Vector2i tile_location(x, y);

			if (noise.get_noise_2dv(tile_location * noise_scale) < 0) {
				continue;
			}

			pangea[x][y] = true;

			if (x > 0 && pangea[x - 1][y]) {
				Vector2i left_location(x - 1, y);
				int left_location_index = _find_island(left_location, island_borders);

				LocationSet &left_location_borders = island_borders[left_location_index];
				left_location_borders.insert(tile_location);

				if (y > 0 && y < dungeon_height - 1 && x > 1 && pangea[left_location.x][y - 1] && pangea[left_location.x][y + 1] && pangea[x - 2][y]) {
					left_location_borders.erase(left_location);
				}

				if (y > 0 && pangea[x][y - 1]) {
					int up_location_index = _find_island(Vector2i(x, y - 1), island_borders);

					if (left_location_index != up_location_index) {
						LocationSet &up_location_borders = island_borders[up_location_index];

						if (left_location_borders.size() < up_location_borders.size()) {
							up_location_borders.insert(left_location_borders.begin(), left_location_borders.end());
							island_borders.erase(island_borders.begin() + left_location_index);
						} else {
							left_location_borders.insert(up_location_borders.begin(), up_location_borders.end());
							island_borders.erase(island_borders.begin() + up_location_index);
						}
					}
				}
			} else {
				if (y > 0 && pangea[x][y - 1]) {
					island_borders[_find_island(Vector2i(x, y - 1), island_borders)].insert(tile_location);
				} else {
					island_borders.push_back({ tile_location });
				}
			}
		}
	}

	_connect_borders(island_borders, pangea);

	return pangea;
}

int DungeonGenerator::_find_island(const Vector2i &location, const LocationSetList &islands) {
	for (int i = 0; i < islands.size(); i++) {
		if (islands[i].count(location)) {
			return i;
		}
	}
	return -1;
}

void DungeonGenerator::_connect_borders(const LocationSetList &borders, Matrix<bool> &pangea) {
	Matrix<unsigned int> closest_distances;
	Matrix<Vector2i> closest_points;
	_find_island_distances(borders, closest_distances, closest_points);

	size_t island_count = borders.size() - 1;
	for (int isl = 0; isl < island_count; isl++) {
		Vector2i location1;
		Vector2i location2;
		int closest_island_index;
		_find_closest_island_locations(
				closest_distances,
				closest_points,
				location1,
				location2,
				closest_island_index);

		_connect_locations(location1, location2, pangea);

		_merge_island_distances(closest_distances, closest_points, closest_island_index);
	}
}

void DungeonGenerator::_find_island_distances(const LocationSetList &borders, Matrix<unsigned int> &closest_distances, Matrix<Vector2i> &closest_points) {
	closest_distances = Matrix<unsigned int>(borders.size(), std::vector<unsigned int>(borders.size(), UINT32_MAX));
	closest_points = Matrix<Vector2i>(borders.size(), std::vector<Vector2i>(borders.size(), Vector2i()));

	for (int island_1_index = 0; island_1_index < borders.size(); island_1_index++) {
		for (int island_2_index = island_1_index + 1; island_2_index < borders.size(); island_2_index++) {
			unsigned int distance = _find_closest_locations(
					borders[island_1_index],
					borders[island_2_index],
					closest_points[island_2_index][island_1_index],
					closest_points[island_1_index][island_2_index]);

			closest_distances[island_1_index][island_2_index] = distance;
			closest_distances[island_2_index][island_1_index] = distance;
		}
	}
}

unsigned int DungeonGenerator::_find_closest_locations(const LocationSet &island1, const LocationSet &island2, Vector2i &location1, Vector2i &location2) {
	unsigned int closest_distance = UINT32_MAX;
	for (const Vector2i &island_1_location : island1) {
		for (const Vector2i &island_2_location : island2_list) {
			unsigned int distance = abs(island_1_location.x - island_2_location.x) + abs(island_1_location.y - island_2_location.y);
			if (distance < closest_distance) {
				location1 = island_1_location;
				location2 = island_2_location;
				closest_distance = distance;
			}
		}
	}
	return closest_distance;
}

void DungeonGenerator::_find_closest_island_locations(
		const Matrix<unsigned int> &closest_distances,
		const Matrix<Vector2i> &closest_points,
		Vector2i &location1,
		Vector2i &location2,
		int &closest_island_index) {
	location1 = closest_points[0][1];
	location2 = closest_points[1][0];
	closest_island_index = 1;
	unsigned int closest_distance = closest_distances[0][1];
	for (int island_index = 2; island_index < closest_distances.size(); island_index++) {
		if (closest_distances[0][island_index] < closest_distance) {
			location1 = closest_points[island_index][0];
			location2 = closest_points[0][island_index];
			closest_island_index = island_index;
			closest_distance = closest_distances[0][island_index];
		}
	}
}

void DungeonGenerator::_connect_locations(const Vector2i &location1, const Vector2i &location2, Matrix<bool> &pangea) {
	int min_x = location1.x < location2.x ? location1.x : location2.x;
	int max_x = location1.x > location2.x ? location1.x : location2.x;
	int min_y = location1.y < location2.y ? location1.y : location2.y;
	int max_y = location1.y > location2.y ? location1.y : location2.y;

	for (int x = min_x; x <= max_x; x++) {
		pangea[x][location1.y] = true;
	}

	for (int y = min_y; y <= max_y; y++) {
		pangea[location2.x][y] = true;
	}
}

void DungeonGenerator::_merge_island_distances(Matrix<unsigned int> &closest_distances, Matrix<Vector2i> &closest_points, const int closest_island_index) {
	size_t island_count = closest_distances.size();
	for (int island_index = 1; island_index < island_count; island_index++) {
		if (island_index == closest_island_index) {
			continue;
		}

		unsigned int closest_to_current_distance = closest_distances[closest_island_index][island_index];
		if (closest_to_current_distance < closest_distances[0][island_index]) {
			closest_points[0][island_index] = closest_points[closest_island_index][island_index];
			closest_points[island_index][0] = closest_points[island_index][closest_island_index];
			closest_distances[0][island_index] = closest_to_current_distance;
			closest_distances[island_index][0] = closest_to_current_distance;
		}
	}

	for (int island_index = 0; island_index < island_count; island_index++) {
		if (island_index == closest_island_index) {
			continue;
		}

		closest_distances[island_index].erase(closest_distances[island_index].begin() + closest_island_index);
		closest_points[island_index].erase(closest_points[island_index].begin() + closest_island_index);
	}
	closest_distances.erase(closest_distances.begin() + closest_island_index);
	closest_points.erase(closest_points.begin() + closest_island_index);
}

void DungeonGenerator::_spawn_ground(const Matrix<bool> &pangea) {
	Ref<PackedScene> tile_scene = ResourceLoader::load(tile_scene_path, "PackedScene");

	Vector2 top_left = Vector2(dungeon_width, dungeon_height) * tile_size / -2;
	Node *dungeon_node = dynamic_cast<Node *>(ClassDB::instantiate("Node"));
	dungeon_node->set_name("Dungeon");
	for (int x = 0; x < dungeon_width; x++) {
		for (int y = 0; y < dungeon_height; y++) {
			if (!pangea[x][y]) {
				continue;
			}

			Sprite2D *tile = dynamic_cast<Sprite2D *>(tile_scene.ptr()->instantiate());
			tile->set_name("Ground Tile (" + String::num(x) + ", " + String::num(y) + ")");
			tile->set_position(Vector2(x, y) * tile_size + top_left);
			dungeon_node->add_child(tile);
		}
	}
	SceneTree::get_singleton()->get_current_scene()->call_deferred("add_child", dungeon_node);
}

void DungeonGenerator::_bind_methods() {
	ClassDB::bind_method(D_METHOD("generate"), &DungeonGenerator::generate);

	ClassDB::bind_method(D_METHOD("set_tile_scene_path"), &DungeonGenerator::set_tile_scene_path);
	ClassDB::bind_method(D_METHOD("get_tile_scene_path"), &DungeonGenerator::get_tile_scene_path);
	ClassDB::bind_method(D_METHOD("set_dungeon_width"), &DungeonGenerator::set_dungeon_width);
	ClassDB::bind_method(D_METHOD("get_dungeon_width"), &DungeonGenerator::get_dungeon_width);
	ClassDB::bind_method(D_METHOD("set_dungeon_height"), &DungeonGenerator::set_dungeon_height);
	ClassDB::bind_method(D_METHOD("get_dungeon_height"), &DungeonGenerator::get_dungeon_height);
	ClassDB::bind_method(D_METHOD("set_tile_size"), &DungeonGenerator::set_tile_size);
	ClassDB::bind_method(D_METHOD("get_tile_size"), &DungeonGenerator::get_tile_size);
	ClassDB::bind_method(D_METHOD("set_noise_seed"), &DungeonGenerator::set_noise_seed);
	ClassDB::bind_method(D_METHOD("get_noise_seed"), &DungeonGenerator::get_noise_seed);
	ClassDB::bind_method(D_METHOD("set_noise_scale"), &DungeonGenerator::set_noise_scale);
	ClassDB::bind_method(D_METHOD("get_noise_scale"), &DungeonGenerator::get_noise_scale);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "tile_scene_path", PROPERTY_HINT_TYPE_STRING), "set_tile_scene_path", "get_tile_scene_path");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "dungeon_width", PROPERTY_HINT_RANGE, "0, 255, 1"), "set_dungeon_width", "get_dungeon_width");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "dungeon_height", PROPERTY_HINT_RANGE, "0, 255, 1"), "set_dungeon_height", "get_dungeon_height");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "tile_size", PROPERTY_HINT_RANGE, "0, 255, 1"), "set_tile_size", "get_tile_size");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "noise_seed", PROPERTY_HINT_RANGE, "-2147483648, 2147483647, 1"), "set_noise_seed", "get_noise_seed");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "noise_scale", PROPERTY_HINT_RANGE, "0, 2147483647, 0.001"), "set_noise_scale", "get_noise_scale");
}

DungeonGenerator::DungeonGenerator() {
	tile_scene_path = "";

	dungeon_width = 80;
	dungeon_height = 45;
	tile_size = 8;

	noise_seed = 2;
	noise_scale = 20.0f;
}

void DungeonGenerator::set_tile_scene_path(const String &path) {
	tile_scene_path = path;
}

String DungeonGenerator::get_tile_scene_path() const {
	return tile_scene_path;
}

void DungeonGenerator::set_dungeon_width(uint8_t width) {
	dungeon_width = width;
}

uint8_t DungeonGenerator::get_dungeon_width() const {
	return dungeon_width;
}

void DungeonGenerator::set_dungeon_height(uint8_t height) {
	dungeon_height = height;
}

uint8_t DungeonGenerator::get_dungeon_height() const {
	return dungeon_height;
}

void DungeonGenerator::set_tile_size(uint8_t size) {
	tile_size = size;
}

uint8_t DungeonGenerator::get_tile_size() const {
	return tile_size;
}

void DungeonGenerator::set_noise_seed(int seed) {
	noise_seed = seed;
}

int DungeonGenerator::get_noise_seed() const {
	return noise_seed;
}

void DungeonGenerator::set_noise_scale(float scale) {
	noise_scale = scale;
}

float DungeonGenerator::get_noise_scale() const {
	return noise_scale;
}
